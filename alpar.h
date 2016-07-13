
#ifndef		_ALPAR_H_
#define		_ALPAR_H_


/* First Byte */
#define		ALPAR_ACK		0x60
#define		ALPAR_NAK		0xE0

/* Command Byte: PC->IFD */
#define     	CARD_COMMAND		0x00
#define     	SEND_NUM_MASK		0x0A
#define     	SET_CARD_BAUD_RATE	0x0B
#define     	IFS_REQUEST		0x0C
#define		SET_SERIAL_BAUD_RATE	0x0D
#define		SHOW_FIDI		0x0E
#define		NEGOTIATE		0x10
#define		SET_CLOCK_CARD		0x11
#define		POWER_OFF		0x4D
#define		POWER_UP_ISO		0x69
#define		SELECT_CARD		0x6A
#define		POWER_UP_3V		0x6D
#define		POWER_UP_5V		0x6E
#define		IDLE_MODE_LOW		0xA2
#define		POWER_DOWN_MODE		0xA3
#define		IDLE_MODE_HIGH		0xA4

/* Command byte; IFD->PC */
#define		CARD_EXT_OR_INS		0xA0
#define		RX_TIME_OUT		0xFF

/* Error bytes */
#define		LEN_TOO_SHORT		0x08
#define		THREE_CARD_ERR_T1	0x0A

#define		WRONG_APDU		0x20
#define		SHORT_APDU		0x21
#define		CARD_MUTE_T1		0x22
#define		BAD_NAD			0x23
#define		BAD_LRC			0x25
#define		RESYNCHRONIZED		0x26
#define		CHAIN_ABORTED		0x27
#define		BAD_PCB			0x28
#define		CARD_OVERFLOW		0x29

#define		NON_NEGOTIABLE_MODE	0x30
#define		PROTOCOL_NOT_T0_T1	0x31
#define		T1_NOT_ACCEPTED		0x32
#define		PPS_ANS_DIFF_REQ	0x33
#define		ERR_PCK			0x34
#define		BAD_CMD_PARM		0x35
#define		TB3_ABSENT		0x36
#define		PPS_NOT_ACCEPTED	0x37
#define		EARLY_ANSWER_ACTIVATION	0x38

#define		UNKNOWN_COMMAND		0x55

#define		CARD_MUTE_PON		0x80
#define		TIME_OUT		0x81
#define		PARITY_RCV_4_ERR	0x83
#define		PARITY_XMT_4_ERR	0x84
#define		BAD_FI_DI		0x86
#define		ATR_GT_19200		0x88
#define		CWI_NOT_SUPP		0x89
#define		BWI_NOT_SUPP		0x8A
#define		WI_NOT_SUPP		0x8B
#define		TC3_NOT_ACCEPT		0x8C
#define		ATR_PARITY_ERR		0x8D

#define		PARITY_ERR_3_T1		0x90
#define		SW1_NOT_6X_9X		0x91
#define		MODE_BYTE_TA2_1		0x92
#define		TB1_ABSENT_COLD_RST	0x93
#define		TB1_DIFF_0_COLD_RST	0x94
#define		IFSC_LT_10_IFSC_FF	0x95
#define		WRONG_TDI		0x96
#define		TB2_PRESENT_ATR		0x97
#define		TC1_NOT_COMPAT_CWT	0x98
#define		NOT_T1_CARD		0x9B

#define		PROCEDURE_BYTE_ERR	0xA0

#define		CARD_ABSENT		0xC0
#define		CHECKSUM_ERR		0xC3
#define		TS_NOT_3B_3F		0xC4
#define		ATR_NOT_SUPPORTED	0xC6
#define		VPP_NOT_SUPPORTED	0xC7

#define		BAD_CARD_NUM		0xE0
#define		CARD_CLK_F_NOT_ACC	0xE1
#define		UART_OVERFLOW		0xE2
#define		SUPPLY_VOLTAGE_DROPOFF	0xE3
#define		TEMPERATURE_ALARM	0xE4
#define		CARD1_DEACTIVATED	0xE5
#define		CARD2_DEACTIVATED	0xE6
#define		FRAMING_ERROR		0xE9

/* Recognized cards */
#define		PENNCASH		0
#define		DSPERSON		1
#define		PAYFLEX			2
#define		BULLCP8			3

#define		UNRECOGNIZED		0xff
#define		UNKNOWN_CLASS		0xff

/* Maximum number of cards */
#define		MAX_CARDS		4

/* Maximum length of ATR   */
#define		MAX_ATR_LEN		20

/* Table for recognizing card from ATR */
struct atr_table
{
    int size;
    unsigned char atr[MAX_ATR_LEN];
    unsigned char *card_name;
    unsigned char card_class;
};

/* Serial initialization / de-initializtion */
int serial_init(const char *device);
int serial_deinit(int fd);

/* Basic ALPAR send/recv */
short int alpar_recv(int fd, unsigned char *ack, unsigned char *command, 
            unsigned char **data, unsigned char *lrc );

int alpar_send(int fd, 
            unsigned char command,
            unsigned short int length,
            const unsigned char *data );

/* Higher layers of ALPAR */

int send_num_mask(int fd, char *result);
int set_card_baud_rate(int fd, unsigned char fidi, unsigned char cku);
int set_clock_card(int fd, unsigned char par);
int idle_mode_high(int fd);
int idle_mode_low(int fd);
int ifs_request(int fd, unsigned char ifsd);
int negotiate(int fd, unsigned char protocol, unsigned char fidi);
int power_down_mode(int fd);
int power_off(int fd);
int power_up_iso(int fd, char *atr);
int power_up_3V(int fd, char *atr);
int power_up_5V(int fd, char *atr);
int select_card(int fd, unsigned char cardno);
int set_serial_baud_rate(int fd, int par);
int show_fidi(int fd, char *fidi);
int card_command(int fd, const char *iso_cmd, int cmd_len, char *iso_resp);

#endif		/* _ALPAR_H_ */
