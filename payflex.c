#include <stdio.h>
#include "alpar.h"
#include "iso7816.h"

int main()
{
	int fd,atrlen,i,j,len;
	char mask[100],atr[100],data[100];

	fd=serial_init("/dev/smartpoll");
	if(send_num_mask(fd, mask)<0)
	{
		perror("send_num_mask failed");
		return -1;
	}

	printf("Mask is : %s\n", mask);
	if((atrlen = power_up_iso(fd, atr))<0)
	{
		perror("power_up_iso failed");
		return -1;
	}

	printf("ATR: ");
	for(i=0;i<atrlen;i++)
		printf("%x ", (unsigned char)atr[i]);
	printf("\n");
	
	if(select_file(fd, 0x3f00)<0)
	{
		printf("Select file failed\n");
		return -1;
	}

	printf("Selected file 0x3f00\n");

	/* Try verifying the PIN --- *VERY CAREFUL HERE* --- */
	/*
	data[0] = 0x47;
	data[1] = 0x46;
	data[2] = 0x58;
	data[3] = 0x49;
	data[4] = 0x32;
	data[5] = 0x56;
	data[6] = 0x78;
	data[7] = 0x40;
	*/

	/*
	data[0] = 0xfd;
	data[1] = 0x52;
	data[2] = 0x38;
	data[3] = 0x4a;
	data[4] = 0x04;
	data[5] = 0xd6;
	data[6] = 0x9b;
	data[7] = 0x80;
	*/

	data[0] = 0x47;
	data[1] = 0x46;
	data[2] = 0x55;
	data[3] = 0x39;
	data[4] = 0x38;
	data[5] = 0x49;
	data[6] = 0x90;
	data[7] = 0x63;

	if(verify_PIN(fd, data)<0)
		return -1;

    printf("Sucessfully verified the PIN for the master file\n");

	if(create_file_tpef(fd, 0x2030, 20, 0, 0, 0, 0)<0)
	{
		printf("Create file failed\n");
		return -1;
	}

    printf("Created a file 0x2030 of size 20 bytes\n");

    if(select_file(fd, 0x2030)<0)
    {
		printf("Select file failed\n");
		return -1;
    }

	printf("Selected file 0x2030\n");

	if((len=read_binary(fd, 0, 20, data))<0)
	{
		printf("read_binary failed\n");
		return -1;
	}

	printf("Data (b4 write): ");
	for(i=0;i<len;i++)
		printf("%x ", (unsigned char)data[i]);
	printf("\n");

	memset(data,0x05,7);

	if(update_binary(fd, 0x5, 7, data)<0)
		return -1;

	printf("Wrote to file 0x2030\n");

	if((len=read_binary(fd, 0, 20, data))<0)
		return -1;

	printf("Data (after write): ");
	for(i=0;i<len;i++)
		printf("%x ", (unsigned char)data[i]);
	printf("\n");

	if(select_file(fd, 0x3f00)<0)
	{
		printf("Select file failed\n");
		return -1;
	}

	printf("Selected file 0x3f00\n");

	if(delete_file(fd, 0x2030)<0)
	{
		printf("Delete file failed\n");
		return -1;
	}

	printf("Deleted file 0x2030\n");

	serial_deinit(fd);
}
