### nRF24LE1 flash writer/programmer

A simple command-line interface with Nordic nRF24LE1 on STM32F4DISCOVERY board 

based on nrf24le1-libbcm2835 https://github.com/derekstavis/nrf24le1-libbcm2835

nRF24LE1 QFN32 (5×5 mm) - http://www.ebay.com/itm/310888537660?ssPageName=STRK:MEWNX:IT&_trksid=p3984.m1497.l2649

Connect to PC via board micro USB port, ChibiOS serial over USB driver on /dev/ttyACM0 (38400/8/N/1)

Pinout

```
SIGNAL  |STM32F4  |nRF24LE1 (QFN32)
        |DISCOVERY|
--------|---------|-----------------
SCK     |PA05     |P0.5   
MISO    |PA06     |P1.0   
MOSI    |PA07     |P0.7   
NSS     |PE15     |P1.1   
PROG    |PB04     |PROG   
RESET   |PB05     |RESET  
--------|---------|-----------------
VDD     |3V       |VDD    
GND     |GND      |GND			
--------|---------|-----------------

```

commands:

test	- test connection with nRF24LE1, you must receive if connections ok
```
ch> test
* FSR original
-> FSR.RDISMB: 0
-> FSR.INFEN: 0
-> FSR.RDYN: 0
-> FSR.WEN: 0
-> FSR.STP: 0
-> FSR.ENDEBUG: 0                                                              
* FSR after WREN, WEN must be 1                                                
-> FSR.RDISMB: 0                                                               
-> FSR.INFEN: 0                                                                
-> FSR.RDYN: 0                                                                 
-> FSR.WEN: 1                                                                  
-> FSR.STP: 0                                                                  
-> FSR.ENDEBUG: 0                                                              
* FSR after WRDIS, WEN must be 0                                               
-> FSR.RDISMB: 0                                                               
-> FSR.INFEN: 0                                                                
-> FSR.RDYN: 0                                                                 
-> FSR.WEN: 0                                                                  
-> FSR.STP: 0                                                                  
-> FSR.ENDEBUG: 0                                                              
```
dump [size]	- dump flash buffer contents, size <= 16384, default 16384 bytes

xmread [size] - receive flash buffer from PC via xmodem, size <= 16384, default 16384 bytes

xmsend [size] - transmit flash buffer to PC via xmodem, size <= 16384, default 16384 bytes

write	- write flash buffer into nRF24LE1 flash

read	- read nRF24LE1 flash into flash buffer

nvwrite - write flash 1024 bytes buffer into nRF24LE1 NVM data

nvread	- read nRF24LE1 1024 bytes NVM data into flash buffer
 
ipread	- read nRF24LE1 512 bytes infopage data into flash buffer

ipwrite - write flash buffer 512 bytes into nRF24LE1 infopage

erase_all - ERASE ALL nRF24LE1 flash data, NVM and infopage (at your own risk)
