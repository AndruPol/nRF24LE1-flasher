#ifndef __NRF24LE1_H__
#define __NRF24LE1_H__

#include <stdlib.h>
#include "nrf24le1.h"
#include "wiring.h"

#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "main.h"

#define debug(fmt, args...) \
{ \
	chprintf((BaseSequentialStream *)&CON, "[" NAME "] %s: " fmt "\r\n", __func__, ##args); \
}

#define NAME "nrf24le1"

void uhet_record_init(void);
void uhet_record_end(void);

void _erase_all(void);
void _erase_page(unsigned i);
void _erase_program_pages(void);

int _enable_program = 0;

#define write_then_read(out,n_out,in,n_in) \
({ \
	int __ret = 0; \
\
	__ret = wiring_write_then_read(out,n_out,in,n_in); \
	if (0 > __ret){ \
		debug("failure on operation write_then_read"); \
	} \
\
	__ret; \
})

void enable_program(uint8_t state) {
	if (state == 1) {
		wiring_set_gpio_value(GPIO_PROG, 1);
		mdelay(10);
		wiring_set_gpio_value(GPIO_RESET, 0);
		udelay(5);
		wiring_set_gpio_value(GPIO_RESET, 1);
		mdelay(2);
	}
	else {
		wiring_set_gpio_value(GPIO_PROG, 0);
		mdelay(1);
		wiring_set_gpio_value(GPIO_RESET, 0);
		udelay(1);
		wiring_set_gpio_value(GPIO_RESET, 1);
	}
	_enable_program = state;
}

void nrf24le1_init(void) {
	debug("init nRF24LE1"); // Inicializando
	wiring_init();
}

void _wait_for_ready(void) {
	uint8_t cmd[1] = { SPICMD_RDSR };
	uint8_t fsr[1] = { 0xAA };
	int count = 0;

	do {
		if (count == 1000) {
			debug("can not wait forever, FSR: 0x%X", *fsr);
			return;
		}
		count++;

		wiring_write_then_read(cmd, 1, fsr, 1);
		udelay(500);

	} while (*fsr & FSR_RDYN);
}

int _enable_infopage_access(void) {
	uint8_t cmd[2];
	uint8_t in[1];
	uint8_t fsr_orig;

	// read fsr
	cmd[0] = SPICMD_RDSR;
	write_then_read(cmd, 1, in, 1);
	fsr_orig = in[0];

	// fsr.INFEN = 1
	cmd[0] = SPICMD_WRSR;
	cmd[1] = fsr_orig | FSR_INFEN;
	write_then_read(cmd, 2, NULL, 0);

	// read fsr
	cmd[0] = SPICMD_RDSR;
	write_then_read(cmd, 1, in, 1);

	// comparando escrita
	if (in[0] != (fsr_orig | FSR_INFEN)) {
		debug("failure enable access to infopage %X %X", fsr_orig,
		      in[0]);
		return -EINVAL;
	}

	return 0;
}

int _read_infopage(char *buf) {
	int i; int ret = 0;

	uint8_t cmd[3];
	uint8_t in[N_BYTES_FOR_READ];

	uint16_t *addr = (uint16_t *) (cmd + 1);
	char *p = buf;

	cmd[0] = SPICMD_READ;
	for (i = 0; i < NRF_PAGE_SIZE; i += N_BYTES_FOR_READ) {

		*addr = htons(i);
		ret = write_then_read(cmd, 3, in, N_BYTES_FOR_READ);
		if (0 != ret)
			return ret;

		memcpy(p, in, N_BYTES_FOR_READ);

		p += N_BYTES_FOR_READ;
	}

	debug("number of bytes read: %i", p - buf);

	return p - buf;		// numero de bytes lidos;
}

int _read_nvm_normal(char *buf) {
	int i;
	int ret = 0;

	uint8_t cmd[3];
	uint8_t in[N_BYTES_FOR_READ];

	uint16_t *addr = (uint16_t *) (cmd + 1);
	char *p = buf;

	cmd[0] = SPICMD_READ;
	for (i = 0; i < NVM_NORMAL_MEM_SIZE; i += N_BYTES_FOR_READ) {

		*addr = htons(i + NVM_NORMAL_PAGE0_INI_ADDR);
		ret = write_then_read(cmd, 3, in, N_BYTES_FOR_READ);
		if (0 != ret)
			return ret;

		memcpy(p, in, N_BYTES_FOR_READ);

		p += N_BYTES_FOR_READ;
	}

	debug("number of bytes read: %i", p - buf);

	return p - buf;		// numero de bytes lidos;
}

int _disable_infopage_access(void) {
	uint8_t cmd[2];
	uint8_t in[1];
	uint8_t fsr_orig;

	// read fsr
	cmd[0] = SPICMD_RDSR;
	write_then_read(cmd, 1, in, 1);
	fsr_orig = in[0];

	// fsr.INFEN = 0
	cmd[0] = SPICMD_WRSR;
	cmd[1] = fsr_orig & ~(FSR_INFEN);
	write_then_read(cmd, 2, NULL, 0);

	// read fsr
	cmd[0] = SPICMD_RDSR;
	write_then_read(cmd, 1, in, 1);

	// comparando escrita
	if (in[0] != (fsr_orig & ~(FSR_INFEN))) {
		debug("failure disable access to infopage %X %X", fsr_orig, in[0]);
		return -EINVAL;
	}

	return 0;
}

ssize_t da_enable_program_show(void) {
//	int ret = mutex_trylock(&mutex);
//	if (0 == ret)
//		return -ERESTARTSYS;

	chprintf((BaseSequentialStream *)&CON, "%i\r\n", _enable_program);

//	mutex_unlock(&mutex);
	return 0;
}

ssize_t da_erase_all_store(void) {
	int ret = 0;

//	ret = mutex_trylock(&mutex);
//	if (0 == ret)
//		return -ERESTARTSYS;

	if (0 == _enable_program) {
		debug("failure, enable_program = 0");
		ret = -EINVAL;
		goto end;
	}

	_erase_all();

end:
//	mutex_unlock(&mutex);
	return ret;
}

ssize_t da_test_show(void) {
	uint8_t cmd;
	uint8_t fsr;
	int ret = 0;

//	ret = mutex_trylock(&mutex);
//	if (0 == ret)
//		return -ERESTARTSYS;

	if (0 == _enable_program) {
		debug("failure, enable_program = 0");
		ret = -EINVAL;
		goto end;
	}

	cmd = SPICMD_RDSR;
	write_then_read(&cmd, 1, &fsr, 1);
	chprintf((BaseSequentialStream *)&CON, "* FSR original\r\n");
	chprintf((BaseSequentialStream *)&CON, "-> FSR.RDISMB: %i\r\n",  (fsr & FSR_RDISMB ? 1 : 0));
	chprintf((BaseSequentialStream *)&CON, "-> FSR.INFEN: %i\r\n",   (fsr & FSR_INFEN ? 1 : 0));
	chprintf((BaseSequentialStream *)&CON, "-> FSR.RDYN: %i\r\n",    (fsr & FSR_RDYN ? 1 : 0));
	chprintf((BaseSequentialStream *)&CON, "-> FSR.WEN: %i\r\n",     (fsr & FSR_WEN ? 1 : 0));
	chprintf((BaseSequentialStream *)&CON, "-> FSR.STP: %i\r\n",     (fsr & FSR_STP ? 1 : 0));
	chprintf((BaseSequentialStream *)&CON, "-> FSR.ENDEBUG: %i\r\n", (fsr & FSR_ENDEBUG ? 1 : 0));

	cmd = SPICMD_WREN;
	write_then_read(&cmd, 1, NULL, 0);

	cmd = SPICMD_RDSR;
	write_then_read(&cmd, 1, &fsr, 1);

	chprintf((BaseSequentialStream *)&CON, "* FSR after WREN, WEN must be 1\r\n");
	chprintf((BaseSequentialStream *)&CON, "-> FSR.RDISMB: %i\r\n", (fsr & FSR_RDISMB ? 1 : 0));
	chprintf((BaseSequentialStream *)&CON, "-> FSR.INFEN: %i\r\n",  (fsr & FSR_INFEN ? 1 : 0));
	chprintf((BaseSequentialStream *)&CON, "-> FSR.RDYN: %i\r\n",   (fsr & FSR_RDYN ? 1 : 0));
	chprintf((BaseSequentialStream *)&CON, "-> FSR.WEN: %i\r\n",    (fsr & FSR_WEN ? 1 : 0));
	chprintf((BaseSequentialStream *)&CON, "-> FSR.STP: %i\r\n",    (fsr & FSR_STP ? 1 : 0));
	chprintf((BaseSequentialStream *)&CON, "-> FSR.ENDEBUG: %i\r\n",(fsr & FSR_ENDEBUG ? 1 : 0));

	cmd = SPICMD_WRDIS;
	write_then_read(&cmd, 1, NULL, 0);

	cmd = SPICMD_RDSR;
	write_then_read(&cmd, 1, &fsr, 1);
	chprintf((BaseSequentialStream *)&CON, "* FSR after WRDIS, WEN must be 0\r\n");
	chprintf((BaseSequentialStream *)&CON, "-> FSR.RDISMB: %i\r\n", (fsr & FSR_RDISMB ? 1 : 0));
    chprintf((BaseSequentialStream *)&CON, "-> FSR.INFEN: %i\r\n",  (fsr & FSR_INFEN ? 1 : 0));
    chprintf((BaseSequentialStream *)&CON, "-> FSR.RDYN: %i\r\n",   (fsr & FSR_RDYN ? 1 : 0));
	chprintf((BaseSequentialStream *)&CON, "-> FSR.WEN: %i\r\n",    (fsr & FSR_WEN ? 1 : 0));
	chprintf((BaseSequentialStream *)&CON, "-> FSR.STP: %i\r\n",    (fsr & FSR_STP ? 1 : 0));
	chprintf((BaseSequentialStream *)&CON, "-> FSR.ENDEBUG: %i\r\n",(fsr & FSR_ENDEBUG ? 1 : 0));

end:
//	mutex_unlock(&mutex);
	return ret;
}

int _write_infopage(const char *buf) {
	int i;
	const uint8_t *infopage = NULL;
	int error_count = 0;

	uint8_t cmd[3 + N_BYTES_FOR_WRITE];
	uint16_t *addr = (uint16_t *) (cmd + 1);

	infopage = buf;

	for (i = 0; i < NRF_PAGE_SIZE; i += N_BYTES_FOR_WRITE) {

		cmd[0] = SPICMD_WREN;
		if (0 > write_then_read(cmd, 1, NULL, 0))
			debug("failure on SPICMD_WREN");

		_wait_for_ready();

		cmd[0] = SPICMD_PROGRAM;
		*addr = htons(i);
		memcpy(cmd + 3, infopage, N_BYTES_FOR_WRITE);

		if (0 != write_then_read(cmd, 3 + N_BYTES_FOR_WRITE, NULL, 0))
			error_count++;

		infopage += N_BYTES_FOR_WRITE;
		_wait_for_ready();
	}

	return (error_count > 0) ? -error_count : i;
}

int _write_nvm_normal(const char *buf) {
	int i;
	const uint8_t *mem = NULL;
	int error_count = 0;

	uint8_t cmd[3 + N_BYTES_FOR_WRITE];
	uint16_t *addr = (uint16_t *) (cmd + 1);

	mem = buf;

	for (i = 0; i < NVM_NORMAL_MEM_SIZE; i += N_BYTES_FOR_WRITE) {

		cmd[0] = SPICMD_WREN;
		if (0 > write_then_read(cmd, 1, NULL, 0))
			debug("failure on SPICMD_WREN");

		_wait_for_ready();

		cmd[0] = SPICMD_PROGRAM;
		*addr = htons(i + NVM_NORMAL_PAGE0_INI_ADDR);
		memcpy(cmd + 3, mem, N_BYTES_FOR_WRITE);

		if (0 != write_then_read(cmd, 3 + N_BYTES_FOR_WRITE, NULL, 0))
			error_count++;

		mem += N_BYTES_FOR_WRITE;
		_wait_for_ready();
	}

	return (error_count > 0) ? -error_count : i;
}

ssize_t da_infopage_store(const char *buf, size_t count) {
	int ret = 0;
	int size = -1;

//	ret = mutex_trylock(&mutex);
//	if (0 == ret) {
//		return -ERESTARTSYS;
//	}

	if (0 == _enable_program) {
		debug("failure, enable_program = 0");
		ret = -EINVAL;
		goto end;
	}

	if (NRF_PAGE_SIZE != count) {
		debug("size infopage(%i) different NRF_PAGE_SIZE(%i)", count, NRF_PAGE_SIZE);
		ret = -EINVAL;
		goto end;
	}

	ret = _enable_infopage_access();
	if (0 != ret)
		goto end;

	_erase_page(0);

	debug("starting writing memory");
	ret = _write_infopage(buf);
	if (0 > ret) {
		debug("number of errors in the writing to infopage: %i", -1 * ret);
	} else {
		debug("bytes written to infopage: %i", ret);
		size = ret;
	}
	debug("order of writing the memory");

	ret = _disable_infopage_access();
	if (0 != ret) {
		debug("failure disable access to infopage");
		goto end;
	}

	ret = size;

end:
//	mutex_unlock(&mutex);
	return ret;
}

ssize_t da_infopage_show(char *buf) {
	int ret;
	int size;

//	if (0 == mutex_trylock(&mutex))
//		return -ERESTARTSYS;

	if (0 == _enable_program) {
		debug("failure, enable_program = 0");
		ret = -EINVAL;
		goto end;
	}

	debug("begin");

	ret = _enable_infopage_access();
	if (0 != ret)
		goto end;

	size = _read_infopage(buf);
	if (0 > size) {
		debug("failure reading infopage, size: %i", size);
	}

	ret = _disable_infopage_access();
	if (0 != ret) {
		debug("failure disable access to infopage");
		goto end;
	}

	ret = size;

end:
//	mutex_unlock(&mutex);

	debug("end");
	return ret;
}

ssize_t da_nvm_normal_show(char* buf) {
	int ret;
	int size;

//	if (0 == mutex_trylock(&mutex))
//		return -ERESTARTSYS;

	if (0 == _enable_program) {
		debug("failure, enable_program = 0");
		ret = -EINVAL;
		goto end;
	}

	debug("begin");

	size = _read_nvm_normal(buf);
	if (0 > size) {
		debug("failure read infopage, size: %i", size);
	}

	ret = size;

end:
//	mutex_unlock(&mutex);

	debug("end");
	return ret;
}

ssize_t da_nvm_normal_store(const char *buf, size_t count) {
	int ret = 0;
	int size = -1;

//	ret = mutex_trylock(&mutex);
//	if (0 == ret) {
//		return -ERESTARTSYS;
//	}

	if (0 == _enable_program) {
		debug("failure, enable_program = 0");
		ret = -EINVAL;
		goto end;
	}

	if (NVM_NORMAL_MEM_SIZE != count) {
		debug
		    ("image size(%i) different NVM_NORMAL_MEM_SIZE(%i)",
		     count, NVM_NORMAL_MEM_SIZE);
		ret = -EINVAL;
		goto end;
	}

	_erase_page(NVM_NORMAL_PAGE0);
	_erase_page(NVM_NORMAL_PAGE1);

	debug("starting writing memory");
	ret = _write_nvm_normal(buf);
	if (0 > ret) {
		debug("number of errors in writing nvm_normal: %i", -1 * ret);
	} else {
		debug("bytes written: %i", ret);
		size = ret;
	}
	debug("order of writing the memory");

	ret = size;

end:
//	mutex_unlock(&mutex);
	return ret;
}


int _toc_toc_tem_alguem_ae(void) {
	uint8_t out[1] = { 0 };
	uint8_t fsr_after_wren;
	uint8_t fsr_after_wrdis;

	out[0] = SPICMD_WREN;
	write_then_read(out, 1, NULL, 0);

	out[0] = SPICMD_RDSR;
	write_then_read(out, 1, &fsr_after_wren, 1);

	out[0] = SPICMD_WRDIS;
	write_then_read(out, 1, NULL, 0);

	out[0] = SPICMD_RDSR;
	write_then_read(out, 1, &fsr_after_wrdis, 1);

	if ((0 != (fsr_after_wren & FSR_WEN)) &&
	    (0 == (fsr_after_wrdis & FSR_WEN)))
		return 0;

	return -EINVAL;
}

void _erase_all(void) {
	uint8_t cmd[1];
	int ret;

	cmd[0] = SPICMD_WREN;
	write_then_read(cmd, 1, NULL, 0);
	_wait_for_ready();

	cmd[0] = SPICMD_ERASEALL;
	ret = write_then_read(cmd, 1, NULL, 0);
	if (0 == ret)
		debug("erasing entire flash");

	_wait_for_ready();
}


uint8_t __enable_wren(void) {
    uint8_t cmd, fsr;

	cmd = SPICMD_WREN;
	write_then_read(&cmd, 1, NULL, 0);

	cmd = SPICMD_RDSR;
	write_then_read(&cmd, 1, &fsr, 1);

	if((fsr & FSR_WEN ? 1 : 0) == 0)
	{
		chprintf((BaseSequentialStream *)&CON, "Writing can not be enabled -> FSR.WEN: %i\r\n", (fsr & FSR_WEN ? 1 : 0));
		return 0;
	}

	return 1;
}

void _erase_page(unsigned i) {
	uint8_t cmd[2];
	int ret;

	if(!__enable_wren()) return;

	_wait_for_ready();

	cmd[0] = SPICMD_ERASEPAGE;
	cmd[1] = i;
	ret = write_then_read(cmd, 2, NULL, 0);

	if (0 == ret) chprintf((BaseSequentialStream *)&CON, "erasing page: %i\r\n", i);

	_wait_for_ready();
}

void _erase_program_pages(void) {
	unsigned i;


	for (i = 0; i < N_PAGES; i++) {
		_erase_page(i);
	}
}

ssize_t uhet_write(char *buf, size_t count) {
	unsigned long ret;
	uint8_t cmd[3 + NRF_PAGE_SIZE];
	uint8_t  i =0, k = 0;
	uint16_t addr = 0;

	if (0 == _enable_program) {
		debug("trying to write with enable_program = 0");
		return -EINVAL;
	}

	ret = _toc_toc_tem_alguem_ae();
	if (0 != ret) {
		debug("flash not responding."); // flash nao responde :/
		return -EINVAL;
	}

	_erase_program_pages(); /* apaga primeiro */

	_wait_for_ready();

	for(i = 0; i < N_PAGES; i++)
	{
		cmd[0] = SPICMD_PROGRAM;
		cmd[2] = (uint8_t)(addr & 0x00ff);
		cmd[1] = (uint8_t)((addr & 0xff00) >> 8);


		chprintf((BaseSequentialStream *)&CON, "ADDR: %02X %02X\r\n",((addr & 0xff00) >> 8), (addr & 0x00ff));
		memcpy(cmd + 3, &buf[addr], NRF_PAGE_SIZE);

		if(!__enable_wren()) return;

		chprintf((BaseSequentialStream *)&CON, "b0:%02X    b1:%02X   b2: %02X\r\n",cmd[0], cmd[2], cmd[1]);
		write_then_read(cmd, 3 + NRF_PAGE_SIZE, NULL, 0);

		addr = addr + NRF_PAGE_SIZE; /* Atualizando o endereco */
		_wait_for_ready();
	}

	return count;
}

ssize_t uhet_read(char* buf, size_t count) {

	if (0 == _enable_program) {
		debug("failure, enable_program = 0");
		return -EINVAL;
	}

	chprintf((BaseSequentialStream *)&CON, "The amount of data to be read: %i   %d\r\n", count, MAX_FIRMWARE_SIZE);
	// lendo da flash
	{
		uint8_t cmd[3];
		uint8_t *byte;
		uint16_t *addr = (uint16_t *) (cmd + 1);
		//uint8_t data[count];
		int i;

		cmd[0] = SPICMD_READ;
		cmd[1] = 0;
		cmd[2] = 0;

		write_then_read(cmd, 3, buf, count);

		debug
		    ("read addr: 0x%p, pack header: 0x%X 0x%X 0x%X,\r\n bytes read: %i",
		     addr, cmd[0], cmd[1], cmd[2], count);

		return count;
	}

	return 0;
}

#endif
