#include <stdio.h>
#include <errno.h>
#include "alpar.h"
#include "iso7816.h"

unsigned char card_class;
unsigned char recognized_card;

int apdu_send(int fd, unsigned char cla, unsigned char ins,
	unsigned char p1, unsigned char p2, unsigned char p3,
	int datalen, const char *data)
{
	int packetlen;
	unsigned char *packet;

	if(datalen < 0 )
	{
		errno = EINVAL;
		return -1;
	}

	//packetlen = (responselen ? 1 : 0) + (datalen ? 1 + datalen : 0); /* Body*/
	//packetlen += 4;	/* Header */

	packetlen = 5 + datalen;
	packet = (unsigned char *)malloc(packetlen);
	if(!packet)
	{
		errno = ENOMEM;
		return -1;
	}
	packet[0] = cla;
	packet[1] = ins;
	packet[2] = p1;
	packet[3] = p2;
	packet[4] = p3;

	if(datalen)
	{
		//packet[4] = datalen;
		memcpy(packet + 5, data, datalen);
	}

	/*
	if(responselen)
	{
		if(datalen)
			packet[5 + datalen] = responselen;
		else
			packet[4] = responselen;
	}
	*/

	return alpar_send(fd, CARD_COMMAND, packetlen, packet);
}

int apdu_recv(int fd, unsigned char *sw1, unsigned char *sw2, unsigned char *data)
{
	unsigned char lrc, *packet, command, ack;
	short int len;
	
	command = 0xff;
	packet = NULL;
	while(command != CARD_COMMAND)
	{
		len = alpar_recv(fd, &ack, &command, &packet, &lrc);
		if(len < 0)
			return -1;
	}
	/* FIXME : Verify lrc here */

	if(ack == ALPAR_NAK)
		return -1;

	*sw1 = packet[len - 2];
	*sw2 = packet[len - 1];

	if(len - 2 > 0)
		memcpy(data, packet, len - 2);

	if(packet)
		free(packet);

	return (len - 2);
}

int select_file(int fd, short int fident)
{
	unsigned char sw1,sw2,p1,p2;
	char fname[2];

	fname[0] = fident >> 8;
	fname[1] = fident & 0x00ff;

	/* P1=0x2 and P2=0x4 for Bull CP8 */
	if(recognized_card == BULLCP8)
	{
		p1 = 0x02;
		p2 = 0x04;
	}
	else
		p1 = p2 = 0;

	if(apdu_send(fd, CARD_CLA, SELECT_FILE, p1, p2, 0x02, 2, (const char *)fname)<0)
		return -1;

	if(apdu_recv(fd, &sw1, &sw2, NULL)<0)
		return -1;

	if(sw1 != APDU_SUCCESS)
		return -1;

	return 0;
}

int read_binary(int fd, short int offset, int read_buf_length, char *read_buf)
{
	unsigned char sw1,sw2;
	int len;

	if(apdu_send(fd, CARD_CLA, READ_BINARY, (unsigned char)(offset >> 8),
		(unsigned char)(offset & 0x00ff), read_buf_length, 0x0, NULL)<0)
		return -1;

	if((len=apdu_recv(fd, &sw1, &sw2, read_buf))<0)
		return -1;

	if(sw1 != APDU_SUCCESS)
		return -1;

	return len;
}

int write_binary(int fd, short int offset, int write_buf_length, char *write_buf)
{
	unsigned char sw1,sw2;

	if(apdu_send(fd, CARD_CLA, WRITE_BINARY, (unsigned char)(offset >> 8),
		(unsigned char)(offset & 0x00ff), write_buf_length,
		write_buf_length, write_buf)<0)
		return -1;

	if(apdu_recv(fd, &sw1, &sw2, NULL)<0)
		return -1;

	if(sw1 != APDU_SUCCESS)
		return -1;

	return 0;
}

int update_binary(int fd, short int offset, int write_buf_length, char *write_buf)
{
	unsigned char sw1,sw2;

	if(apdu_send(fd, CARD_CLA, UPDATE_BINARY, (unsigned char)(offset >> 8),
		(unsigned char)(offset & 0x00ff), write_buf_length,
		write_buf_length, write_buf)<0)
		return -1;

	if(apdu_recv(fd, &sw1, &sw2, NULL)<0)
		return -1;

	if(sw1 != APDU_SUCCESS)
		return -1;

	return 0;
}

int erase_binary(int fd)
{
	unsigned char sw1,sw2;

	if(apdu_send(fd, CARD_CLA, ERASE_BINARY, 0x0, 0x0, 0x0, 0, NULL)<0)
		return -1;

	if(apdu_recv(fd, &sw1, &sw2, NULL)<0)
		return -1;

	if(sw1 != APDU_SUCCESS)
		return -1;

	return 0;
}

int create_binary(int fd)
{
}

int verify_PIN(int fd, char *pin)
{
	unsigned char sw1,sw2;

	if(apdu_send(fd, CARD_CLA, VERIFY_PIN, 0x0, 0x0, 0x8, 8, pin)<0)
		return -1;

	if(apdu_recv(fd, &sw1, &sw2, NULL)<0)
		return -1;

	if(sw1 != APDU_SUCCESS)
		return -1;

	return 0;
}

int change_PIN(int fd)
{
}

int create_file_tpef(int fd, short int fident, short int size,
		unsigned char ac1, unsigned char ac2, unsigned char kn1,
		unsigned char kn2)
{
	unsigned char sw1,sw2,header[11];

	header[0] = (unsigned char)(size >> 8);
	header[1] = (unsigned char)(size & 0x00ff);
	header[2] = (unsigned char)(fident >> 8);
	header[3] = (unsigned char)(fident & 0x00ff);
	header[4] = 0x1; /* File type */
	header[5] = ac1;
	header[6] = ac2; /* Access control */
	header[7] = kn1;
	header[8] = kn2; /* Keys */
	header[9] = header[10] = 0; /* RFU bytes*/

	if(apdu_send(fd, CARD_CLA, CREATE_FILE, 0x0, 0x0, 0xb, 11, (const char *)header)<0)
		return -1;

	if(apdu_recv(fd, &sw1, &sw2, NULL)<0)
		return -1;

	if(sw1 != APDU_SUCCESS)
		return -1;

	return 0;
}

int delete_file(int fd, short int fident)
{
	unsigned char sw1,sw2;
	char fname[2];

	fname[0] = fident >> 8;
	fname[1] = fident & 0x00ff;

	if(apdu_send(fd, CARD_CLA, DELETE_FILE, 0x0, 0x0, 0x02, 2, (const char *)fname)<0)
		return -1;

	if(apdu_recv(fd, &sw1, &sw2, NULL)<0)
		return -1;

	if(sw1 != APDU_SUCCESS)
		return -1;

	return 0;
}

int generate_key(int fd)
{
}

int get_response(int fd)
{
}

int invalidate(int fd)
{
	unsigned char sw1,sw2;

	if(apdu_send(fd, CARD_CLA, INVALIDATE, 0x0, 0x0, 0x0, 0, NULL)<0)
		return -1;

	if(apdu_recv(fd, &sw1, &sw2, NULL)<0)
		return -1;

	if(sw1 != APDU_SUCCESS)
		return -1;

	return 0;
}

int rehabilitate(int fd)
{
	unsigned char sw1,sw2;

	if(apdu_send(fd, CARD_CLA, REHABILITATE, 0x0, 0x0, 0x0, 0, NULL)<0)
		return -1;

	if(apdu_recv(fd, &sw1, &sw2, NULL)<0)
		return -1;

	if(sw1 != APDU_SUCCESS)
		return -1;

	return 0;
}

int read_directory(int fd)
{
}

int unblock_code(int fd)
{
}
