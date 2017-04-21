/*
 * main.h
 */

#ifndef MAIN_H_
#define MAIN_H_
#ifdef __cplusplus
extern "C" {
#endif

#define	FIRMWARE		"0.1"

#define FLASHSIZE		16384
#define NVMSIZE			1024
extern char flash_buffer[FLASHSIZE];

#define BOARDLED_GPIO	GPIOD
#define BOARDLED_GREEN	GPIOD_LED4	// green led
#define BOARDLED_ORANGE	GPIOD_LED3	// orange led
#define BOARDLED_RED	GPIOD_LED5	// red led
#define BOARDLED_BLUE	GPIOD_LED6	// blue led

extern BinarySemaphore transsem;

#define	CON				SDU1		//CONSOLE
#define SD_GPIO			GPIOD
#define	SD_TX_PIN		5			//PD.05
#define SD_RX_PIN		6			//PD.06

// WORKING AREAS
#define SHELL_WA_SIZE   THD_WA_SIZE(34000)

// PROCESS PRIORITY
#define SHELL_PRIO		NORMALPRIO

extern Thread   *shelltp;
extern SerialUSBDriver SDU1;

#ifdef __cplusplus
}
#endif
#endif /* MAIN_H_ */
