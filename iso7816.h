#ifndef _ISO7816_H_
#define _ISO7816_H_

#define APDU_SUCCESS	0x90

#define SELECT_FILE	0xa4
#define READ_BINARY	0xb0
#define WRITE_BINARY	0xd0
#define UPDATE_BINARY	0xd6
#define ERASE_BINARY	0x0e
#define VERIFY_PIN	0x20
#define	CREATE_FILE	0xe0
#define	DELETE_FILE	0xe4
#define	INVALIDATE	0x04
#define	REHABILITATE	0x44

#define	BULL_CP8_CLA	0x80
#define	PHILIPS_DS_CLA	0x0

#define CARD_CLA	    card_class

int apdu_send(int fd, unsigned char cla, unsigned char ins,
	unsigned char p1, unsigned char p2, unsigned char p3,
	int datalen, const char *data);

int apdu_recv(int fd, unsigned char *sw1, unsigned char *sw2, unsigned char *data);

int select_file(int fd, short int fident);
int read_binary(int fd, short int offset, int read_buf_length, char *read_buf);
int write_binary(int fd, short int offset, int write_buf_length, char *write_buf);
int update_binary(int fd, short int offset, int write_buf_length, char *write_buf);
int erase_binary(int fd);
int create_binary(int fd);
int verify_PIN(int fd, char *pin);
int change_PIN(int fd);
int create_file_tpef(int fd, short int fident, short int size,
		unsigned char ac1, unsigned char ac2, unsigned char kn1,
		unsigned char kn2);
int delete_file(int fd, short int fident);
int generate_key(int fd);
int get_response(int fd);
int invalidate(int fd);
int rehabilitate(int fd);
int read_directory(int fd);
int unblock_code(int fd);

#endif
