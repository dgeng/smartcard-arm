#define __KERNEL__
#define MODULE

#define SP_MAJOR    173
#define SP_NAME     "smartpoll"

#define MAX_SIZE    506 + 4 + 1

#include <linux/config.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/serialP.h>
#include <linux/major.h>
#include <linux/string.h>
#include <linux/fcntl.h>
#include <linux/ptrace.h>
#include <linux/ioport.h>
#include <linux/mm.h>
#include <linux/malloc.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/system.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/bitops.h>
#include <asm/arch/hardware.h>
#include <asm/arch/serial_reg.h>
#include <asm/arch/memory.h>

#define     serial_send(x) { while(!(Ser3UTSR0 & UTSR0_TFS)); Ser3UTDR = (x); }

#define     serial_recv(x) { while(!(Ser3UTSR1 & UTSR1_RNE)); (x) = Ser3UTDR; }

static ssize_t smartpoll_read(struct file *filp, char *buf, size_t count, loff_t *l);
static ssize_t smartpoll_write(struct file *filp, const char *buf, size_t count, loff_t *l);
static int smartpoll_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
static int smartpoll_open(struct inode *inode, struct file *filp);
static int smartpoll_release(struct inode *inode, struct file *filp);

static struct file_operations smartpoll_fops = {
   read: smartpoll_read,
   write: smartpoll_write,
   ioctl: smartpoll_ioctl,
   open: smartpoll_open,
   release: smartpoll_release,
};

static unsigned char smartpoll_buffer[MAX_SIZE]; /* Max data size = 506 (data) + 4 (header) + 1 (LRC) */
static int smartpoll_response_len;

static ssize_t smartpoll_read(struct file *filp, char *buf, size_t count, loff_t *l)
{
    int len = smartpoll_response_len;

    if(count < MAX_SIZE)
        return -ENOBUFS;

    smartpoll_response_len = 0;

    copy_to_user(buf, smartpoll_buffer, len);
    return len;
}

/* 
 * Write involves writing to TDA8008 and *also* getting the response
 * and storing the response in the buffer. Cannot wait for read to come
 * in, since TDA8008 sends immediately
 */
static ssize_t smartpoll_write(struct file *filp, const char *buf, size_t count, loff_t *l)
{
    int i;

    if(count > MAX_SIZE)
        return -EMSGSIZE;

    smartpoll_response_len = 0;

    /* flush xmit buffer */
    for(i=0;i<8;i++)
        Ser3UTDR = 0;

    /* flush rcv buffer */
   while(Ser3UTSR1 & UTSR1_RNE)
        i = Ser3UTDR;

   copy_from_user(smartpoll_buffer, buf, count);

   cli(); /* Vivek : hack */
   for(i=0;i<count;i++)
       serial_send(smartpoll_buffer[i]);

   serial_recv(smartpoll_buffer[0]);    /* pattern      */
   serial_recv(smartpoll_buffer[1]);    /* length high  */
   serial_recv(smartpoll_buffer[2]);    /* length low   */

   /* Calculate the length of the response */
   smartpoll_response_len = ((unsigned short)smartpoll_buffer[1] << 8) | (unsigned short)smartpoll_buffer[2];

   serial_recv(smartpoll_buffer[3]);    /* control      */

   smartpoll_response_len += 5;         /* data + header(4) + LRC(1) */

   for(i=4; i < smartpoll_response_len; i++)    /* read rest of the packet */
       serial_recv(smartpoll_buffer[i]);
   sti(); /* Vivek : hackend */

   return count;
}

static int smartpoll_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    return 0;
}

static int smartpoll_open(struct inode *inode, struct file *filp)
{
    int i;

    /* flush xmit buffer */
    for(i=0;i<8;i++)
        Ser3UTDR = 0;

    /* flush rcv buffer */
   while(Ser3UTSR1 & UTSR1_RNE)
        i = Ser3UTDR;

   MOD_INC_USE_COUNT;
   return 0;
}

static int smartpoll_release(struct inode *inode, struct file *filp)
{
   MOD_DEC_USE_COUNT;
   return 0;
}


static int __init smartpoll_init_module(void)
{
    int i;

    /*
	printk("Ser3UTCR0 = %x\n",Ser3UTCR0);
	printk("Ser3UTCR1 = %x\n",Ser3UTCR1);
	printk("Ser3UTCR2 = %x\n",Ser3UTCR2);
	printk("Ser3UTCR3 = %x\n",Ser3UTCR3);
    */

    if(register_chrdev(SP_MAJOR, SP_NAME, &smartpoll_fops))
    {
        printk("Failed to register smart card serial device\n");
        return -EIO;
    }

    Ser3UTSR0 = 0xff;   /* Clear sticky status */

    /* Parity enable + Parity Even + 2 Stop bits + 8 Bit data*/
    Ser3UTCR0 = UTCR0_PE | UTCR0_EvenPar | UTCR0_2StpBit | UTCR0_8BitData;
    Ser3UTCR1 = 0x0;
    Ser3UTCR2 = 0x05;   /* 38400 Baud */

    /* flush xmit buffer */
    for(i=0;i<8;i++)
        Ser3UTDR = 0;

    /* flush rcv buffer */
    while(Ser3UTSR1 & UTSR1_RNE)
        i = Ser3UTDR;

    /* Enable Receieve and transmit */
    Ser3UTCR3 = UTCR3_TXE | UTCR3_RXE;

    printk("Smart card serial driver loaded\n");

	return 0;
}


static void __exit smartpoll_cleanup_module(void)
{
    /* Disable receive and transmit */
    Ser3UTCR3 = 0x0;
    unregister_chrdev(SP_MAJOR,SP_NAME);
}

/* The default module stuff */
module_init(smartpoll_init_module);
module_exit(smartpoll_cleanup_module);

MODULE_AUTHOR("Vivek K S");
MODULE_DESCRIPTION("Smart card serial driver");

EXPORT_NO_SYMBOLS;
