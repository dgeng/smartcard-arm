%module smartcard
%{
#include <stdio.h>
#include "alpar.h"
#include "iso7816.h"
%}

/* Serial initialization / de-initializtion */
extern int serial_init(const char *device);
extern int serial_deinit(int fd);

/* Basic ALPAR send/recv */
extern short int alpar_recv(int fd, unsigned char *ack, unsigned char *command, 
            unsigned char **data, unsigned char *lrc );

extern int alpar_send(int fd, 
            unsigned char command,
            unsigned short int length,
            const unsigned char *data );

/* Higher layers of ALPAR */

extern int send_num_mask(int fd, char *REFERENCE);
extern int idle_mode_high(int fd);
extern int idle_mode_low(int fd);
extern int power_down_mode(int fd);
extern int power_off(int fd);
extern int power_up_iso(int fd, char *REFERENCE);
extern int power_up_3V(int fd, char *REFERENCE);
extern int power_up_5V(int fd, char *REFERENCE);
extern int card_command(int fd, const char *iso_cmd, int cmd_len, char *REFERENCE);

extern int apdu_send(int fd, unsigned char cla, unsigned char ins, unsigned char p1, unsigned char p2, unsigned char p3, int datalen, const char *data);

extern int apdu_recv(int fd, unsigned char *OUTPUT, unsigned char *OUTPUT, unsigned char *REFERENCE);

extern int select_file(int fd, short int fident);
extern int read_binary(int fd, short int offset, int read_buf_length, char *REFERENCE);
extern int write_binary(int fd, short int offset, int write_buf_length, char *REFERENCE);
extern int update_binary(int fd, short int offset, int write_buf_length, char *REFERENCE);
extern int erase_binary(int fd);
extern int verify_PIN(int fd, char *REFERENCE);
extern int delete_file(int fd, short int fident);
extern int invalidate(int fd);
extern int rehabilitate(int fd);
extern int create_file_tpef(int fd, short int fident, short int size,
		unsigned char ac1, unsigned char ac2, unsigned char kn1,
		unsigned char kn2);
