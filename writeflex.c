#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "alpar.h"
#include "iso7816.h"

int main(int argc, char *argv[])
{
	int fd,fds,atrlen,i,j,len,scode_len;
	char mask[100],atr[100],data[300];
	
	if(argc != 2)
	{
		printf("Usage: %s <sCode file name>\n", argv[0]);
		exit(-1);
	}

	if((fds=open(argv[1],O_RDONLY))<0)
	{
		perror("Unable to open sCode file");
		return -1;
	}

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

	if(create_file_tpef(fd, 0x2030, 200, 0, 0, 0, 0)<0)
	{
		printf("Create file failed\n");
		return -1;
	}

	printf("Created a file 0x2030 of size 200 bytes\n");

	if(select_file(fd, 0x2030)<0)
	{
		printf("Select file failed\n");
		return -1;
	}

	printf("Selected file 0x2030\n");

	scode_len=read(fds, data + 1, 200);

	if(scode_len < 0)
	{
		perror("Unable to read scode_len");
		return -1;
	}

	data[scode_len + 1]=0;
	data[0] = scode_len;	/* Store length */

	printf("scode_len=%d, sCode=%s\n",scode_len, data);


	for(i=0,j=scode_len + 1;j>0;)
	{
		if(update_binary(fd, i, (j >= 20) ? 20 : j, data + i)<0)
			return -1;
		if(j>=20)
		{
			j-=20;
			i+=20;
		}
		else
			break;
	}

	printf("Wrote to file 0x2030\n");

	if((len=read_binary(fd, 0, 1, data))<0)
	{
		printf("read binary failed\n");
		return -1;
	}

	if(data[0] != scode_len)
	{
		printf("Stored data size does not match data!!\n");
		return -1;
	}

	if((len=read_binary(fd, 1, data[0], data + 1))<0)
	{
		printf("read binary failed\n");
		return -1;
	}

	printf("Data (after write) (%d): ",len);
	for(i=0;i<len;i++)
		printf("%x ", (unsigned char)data[i + 1]);
	printf("\n");

	serial_deinit(fd);
}
