/*
 * cmds.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ch.h"
#include "hal.h"

#include "shell.h"
#include "chprintf.h"

#include "main.h"
#include "xmodem.h"
#include "nrf24le1.h"

/*
 * command functions start here
*/

/*
 * cmd mem
*/
void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
	size_t n, size;

	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: mem\r\n");
		return;
	}
	n = chHeapStatus(NULL, &size);
	chprintf(chp, "core free memory : %u bytes\r\n", chCoreStatus());
	chprintf(chp, "heap fragments   : %u\r\n", n);
	chprintf(chp, "heap free total  : %u bytes\r\n", size);
}

/*
 * cmd xmread - receive flash buffer from PC
*/
void cmd_xmread(BaseSequentialStream *chp, int argc, char *argv[]) {
	(void)argv;
	if (argc > 1) {
		chprintf(chp, "Usage: xmread [size]\r\n");
		return;
	}

	unsigned int size = FLASHSIZE;
	if (argc == 1) {
		size = (atoi(argv[0]) > FLASHSIZE ? FLASHSIZE : atoi(argv[0]));
	}

	memset(&flash_buffer, 0, FLASHSIZE);

	xmodem_t *xm_p = &xmodem;
	xmodem.size = size;
	chprintf((BaseSequentialStream *)&CON,"Send data using the xmodem protocol from your terminal emulator now...\r\n");

	chMsgSend(xReceiveThread_p, (msg_t) xm_p);
	shellExit(RDY_OK);
}

/*
 * cmd xmsend - transmit flash buffer to PC
*/
void cmd_xmsend(BaseSequentialStream *chp, int argc, char *argv[]) {
	(void)argv;
	if (argc > 1) {
		chprintf(chp, "Usage: xmsend [size]\r\n");
		return;
	}

	unsigned int size = FLASHSIZE;
	if (argc == 1) {
		size = (atoi(argv[0]) > FLASHSIZE ? FLASHSIZE : atoi(argv[0]));
	}

	xmodem_t *xm_p = &xmodem;
	xmodem.size = size;

	chprintf((BaseSequentialStream *)&CON,"Prepare your terminal emulator to receive data now...\r\n");

	chMsgSend(xTransmitThread_p, (msg_t) xm_p);
	shellExit(RDY_OK);
}

/*
 * cmd dump flash buffer
*/
void cmd_dump(BaseSequentialStream *chp, int argc, char *argv[]) {
	(void)argv;
	if (argc > 1) {
		chprintf(chp, "Usage: dump [size]\r\n");
		return;
	}

	unsigned int size = FLASHSIZE;
	if (argc == 1) {
		size = (atoi(argv[0]) > FLASHSIZE ? FLASHSIZE : atoi(argv[0]));
	}

	char *p = flash_buffer;
	unsigned int i=0;
	while (i<size) {
		chprintf((BaseSequentialStream *)&CON,"0x%.04X %.02X %.02X %.02X %02X  %.02X %.02X %.02X %.02X   %.02X %.02X %.02X %.02X  %.02X %.02X %.02X %.02X\r\n", i,
				(char) *p,     (char) *(p+1), (char) *(p+2), (char) *(p+3),
				(char) *(p+4), (char) *(p+5), (char) *(p+6), (char) *(p+7),
				(char) *(p+8), (char) *(p+9), (char) *(p+10),(char) *(p+11),
				(char) *(p+12),(char) *(p+13),(char) *(p+14),(char) *(p+15)
				);
		i = i+16;
		p = p+16;
	}
}

/*
 * cmd test
*/
void cmd_test(BaseSequentialStream *chp, int argc, char *argv[]) {
	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: test\r\n");
		return;
	}
	wiring_init();
	enable_program(1);
	da_test_show();
	enable_program(0);
	wiring_destroy();
}

/*
 * cmd write
*/
void cmd_write(BaseSequentialStream *chp, int argc, char *argv[]) {
	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: write\r\n");
		return;
	}

	palSetPad(BOARDLED_GPIO, BOARDLED_RED); // turn on red led
	wiring_init();
	enable_program(1);
	uhet_write(flash_buffer, FLASHSIZE);
	enable_program(0);
	palClearPad(BOARDLED_GPIO, BOARDLED_RED); // turn off red led
	wiring_destroy();
}
/*
 * cmd read
*/
void cmd_read(BaseSequentialStream *chp, int argc, char *argv[]) {
	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: read\r\n");
		return;
	}

	memset(flash_buffer, 0x0, FLASHSIZE);
	wiring_init();
	enable_program(1);
	uhet_read(flash_buffer, FLASHSIZE);
	enable_program(0);
	wiring_destroy();
}

/*
 * cmd nvwrite
*/
void cmd_nvwrite(BaseSequentialStream *chp, int argc, char *argv[]) {
	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: nvwrite\r\n");
		return;
	}

	palSetPad(BOARDLED_GPIO, BOARDLED_RED); // turn on red led
	wiring_init();
	enable_program(1);
	da_nvm_normal_store(flash_buffer, NVMSIZE);
	enable_program(0);
	wiring_destroy();
	palClearPad(BOARDLED_GPIO, BOARDLED_RED); // turn off red led
}
/*
 * cmd nvread
*/
void cmd_nvread(BaseSequentialStream *chp, int argc, char *argv[]) {
	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: nvread\r\n");
		return;
	}

	memset(flash_buffer, 0x0, FLASHSIZE);
	wiring_init();
	enable_program(1);
	da_nvm_normal_show(flash_buffer);
	enable_program(0);
	wiring_destroy();
}
/*
 * cmd ipread
*/
void cmd_ipread(BaseSequentialStream *chp, int argc, char *argv[]) {
	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: ipread\r\n");
		return;
	}

	memset(flash_buffer, 0x0, FLASHSIZE);
	wiring_init();
	enable_program(1);
	da_infopage_show(flash_buffer);
	enable_program(0);
	wiring_destroy();
}
/*
 * cmd ipwrite
*/
void cmd_ipwrite(BaseSequentialStream *chp, int argc, char *argv[]) {
	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: ipwrite\r\n");
		return;
	}

	palSetPad(BOARDLED_GPIO, BOARDLED_RED); // turn on red led
	wiring_init();
	enable_program(1);
	da_infopage_store(flash_buffer, NRF_PAGE_SIZE);
	enable_program(0);
	wiring_destroy();
	palClearPad(BOARDLED_GPIO, BOARDLED_RED); // turn off red led
}
/*
 * cmd erase_all
*/
void cmd_erase_all(BaseSequentialStream *chp, int argc, char *argv[]) {
	(void)argv;
	if (argc > 0) {
		chprintf(chp, "Usage: erase_all\r\n");
		return;
	}

	palSetPad(BOARDLED_GPIO, BOARDLED_RED); // turn on red led
	wiring_init();
	enable_program(1);
	da_erase_all_store();
	enable_program(0);
	wiring_destroy();
	palClearPad(BOARDLED_GPIO, BOARDLED_RED); // turn off red led
}
