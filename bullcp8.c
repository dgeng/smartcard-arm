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
	
	if(select_file(fd, 0x1f01)<0)
	{
		printf("Select file failed\n");
		return -1;
	}

	printf("Selected file 0x1f01\n");

	if((len=read_binary(fd, 0, 50, data))<0)
	{
		printf("read_binary failed\n");
		return -1;
	}

	printf("Data (b4 write): ");
	for(i=0;i<len;i++)
		printf("%x ", (unsigned char)data[i]);
	printf("\n");

	/*
	if(erase_binary(fd)<0)
		return -1;

	printf("Erased file 0x1f01\n");
	*/

	memset(data,0x05,10);

	if(update_binary(fd, 0x14, 10, data)<0)
		return -1;

	printf("Wrote to file 0x1f01\n");

	if((len=read_binary(fd, 0, 50, data))<0)
		return -1;

	printf("Data (after write): ");
	for(i=0;i<len;i++)
		printf("%x ", (unsigned char)data[i]);
	printf("\n");

	if(select_file(fd, 0x1f02)<0)
		return -1;

	printf("Selected file 0x1f02\n");

	for(i=0,j=0;i<0xf;i+=2)
		data[j++] = (i << 4) | ( i + 1) ;

	if(verify_PIN(fd, data)<0)
		return -1;

	printf("PIN verification was successfull\n");

	serial_deinit(fd);
}
