/*Program with 3 functions to handle MFRC522-RFID
program with 3 functions, depending on position of dip switches in Bolt 18F2550 board.
SW1=ON, the rest=OFF showSerialNumber(), reads serial number and transmit to serial port.
SW2=ON, the rest=OFF readDataHEX(), reads all memory in tag (64 lines) and transmits to serial in hex.
SW3=ON, the rest=OFF readDataASCII(), reads all tag (64 lines) and transmits to serial in ASCII.
Please note that, for this C18 project, in the Bolt 18F2550 standard template linker file "rm18f2550.lkr", 
the next 2 lines were modified to avoid an error when compiling:
CODEPAGE   NAME=vectors    START=0x800          END=0x0x849        PROTECTED
CODEPAGE   NAME=page       START=0x84A          END=0x7FFF
*/

#include <p18cxxx.h>
#include <delays.h>
#include <stdio.h>		// sprintf() library
#include <stdlib.h>		// atoi(),atof() library 
#include "18F2550BOLT.h"
#include "MFRC522-RFID-SPI.h"

extern void _startup( void ); // See c018i.c in your C18 compiler dir 
#pragma code _RESET_INTERRUPT_VECTOR = 0x000800 

void _reset( void ) 
{ 
    _asm goto _startup _endasm 
} 
#define SW1  PORTAbits.RA4
#define SW2  PORTAbits.RA5
#define SW3  PORTCbits.RC0
#define SW4  PORTCbits.RC1

char ok[2] = "OK";
void main(void)					//select function to run, and comment the rest.
{
ADCON1=0x0F; 	//DISABLE CONVERTERS A/D
CMCON=7;
TRISA=0X30;		//RA4,RA5 ARE INPUTS (DIP SWITCHES)
TRISC=0X0F;		//RC0,RC1 ARE INPUTS (DIP SWITCHES)
InitLCD();
ClearScreen();
PrintString(ok);
delay_ms(500);
	while(1)
	{
		if(SW1==0)
		{
		showSerialNumber();			//reads serial number and transmit to serial port
		}
		if(SW2==0)
		{
		readDataHEX();				//reads all memory in tag (64 lines) and transmits to serial in hex
		}
		if(SW3==0)
		{
		readDataASCII();			//reads all tag (64 lines) and transmits to serial in ASCII
		}
	}
	//clearTagsMemory();		//erases all tag memory
	//writeTagBlockMemory();	//writes in tag memory, only may write 1 sector at a time. 
} 

