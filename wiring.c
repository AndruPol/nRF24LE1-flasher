#include "wiring.h"
#include <string.h>

#include "ch.h"
#include "hal.h"

/*
 * SPI speed definitions.
 */
#define SPI_DIV2			0
#define SPI_DIV4			(SPI_CR1_BR_0)
#define SPI_DIV8			(SPI_CR1_BR_1)
#define SPI_DIV16			(SPI_CR1_BR_1 | SPI_CR1_BR_0)
#define SPI_DIV32			(SPI_CR1_BR_2)
#define SPI_DIV64			(SPI_CR1_BR_2 | SPI_CR1_BR_0)

#define HIGHSPEED_SPI_DIV	SPI_DIV2

#define	spip				SPID1
/*
 * SPI configuration structure.
 */
static const SPIConfig spicfg = {NULL, GPIOE, GPIOE_PIN15, SPI_DIV32};

/*
 * SPI bus exchange routine
 */
static void spi_exchange(uint8_t *tx, uint8_t *rx, size_t size) {
	/*
	 * Do exchange between device and MCU.
	 */

	spiSelect(&spip);                  /* Slave Select assertion.          */
	spiExchange(&spip, size, tx, rx);  /* Atomic transfer operations.      */
	spiUnselect(&spip);                /* Slave Select de-assertion.       */
}

/*
 * SPI bus exchange routine
 */
static void spi_send(uint8_t *tx, size_t size) {
	/*
	 * Do send to device.
	 */

	spiSelect(&spip);                  /* Slave Select assertion.          */
	spiSend(&spip, size, tx); 		   /* Atomic transfer operations.      */
	spiUnselect(&spip);                /* Slave Select de-assertion.       */
}

void wiring_init(void) {

	/*
	 * SPI1 I/O pins setup.
	 */
/*	palSetPadMode(GPIOA, 5, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);	// SCK		PA05 -> P0.5
	palSetPadMode(GPIOA, 6, PAL_MODE_ALTERNATE(5));					// MISO		PA06 -> P1.0
	palSetPadMode(GPIOA, 7, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);	// MOSI		PA07 -> P0.7
*/

	palSetPadMode(GPIOB, WIRING_NRF_PROG_PIN, PAL_MODE_OUTPUT_PUSHPULL);		// PROG		PB04 -> PROG
	palSetPadMode(GPIOB, WIRING_NRF_RESET_PIN, PAL_MODE_OUTPUT_PUSHPULL);		// RESET	PB05 -> RESET
	palClearPad(GPIOB, WIRING_NRF_PROG_PIN);
	palSetPad(GPIOB, WIRING_NRF_RESET_PIN);

	palSetPadMode(GPIOE, GPIOE_PIN15, PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);	// NSS	PC06 -> P1.1
	palSetPad(GPIOE, GPIOE_PIN15);

	spiStart(&spip, &spicfg);	      /* Setup transfer parameters.       */

/*	bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64);    // 4Mhz clock
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default
	
	bcm2835_gpio_fsel(WIRING_NRF_PROG_PIN, BCM2835_GPIO_FSEL_OUTP);	
	bcm2835_gpio_fsel(WIRING_NRF_RESET_PIN, BCM2835_GPIO_FSEL_OUTP);
*/

}

uint8_t wiring_write_then_read(uint8_t* out, uint16_t out_len, uint8_t* in, uint16_t in_len) {

//	uint8_t transfer_buf[out_len + in_len];
	unsigned int ret = 0;

//	memset(transfer_buf, 0, out_len + in_len);
	
/*	if (NULL != out) {
		memcpy(transfer_buf, out, out_len);
		spi_send(out, out_len);
		ret += out_len;
	}
*/
	if (NULL != in) {
		uint16_t n = (out_len >= in_len ? out_len : in_len);
		uint8_t txbuf[n+1];
		uint8_t rxbuf[n+1];
		memset(txbuf, 0, n+1);
		if (NULL != out)
			memcpy(txbuf, out, out_len);
		spi_exchange(txbuf, rxbuf, n+1);
		if (NULL != in)
			memcpy(in, rxbuf+out_len, in_len);
		ret = out_len+in_len;
	}
	else {
		spi_send(out, out_len);
		ret = out_len;
	}
//	bcm2835_spi_transfern(transfer_buf, ret);

//	memcpy(in, &transfer_buf[out_len], in_len);

	return ret;

}

void wiring_set_gpio_value(uint8_t pin, uint8_t state) {

	if (state == 0) {
		palClearPad(GPIOB, pin);
	}
	else {
		palSetPad(GPIOB, pin);
	}

}

void wiring_destroy(void) {

	spiStop(&spip);

}
