#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include "alpar.h"

void isend(int fd, const char *command_str);
void irecv(int fd);
void help();
void send_and_recv_loop(int serial_handle);

int main(int argc, char *argv[])
{
    int serial_handle;

    if(argc != 2)
    {
        printf("Usage: %s <serial device name>\n", argv[0]);
        exit(-1);
    }

    printf("Smart card test program\n");
    if((serial_handle = serial_init(argv[1])) == -1)
    {
        printf("Initialization failed\n");
        return 1;
    }

    send_and_recv_loop(serial_handle);

    serial_deinit(serial_handle);
    return 0;
}

void help()
{
    printf("\nrecv                      recv a packet from the smart card\n");
    printf("send <control> <len>      send a packet to the smart card\n");
    printf("quit                        exit\n");
}

void send_and_recv_loop(int serial_handle)
{
    fd_set fds;

    while(1)
    {
        unsigned char command[100];
        unsigned char *c;
        int err;

        printf("\nsmart>");
        fflush(stdout);

        fgets(command, 100, stdin);
        //printf("command is : %s\n", command);
        //fflush(stdin);
        c= command;
        while(*c==' ' || *c=='\t')
            c++;

        if(!strncmp(c,"quit",4))
            break;
        else if(!strncmp(c,"send",4))
        {
            isend(serial_handle,c);
            irecv(serial_handle);
        }
        else if(!strncmp(c,"recv",4))
            irecv(serial_handle);
        else if(!strncmp(c,"help",4))
            help();
        else
            printf("Invalid command\n");

    }
}

void isend(int fd, const char *command_str)
{
    unsigned short int control;
    unsigned int length;
    char *data;

    data = NULL;
    if(sscanf(command_str,"%*s %hx %x",&control,&length)!=2)
    {
        printf("scanf failed..less arguements ?");
        return;
    }

    if(length)
    {
        int i;

        data = (char *)malloc(length * sizeof(char));

        if(!data)
        {
            printf("No memory\n");
            return;
        }

        for(i=0;i<length;i++)
        {
            printf("data[%d]=",i);
            scanf("%x",&data[i]);
        }
        getchar(); /* to get rid of \n */
    }

    printf("control=%hx,length=%x\n",control,length);

    if(length)
    {
        int i;

        for(i=0;i<length;i++)
            printf("data[%d]=%u(0x%x)\n",i,(unsigned char)data[i],(unsigned char)data[i]);
    }

    if(alpar_send(fd, control, length, data)==-1)
        perror("send failed");

    if(data)
        free(data);
}

void irecv(int fd)
{
    unsigned char command;
    unsigned char *data;
    short int length;
    unsigned char lrc;
    unsigned char ack;

    length = alpar_recv(fd,&ack,&command,&data,&lrc);
    if(length == -1)
    {
        perror("receieve failure");
        return;
    }

    printf("command=%x,length=%hx\n",command,length);
    if(length)
    {
        int i;

        for(i=0;i<length;i++)
            printf("data[%d]=%u(0x%x)(\'%c\')\n",i,(unsigned char)data[i],(unsigned char)data[i],(unsigned char)data[i]);
        free(data);
    }
    printf("lrc=%u\n",lrc);
}
