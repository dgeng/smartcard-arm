#include <stdio.h>
#include "alpar.h"
#include "iso7816.h"

int main()
{
	int fd,atrlen,i,j,len;
	char mask[100],atr[100],data[300];

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

	if(select_file(fd, 0x2030)<0)
	{
		printf("Select file failed\n");
		return -1;
	}

	printf("Selected file 0x2030\n");

	if((len=read_binary(fd, 0, 1, data))<0)
	{
		printf("read binary failed\n");
		return -1;
	}

	if((len=read_binary(fd, 1, data[0], data + 1))<0)
	{
		printf("read_binary failed\n");
		return -1;
	}

	printf("Data (read from card) (%d): ",data[0]);
	for(i=0;i<len;i++)
		printf("%x ", (unsigned char)data[i + 1]);
	printf("\n");

	serial_deinit(fd);
}
