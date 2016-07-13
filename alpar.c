#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/serial.h>
#include <termios.h>
#include <unistd.h>
#include "alpar.h"

extern unsigned char card_class;
extern unsigned char recognized_card;

struct atr_table recognized_cards[MAX_CARDS]=
{
	{
		6,
		{0x3b,0x23,0x0,0x35,0x13,0x80},
		"Schlumberger: T=? PENN Cash",
		UNKNOWN_CLASS
	},
	{
		10,
		{0x3b,0x75,0x11,0x0,0x0,0x24,0xc2,0x1,0x90,0x0},
		"Philips: T=0 DS Personalization",
		0x0
	},
	{ 	14, 
		{0x3b,0x79,0x94,0x0,0x0,0x59,0x1,0x1,0xe,0x1,0x0,0x1,0x2,0xa9},
		"Schlumberger: T=0 Payflex S 2K",
		0x0
	},
	{ 	15, 
		{0x3b,0xe7,0x25,0x0,0x81,0x21,0x5d,0x29,0x40,0x0,0x0,0x68,0x90,0x0,0xae},
		"Philips: T=1 Bull CP8",
		0x80
	}
};

int serial_init(const char *device)
{
    int serial_handle;
    struct termios termp;

    serial_handle = open(device,O_RDWR );

    if(serial_handle == -1)
    {
        perror("Could not open serial device");
        return -1;
    }

    return serial_handle;
}

short int alpar_recv(int fd, unsigned char *ack, unsigned char *command, 
            unsigned char **data, unsigned char *lrc )
{
    unsigned short int length;
    int total_bytes_read;
    unsigned char packet[520];
    unsigned char local_lrc;

    total_bytes_read = read(fd, packet, 520);

    *ack = packet[0];

    if((*ack != ALPAR_ACK) && (*ack != ALPAR_NAK) )
    {
	    errno = EIO;
	    return -1;
    }

    length = ( packet[1] << 8 ) | ( packet[2] );

    *command = packet[3];

    if(length)
    {
        *data = (char *)malloc(length * sizeof(char));
        if(! *data)
        {
            errno = ENOMEM;
            return -1;
        }

        memcpy( *data, packet + 4, length);
    }
    else
        *data = NULL;

    *lrc = packet[total_bytes_read - 1];

    return length;
}

int alpar_send(int fd, unsigned char command, 
            unsigned short int length,
            const unsigned char *data )
{
    unsigned char *local_data;
    unsigned char lrc;
    int i;

    if(length > 506)
    {
        errno = EMSGSIZE;
        return -1;
    }

    local_data = (unsigned char *) malloc((length + 4) * sizeof(char));

    if(!local_data)
    {
        errno = ENOMEM;
        return -1;
    }

    local_data[0] = ALPAR_ACK;
    local_data[1] = (length & 0xFF00) >> 8 ;
    local_data[2] = length & 0x00FF;
    local_data[3] = command;

    if(length)
        memcpy(local_data + 4,data,length);

    lrc = local_data[0];

    for(i=1; i<(length + 4); i++)
        lrc = lrc ^ local_data[i];

    local_data[i] = lrc;

    if(write(fd, local_data, length + 5 ) != (length + 5))
    {
        errno = EIO;
        free(local_data);
        return -1;
    }

    free(local_data);
    return length ;
}

int serial_deinit(int fd)
{
    if(fd == -1)
        return -1;
    return close(fd);
}

int send_num_mask(int fd, char *result)
{
	unsigned char ack, command, lrc, *data;
	short len;
	
	if(alpar_send(fd, SEND_NUM_MASK, 0, NULL)<0)
		return -1;

	data = NULL;
	len = alpar_recv(fd, &ack, &command, &data, &lrc);
	if(len < 0)
		return -1;

	memcpy(result, data, len);
	result[len]=0;

	if(data)
		free(data);

	return len;
}

int power_up_iso(int fd, char *atr)
{
	unsigned char ack, command, lrc, *data;
	short len;
	
	if(alpar_send(fd, POWER_UP_ISO, 0, NULL)<0)
		return -1;

	data = NULL;
	len = alpar_recv(fd, &ack, &command, &data, &lrc);
	if(len < 0)
		return -1;

	if(ack != ALPAR_ACK)
	{
		/* Card absent */
		if(data[0] == 0xc0)
			errno = ENOENT;

		return -1;
	}

	memcpy(atr, data, len);
	atr[len]=0;

	if(data)
		free(data);

	/* 
	   Try to recognize the card by the ATR,
	   If the card is recognized, set the card
	   class appropriately
	 */

	if(len)
	{
		int i;

		for(i=0;i<MAX_CARDS;i++)
		{
			if(!memcmp(atr, recognized_cards[i].atr,
					       recognized_cards[i].size))
			{
				fprintf(stderr,"Recognized card %s\n",recognized_cards[i].card_name);
				card_class = recognized_cards[i].card_class;
				recognized_card = i;
				break;
			}
		}
		if(i==MAX_CARDS)
		{
			fprintf(stderr,"Card was not recognized\n");
			card_class = UNKNOWN_CLASS;	/* Unrecognized card */
			recognized_card = UNRECOGNIZED;
		}
	}
	else
	{
		card_class = UNKNOWN_CLASS;	/* Unrecognized card */
		recognized_card = UNRECOGNIZED;
	}
	return len;
}

int power_up_3V(int fd, char *atr)
{
	unsigned char ack, command, lrc, *data;
	short len;
	
	data = NULL;
	if(alpar_send(fd, POWER_UP_3V, 0, NULL)<0)
		return -1;

	len = alpar_recv(fd, &ack, &command, &data, &lrc);
	if(len < 0)
		return -1;

	memcpy(atr, data, len);
	atr[len]=0;

	if(data)
		free(data);
	return len;
}

int power_up_5V(int fd, char *atr)
{
	unsigned char ack, command, lrc, *data;
	short len;
	
	data = NULL;
	if(alpar_send(fd, POWER_UP_5V, 0, NULL)<0)
		return -1;

	len = alpar_recv(fd, &ack, &command, &data, &lrc);
	if(len < 0)
		return -1;

	memcpy(atr, data, len);
	atr[len]=0;

	if(data)
		free(data);
	return len;
}

int power_down_mode(int fd)
{
	unsigned char ack, command, lrc, *data;
	short len;
	
	data = NULL;

	if(alpar_send(fd, POWER_DOWN_MODE, 0, NULL)<0)
		return -1;

	len = alpar_recv(fd, &ack, &command, &data, &lrc);
	if(len < 0)
		return -1;

	if(data)
		free(data);
	return len;
}

int power_off(int fd)
{
	unsigned char ack, command, lrc, *data;
	short len;
	
	data = NULL;

	if(alpar_send(fd, POWER_OFF, 0, NULL)<0)
		return -1;

	len = alpar_recv(fd, &ack, &command, &data, &lrc);
	if(len < 0)
		return -1;

	if(data)
		free(data);
	return len;
}

int idle_mode_high(int fd)
{
	unsigned char ack, command, lrc, *data;
	short len;
	
	data = NULL;

	if(alpar_send(fd, IDLE_MODE_HIGH, 0, NULL)<0)
		return -1;

	len = alpar_recv(fd, &ack, &command, &data, &lrc);
	if(len < 0)
		return -1;

	if(data)
		free(data);
	return len;
}

int idle_mode_low(int fd)
{
	unsigned char ack, command, lrc, *data;
	short len;
	
	data = NULL;

	if(alpar_send(fd, IDLE_MODE_LOW, 0, NULL)<0)
		return -1;

	len = alpar_recv(fd, &ack, &command, &data, &lrc);
	if(len < 0)
		return -1;

	if(data)
		free(data);
	return len;
}

int select_card(int fd, unsigned char cardno)
{
	unsigned char ack, command, lrc, *data;
	short len;
	
	data = NULL;
	if(alpar_send(fd, SELECT_CARD, 1, &cardno)<0)
		return -1;

	len = alpar_recv(fd, &ack, &command, &data, &lrc);
	if(len < 0)
		return -1;

	if(data)
		free(data);

	return len;
}

int set_clock_card(int fd, unsigned char par)
{
	unsigned char ack, command, lrc, *data;
	short len;
	
	data = NULL;
	if(alpar_send(fd, SELECT_CARD, 1, &par)<0)
		return -1;

	len = alpar_recv(fd, &ack, &command, &data, &lrc);
	if(len < 0)
		return -1;

	if(data)
		free(data);

	return len;
}

int ifs_request(int fd, unsigned char ifsd)
{
	unsigned char ack, command, lrc, *data;
	short len;
	
	data = NULL;
	if(alpar_send(fd, SELECT_CARD, 1, &ifsd)<0)
		return -1;

	len = alpar_recv(fd, &ack, &command, &data, &lrc);
	if(len < 0)
		return -1;

	if(data)
		free(data);

	return len;
}

int show_fidi(int fd, char *fidi)
{
	unsigned char ack, command, lrc, *data;
	short len;
	
	if(alpar_send(fd, SHOW_FIDI, 0, NULL)<0)
		return -1;

	data = NULL;
	len = alpar_recv(fd, &ack, &command, &data, &lrc);
	if(len < 0)
		return -1;

	memcpy(fidi, data, len);
	fidi[len]=0;

	if(data)
		free(data);

	return len;
}

int set_card_baud_rate(int fd, unsigned char fidi, unsigned char cku)
{
	unsigned char ack, command, lrc, *data;
	unsigned char params[2];
	short len;
	
	data = NULL;
	params[0]=fidi;
	params[1]=cku;
	if(alpar_send(fd, SET_CARD_BAUD_RATE, 2, params)<0)
		return -1;

	len = alpar_recv(fd, &ack, &command, &data, &lrc);
	if(len < 0)
		return -1;

	if(data)
		free(data);

	return len;
}

int negotiate(int fd, unsigned char protocol, unsigned char fidi)
{
	unsigned char ack, command, lrc, *data;
	unsigned char params[2];
	short len;
	
	data = NULL;
	params[0]=protocol;
	params[1]=fidi;
	if(alpar_send(fd, NEGOTIATE, 2, params)<0)
		return -1;

	len = alpar_recv(fd, &ack, &command, &data, &lrc);
	if(len < 0)
		return -1;

	if(data)
		free(data);

	return len;
}

int card_command(int fd, const char *iso_cmd, int cmd_len, char *iso_resp)
{
	unsigned char ack, command, lrc, *data;
	short len;
	
	data = NULL;
	if(alpar_send(fd, CARD_COMMAND, cmd_len, iso_cmd)<0)
		return -1;

	len = alpar_recv(fd, &ack, &command, &data, &lrc);
	if(len < 0)
		return -1;

	memcpy(iso_resp, data, len);
	iso_resp[len]=0;

	if(data)
		free(data);

	return len;
}
