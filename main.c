/*
 * NRF24LE1 flasher by A.Polyakov
*/

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"

#include "main.h"
#include "cmds.h"
#include "xmodem.h"
#include "nrf24le1.h"

char flash_buffer[FLASHSIZE];
BinarySemaphore transsem;

Thread *shelltp;

// called on kernel panic
void port_halt(void){
	port_disable();
	palSetPad(BOARDLED_GPIO, BOARDLED_BLUE); // turn on blue LED
	while(TRUE) {
	}
}

/*------------------------------------------------------------------------*
 * main                                                                   *
 *------------------------------------------------------------------------*/
int main(void) {

	static const ShellConfig shell_cfg = {
			(BaseSequentialStream *)&CON,
			commands
	};

	/*
	 * System initializations.
	 * - HAL initialization, this also initializes the configured device drivers
	 *   and performs the board-specific initializations.
	 * - Kernel initialization, the main() function becomes a thread and the
	 *   RTOS is active.
	 */
	halInit();
	chSysInit();

	palSetPadMode(BOARDLED_GPIO, BOARDLED_GREEN, PAL_MODE_OUTPUT_PUSHPULL);	/* green led */
	palSetPadMode(BOARDLED_GPIO, BOARDLED_RED, PAL_MODE_OUTPUT_PUSHPULL);	/* red led */
	palSetPadMode(BOARDLED_GPIO, BOARDLED_BLUE, PAL_MODE_OUTPUT_PUSHPULL);	/* blue led */

	sdStart(&CON, NULL);
	palSetPadMode(SD_GPIO, SD_TX_PIN, PAL_MODE_ALTERNATE(7) | PAL_STM32_OSPEED_HIGHEST);
	palSetPadMode(SD_GPIO, SD_RX_PIN, PAL_MODE_ALTERNATE(7));

	chprintf((BaseSequentialStream *)&CON,"\r\nNRF24LE1 FLASH WRITER\r\n");
	chprintf((BaseSequentialStream *)&CON,"Firmware version %s\r\n", FIRMWARE);

	chBSemInit(&transsem,FALSE);

    xmodem_init();
	nrf24le1_init();

	/*
     * Shell manager initialization.
     */
    shellInit();
    shelltp = shellCreate(&shell_cfg, SHELL_WA_SIZE, SHELL_PRIO);

	/*
	 * main loop.
	 */
	while (TRUE) {
		palTogglePad(BOARDLED_GPIO, BOARDLED_GREEN);
		if (!shelltp) {
			chBSemWait(&transsem);
			if (xmodem.state == XMODEM_RECEIVE) {
				if (xmodem.error == XMODEM_RECEIVE_ERROR) {
					chprintf((BaseSequentialStream *)&CON,"Xmodem receive error: status: %d\r\n", xmodem.status);
				}
				else {
					chprintf((BaseSequentialStream *)&CON,"Xmodem successfully received %d bytes\r\n", xmodem.status);
				}
				xmodem.state = XMODEM_IDLE;
			}
			if (xmodem.state == XMODEM_TRANSMIT) {
				if (xmodem.error == XMODEM_TRANSMIT_ERROR)
					chprintf((BaseSequentialStream *)&CON,"Xmodem transmit error: status: %d\r\n", xmodem.status);
				else
					chprintf((BaseSequentialStream *)&CON,"Xmodem successfully transmitted %d bytes\r\n", xmodem.status);
				xmodem.state = XMODEM_IDLE;
			}
			shelltp = shellCreate(&shell_cfg, SHELL_WA_SIZE, SHELL_PRIO);
			chBSemSignal(&transsem);
		}
		else if (chThdTerminated(shelltp)) {
			chThdRelease(shelltp);    /* Recovers memory of the previous shell.   */
			shelltp = NULL;           /* Triggers spawning of a new shell.        */
		}
		chThdSleepMilliseconds(500);
	}
}
