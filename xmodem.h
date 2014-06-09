extern int xmodemReceive(unsigned char *dest, int destsz);
extern int xmodemTransmit(unsigned char *src, int srcsz);
extern unsigned short crc16tab[];
extern unsigned short crc16_ccitt(const void *buf, int len);
//extern unsigned char xbuff[];

/*
 * ChibiOS specific block start
 */
#include "ch.h"

typedef enum {
	XMODEM_NO_ERROR,
	XMODEM_RECEIVE_ERROR,
	XMODEM_TRANSMIT_ERROR
} xmodem_error_t;

typedef enum {
	XMODEM_IDLE,
	XMODEM_RECEIVE,
	XMODEM_TRANSMIT
} xmodem_state_t;

typedef struct _xmodem_t xmodem_t;
struct _xmodem_t {
	xmodem_error_t error;
	xmodem_state_t state;
	int status;
	unsigned int size;
};

extern xmodem_t xmodem;
extern Thread *xReceiveThread_p;
extern Thread *xTransmitThread_p;

// init xmodem processes
void xmodem_init(void);
/*
 * ChibiOS specific block end
 */
