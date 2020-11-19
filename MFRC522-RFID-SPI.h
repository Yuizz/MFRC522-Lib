/*
 * Name: MFRC522-C18.c
 * Create date: may 2014
 * Moises Melendez, sesowm@hotmail.com	
 * Mifare1 searching card  ->  prevent conflict  ->  select card  ->  read / write

 * Rev. 1 made by jb: all putcUSART('\n') were replaced with putcUSART('\r') for compatibility with Hyperterminal
 * In Hyperterminal, user must also configure insertion of a line feed every after receiving a return character. 
 * So every line starts exactly on the left of the hyperterminal window.

 * Rev. 2 made by jb: modifications to showserialnumber() function to send characters to serial port;

 * Program that reads the serial number of a tag MIFARE 1 S50 and by a reader MF522-AN controlled 
 * by BOLT PIC18F2550 system then displays it in a serial terminal. 
 *	
 * Shows the whole content of the TAG's memory and send it to a serial terminal in hex and ASCII 
 * format.
 *
 * Writes data to TAG's data blocks (0 - 63) including sector trailers.
 *
 *
 *						J5							CON8
 *
 *
 *						GND _______________________ M-GND
 *						+5V ___
 *						.      |__3.3V Regulator___ M+3.3V
 *						.
 *						.
 *		BOLT SYSTEM		RB2	_______________________ SS		MF522-AN device			
 *						RB3 _______________________ MISO
 *						RB4	_______________________ RST		
 *						RB6	_______________________ SCK
 *						RB7	_______________________ MOSI	
 * 				
 *	 
 *						LCD				
 */


#include <usart.h>
#include <sw_spi.h>

//#include "18F2550BOLT.h"			//universal library BOLT
//#include "ADC-BOLT.h"				//Bolt-ADC-Channel-4 library  
//#include "PUERTO-SERIAL-BOLT.h"

#define	uchar	unsigned char
#define	uint	unsigned int

//data array maxium length
#define MAX_LEN 16

//signal RST in RB4	 
#define RST PORTBbits.RB4

//MF522 command bits
#define PCD_IDLE              0x00               //NO action; cancel current commands
#define PCD_AUTHENT           0x0E               //verify password key
#define PCD_RECEIVE           0x08               //receive data
#define PCD_TRANSMIT          0x04               //send data
#define PCD_TRANSCEIVE        0x0C               //send and receive data
#define PCD_RESETPHASE        0x0F               //reset
#define PCD_CALCCRC           0x03               //CRC check and caculation

//Mifare_One card command bits
#define PICC_REQIDL           0x26               //Search the cards that not into sleep mode in the antenna area 
#define PICC_REQALL           0x52               //Search all the cards in the antenna area
#define PICC_ANTICOLL         0x93               //prevent conflict
#define PICC_SElECTTAG        0x93               //select card
#define PICC_AUTHENT1A        0x60               //verify A password key
#define PICC_AUTHENT1B        0x61               //verify B password key
#define PICC_READ             0x30               //read 
#define PICC_WRITE            0xA0               //write
#define PICC_DECREMENT        0xC0               //deduct value
#define PICC_INCREMENT        0xC1               //charge up value
#define PICC_RESTORE          0xC2               //Restore data into buffer
#define PICC_TRANSFER         0xB0               //Save data into buffer
#define PICC_HALT             0x50               //sleep mode

//THe mistake code that return when communicate with MF522
#define MI_OK                 0
#define MI_NOTAGERR           1
#define MI_ERR                2

//------------------MFRC522 register ----------------------
//Page 0:Command and Status
#define     Reserved00            0x00    
#define     CommandReg            0x01    
#define     CommIEnReg            0x02    
#define     DivlEnReg             0x03    
#define     CommIrqReg            0x04    
#define     DivIrqReg             0x05
#define     ErrorReg              0x06    
#define     Status1Reg            0x07    
#define     Status2Reg            0x08    
#define     FIFODataReg           0x09
#define     FIFOLevelReg          0x0A
#define     WaterLevelReg         0x0B
#define     ControlReg            0x0C
#define     BitFramingReg         0x0D
#define     CollReg               0x0E
#define     Reserved01            0x0F
//Page 1:Command     
#define     Reserved10            0x10
#define     ModeReg               0x11
#define     TxModeReg             0x12
#define     RxModeReg             0x13
#define     TxControlReg          0x14
#define     TxAutoReg             0x15
#define     TxSelReg              0x16
#define     RxSelReg              0x17
#define     RxThresholdReg        0x18
#define     DemodReg              0x19
#define     Reserved11            0x1A
#define     Reserved12            0x1B
#define     MifareReg             0x1C
#define     Reserved13            0x1D
#define     Reserved14            0x1E
#define     SerialSpeedReg        0x1F
//Page 2:CFG    
#define     Reserved20            0x20  
#define     CRCResultRegM         0x21
#define     CRCResultRegL         0x22
#define     Reserved21            0x23
#define     ModWidthReg           0x24
#define     Reserved22            0x25
#define     RFCfgReg              0x26
#define     GsNReg                0x27
#define     CWGsPReg	          0x28
#define     ModGsPReg             0x29
#define     TModeReg              0x2A
#define     TPrescalerReg         0x2B
#define     TReloadRegH           0x2C
#define     TReloadRegL           0x2D
#define     TCounterValueRegH     0x2E
#define     TCounterValueRegL     0x2F
//Page 3:TestRegister     
#define     Reserved30            0x30
#define     TestSel1Reg           0x31
#define     TestSel2Reg           0x32
#define     TestPinEnReg          0x33
#define     TestPinValueReg       0x34
#define     TestBusReg            0x35
#define     AutoTestReg           0x36
#define     VersionReg            0x37
#define     AnalogTestReg         0x38
#define     TestDAC1Reg           0x39  
#define     TestDAC2Reg           0x3A   
#define     TestADCReg            0x3B   
#define     Reserved31            0x3C   
#define     Reserved32            0x3D   
#define     Reserved33            0x3E   
#define     Reserved34			  0x3F
//---------------------------------------------------------



//prototype functions
void delay1s(void);
void setup(void);
void MFRC522_Init(void);
void Write_MFRC522(uchar addr, uchar val);
uchar Read_MFRC522(uchar addr);
void SetBitMask(uchar reg, uchar mask);
void ClearBitMask(uchar reg, uchar mask);
void AntennaOn(void);
void AntennaOff(void);
void MFRC522_Reset(void);
uchar MFRC522_Request(uchar reqMode, uchar *TagType);
uchar MFRC522_ToCard(uchar command, uchar *sendData, uchar sendLen, uchar *backData, uint *backLen);
uchar MFRC522_Anticoll(uchar *serNum);
void CalulateCRC(uchar *pIndata, uchar len, uchar *pOutData);
uchar MFRC522_SelectTag(uchar *serNum);
uchar MFRC522_Auth(uchar authMode, uchar BlockAddr, uchar *Sectorkey, uchar *serNum);
uchar MFRC522_Read(uchar blockAddr, uchar *recvData);
uchar MFRC522_Write(uchar blockAddr, uchar *writeData);
void MFRC522_Halt(void);
void showSerialNumber(void);
void sendToSerialASCII(int sector, int block, uchar status, uchar *str);
void sendToSerialHEX(int block, uchar status, uchar *str);
void readDataASCII(void);
void readDataHEX(void);
void clearTagsMemory(void);
void writeTagBlockMemory(void);
void witeDataToTagMemory(void);
void writeTagBlockData(int block, uchar *data);


//------------------------------------------------------------------------------

/* Description: Clears all contents in user's data blocks **********************
 * Input parameter: 
 * Return: null					 */
void clearTagsMemory(void){
	//4 bytes Serial number of card, the 5 bytes is verfiy bytes
	uchar serNum[5];
	//buffer A password, 16 buffer, the passowrd of every buffer is 6 byte 
	uchar sectorX_KeyA[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}; 
	uchar i,j;
	uchar status;
    uchar str[MAX_LEN];
    //Select operation buck address  0 - 63
	char msg1[]={"TAG's memory cleaning started"}; 	              
	uchar dataXX[]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	//uchar dataXF[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	setup();
	for(;;){
		//Search card, return card types
		status = MFRC522_Request(PICC_REQIDL, str);	
		if (status == MI_OK){  }
		//Prevent conflict, return the 4 bytes Serial number of the card
		status = MFRC522_Anticoll(str);
		for(i=0; i<5; i++){	 serNum[i]=str[i];  }	//memcpy(serNum, str, 5);
		if (status == MI_OK){  
			putsUSART(msg1);putcUSART('\r');Delay10KTCYx(10);    }
		status = MFRC522_SelectTag(serNum);	
		status = MFRC522_Auth(0x60,0,sectorX_KeyA,serNum);	//sector 0
		if(status==MI_OK){   for(i=1;i<3;i++){   writeTagBlockData(i,dataXX);   }}
		status = MFRC522_Auth(0x60,4,sectorX_KeyA,serNum);	//sector 1
		if(status==MI_OK){   for(i=4;i<7;i++){   writeTagBlockData(i,dataXX);   }}
		status = MFRC522_Auth(0x60,8,sectorX_KeyA,serNum);	//sector 2
		if(status==MI_OK){   for(i=8;i<11;i++){   writeTagBlockData(i,dataXX);   }}
		status = MFRC522_Auth(0x60,12,sectorX_KeyA,serNum);	//sector 3
		if(status==MI_OK){   for(i=12;i<15;i++){   writeTagBlockData(i,dataXX);   }}
		status = MFRC522_Auth(0x60,16,sectorX_KeyA,serNum);	//sector 4
		if(status==MI_OK){   for(i=16;i<19;i++){   writeTagBlockData(i,dataXX);   }}
		status = MFRC522_Auth(0x60,20,sectorX_KeyA,serNum);	//sector 5
		if(status==MI_OK){   for(i=20;i<23;i++){   writeTagBlockData(i,dataXX);   }}
		status = MFRC522_Auth(0x60,24,sectorX_KeyA,serNum);	//sector 6
		if(status==MI_OK){   for(i=24;i<27;i++){   writeTagBlockData(i,dataXX);   }}
		status = MFRC522_Auth(0x60,28,sectorX_KeyA,serNum);	//sector 7
		if(status==MI_OK){   for(i=28;i<31;i++){   writeTagBlockData(i,dataXX);   }}
		status = MFRC522_Auth(0x60,32,sectorX_KeyA,serNum);	//sector 8
		if(status==MI_OK){   for(i=32;i<35;i++){   writeTagBlockData(i,dataXX);   }}
		status = MFRC522_Auth(0x60,36,sectorX_KeyA,serNum);	//sector 9
		if(status==MI_OK){   for(i=36;i<39;i++){   writeTagBlockData(i,dataXX);   }}
		status = MFRC522_Auth(0x60,40,sectorX_KeyA,serNum);	//sector 10
		if(status==MI_OK){   for(i=40;i<43;i++){   writeTagBlockData(i,dataXX);   }}
		status = MFRC522_Auth(0x60,44,sectorX_KeyA,serNum);	//sector 11
		if(status==MI_OK){   for(i=44;i<47;i++){   writeTagBlockData(i,dataXX);   }}
		status = MFRC522_Auth(0x60,48,sectorX_KeyA,serNum);	//sector 12
		if(status==MI_OK){   for(i=48;i<51;i++){   writeTagBlockData(i,dataXX);   }}
		status = MFRC522_Auth(0x60,52,sectorX_KeyA,serNum);	//sector 13
		if(status==MI_OK){   for(i=52;i<55;i++){   writeTagBlockData(i,dataXX);   }}
		status = MFRC522_Auth(0x60,56,sectorX_KeyA,serNum);	//sector 14
		if(status==MI_OK){   for(i=56;i<59;i++){   writeTagBlockData(i,dataXX);   }}
		status = MFRC522_Auth(0x60,60,sectorX_KeyA,serNum);	//sector 15
		if(status==MI_OK){   for(i=60;i<63;i++){   writeTagBlockData(i,dataXX);   }}		
		MFRC522_Halt();
		delay1s();							}}

/* Description: write  TAG's memory bytes *****************************************
 * Input parameter: null
 * Return: null					 */
void writeTagBlockMemory(void){
	//4 bytes Serial number of card, the 5 bytes is verify bytes
	uchar serNum[5];
	//buffer A password, 16 buffer, the password of every buffer is 6 bytes 
	uchar sectorX_KeyA[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}; 
	uchar i,j;
	uchar status;
    uchar str[MAX_LEN];
    //Select operation buck address  0 - 63
	//char msg1[]={"Writing TAG's memory block"};
  //SECTOR 00     //0123456789ABCDEF
  //uchar data00[]="			    ";  //Do not write serial number or manufacturer data
  //uchar data01[]="William Martinez";
  //uchar data02[]="ID: 5602-8788-ME";
  //uchar data03[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x07,0x80,0x69,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	//Warning: only write sector trailer when you know what you're doing
  //SECTOR 01
  uchar data04[]="License permit:B";
  uchar data05[]="Penny Lane 63-C ";
  uchar data06[]="London 59032    ";
  //uchar data07[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x07,0x80,0x69,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	//Warning: only write sector trailer when you know what you're doing
  //SECTOR 02
//	uchar data08[]="Insurance data  ";
//	uchar data09[]="2014-BA         ";
//	uchar data10[]="Health service  ";
    //uchar data11[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x07,0x80,0x69,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	//Warning: only write sector trailer when you know what you're doing
  //SECTOR 03
//	uchar data12[]="Hola a todos hoy";
//	uchar data13[]="hace un dia con ";
//	uchar data14[]="  buen clima!   ";
  //uchar data15[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x07,0x80,0x69,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	//Warning: only write sector trailer when you know what you're doing
  //SECTOR 04
//	uchar data16[]="     _ /\ _     ";
//	uchar data17[]="     \ oo /     ";
//	uchar data18[]="     /    \     ";
  //uchar data19[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,x07,0x80,0x69,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	//Warning: only write sector trailer when you know what you're doing
  //SECTOR 05
//	uchar data20[]="                ";
//	uchar data21[]="                ";
//	uchar data22[]="                ";
  //uchar data23[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,x07,0x80,0x69,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	//Warning: only write sector trailer when you know what you're doing
  //SECTOR 06
//	uchar data24[]="                ";
//	uchar data25[]="                ";
//	uchar data26[]="                ";
  //uchar data27[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,x07,0x80,0x69,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	//Warning: only write sector trailer when you know what you're doing
  //SECTOR 07
//	uchar data28[]="                ";
//	uchar data29[]="                ";
//	uchar data30[]="                ";
  //uchar data31[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFFx07,0x80,0x69,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	//Warning: only write sector trailer when you know what you're doing
  //SECTOR 08
//	uchar data32[]="                ";
//	uchar data33[]="                ";
//	uchar data34[]="                ";
  //uchar data35[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,x07,0x80,0x69,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	//Warning: only write sector trailer when you know what you're doing
  //SECTOR 09
//	uchar data36[]="                ";
//	uchar data37[]="                ";
//	uchar data38[]="                ";
  //uchar data39[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,x07,0x80,0x69,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	//Warning: only write sector trailer when you know what you're doing
  //SECTOR 10
//	uchar data40[]="                ";
//	uchar data41[]="                ";
//	uchar data42[]="                ";
  //uchar data43[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,x07,0x80,0x69,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	//Warning: only write sector trailer when you know what you're doing
  //SECTOR 11
//	uchar data44[]="                ";
//	uchar data45[]="                ";
//	uchar data46[]="                ";
  //uchar data47[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,x07,0x80,0x69,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	//Warning: only write sector trailer when you know what you're doing
  //SECTOR 12
//	uchar data48[]="                ";
//	uchar data49[]="                ";
//	uchar data50[]="                ";
  //uchar data51[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,x07,0x80,0x69,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	//Warning: only write sector trailer when you know what you're doing
  //SECTOR 13
//	uchar data52[]="                ";
//	uchar data53[]="                ";
//	uchar data54[]="                ";
  //uchar data55[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,x07,0x80,0x69,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	//Warning: only write sector trailer when you know what you're doing
  //SECTOR 14
//	uchar data56[]="                ";
//	uchar data57[]="                ";
//	uchar data58[]="                ";
  //uchar data59[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,x07,0x80,0x69,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	//Warning: only write sector trailer when you know what you're doing
  //SECTOR 15
//	uchar data60[]="                ";
//	uchar data61[]="                ";
//	uchar data62[]="                ";
  //uchar data63[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,x07,0x80,0x69,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};	//Warning: only write sector trailer when you know what you're doing
	setup();
	for(;;){
		//Search card, return card types
		status = MFRC522_Request(PICC_REQIDL, str);	
		if (status == MI_OK){  }
		//Prevent conflict, return the 4 bytes Serial number of the card
		status = MFRC522_Anticoll(str);
		for(i=0; i<5; i++){	 serNum[i]=str[i];  }	//memcpy(serNum, str, 5);
		if (status == MI_OK){  }
			//putsUSART(msg1);putcUSART('\r');Delay10KTCYx(10);    }
		status = MFRC522_SelectTag(serNum);	
	  //status = MFRC522_Auth(0x60,1,sectorX_KeyA,serNum);
//		status = MFRC522_Auth(0x60,0,sectorX_KeyA,serNum);	//sector 0
//		if(status==MI_OK){
//			writeTagBlockData(1,data01);
//			writeTagBlockData(2,data02);	}
		status = MFRC522_Auth(0x60,4,sectorX_KeyA,serNum);	//sector 1
		if(status==MI_OK){   
			writeTagBlockData(4,data04);
			writeTagBlockData(5,data05);
			writeTagBlockData(6,data06);	}
//		status = MFRC522_Auth(0x60,8,sectorX_KeyA,serNum);	//sector 2
//		if(status==MI_OK){   
//			writeTagBlockData(8,data08);
//			writeTagBlockData(9,data09);
//			writeTagBlockData(10,data10);	}
//		status = MFRC522_Auth(0x60,12,sectorX_KeyA,serNum);	//sector 3
//		if(status==MI_OK){   
//			writeTagBlockData(12,data12);
//			writeTagBlockData(13,data13);
//			writeTagBlockData(14,data14);	}
//		status = MFRC522_Auth(0x60,16,sectorX_KeyA,serNum);	//sector 4
//		if(status==MI_OK){   
//			writeTagBlockData(16,data16);
//			writeTagBlockData(17,data17);
//			writeTagBlockData(18,data18);	}
//		status = MFRC522_Auth(0x60,20,sectorX_KeyA,serNum);	//sector 5
//		if(status==MI_OK){   
//			writeTagBlockData(20,data20);
//			writeTagBlockData(21,data21);
//			writeTagBlockData(22,data22);	}
//		status = MFRC522_Auth(0x60,24,sectorX_KeyA,serNum);	//sector 6
//		if(status==MI_OK){   
//			writeTagBlockData(24,data24);
//			writeTagBlockData(25,data25);
//			writeTagBlockData(26,data26);	}
//		status = MFRC522_Auth(0x60,28,sectorX_KeyA,serNum);	//sector 7
//		if(status==MI_OK){   
//			writeTagBlockData(28,data28);
//			writeTagBlockData(29,data29);
//			writeTagBlockData(30,data30);	}
//		status = MFRC522_Auth(0x60,32,sectorX_KeyA,serNum);	//sector 8
//		if(status==MI_OK){   
//			writeTagBlockData(32,data32);
//			writeTagBlockData(33,data33);
//			writeTagBlockData(34,data34);	}							
//		status = MFRC522_Auth(0x60,36,sectorX_KeyA,serNum);	//sector 9		
//		if(status==MI_OK){   
//			writeTagBlockData(36,data36);							
//			writeTagBlockData(37,data37);
//			writeTagBlockData(38,data38);	}
//		status = MFRC522_Auth(0x60,40,sectorX_KeyA,serNum);	//sector 10
//		if(status==MI_OK){   
//			writeTagBlockData(41,data41);
//			writeTagBlockData(42,data42);
//			writeTagBlockData(43,data43);	}
//		status = MFRC522_Auth(0x60,44,sectorX_KeyA,serNum);	//sector 11
//		if(status==MI_OK){   
//			writeTagBlockData(44,data44);								
//			writeTagBlockData(45,data45);									
//			writeTagBlockData(46,data46);	}							
//		status = MFRC522_Auth(0x60,48,sectorX_KeyA,serNum);	//sector 12
//		if(status==MI_OK){   
//			writeTagBlockData(48,data48);
//			writeTagBlockData(49,data49);
//			writeTagBlockData(50,data50);	}
//		status = MFRC522_Auth(0x60,52,sectorX_KeyA,serNum);	//sector 13
//		if(status==MI_OK){   
//			writeTagBlockData(52,data52);
//			writeTagBlockData(53,data53);
//			writeTagBlockData(54,data54);	}
//		status = MFRC522_Auth(0x60,56,sectorX_KeyA,serNum);	//sector 14
//		if(status==MI_OK){   
//			writeTagBlockData(56,data56);
//			writeTagBlockData(57,data57);
//			writeTagBlockData(58,data58);	}
//		status = MFRC522_Auth(0x60,60,sectorX_KeyA,serNum);	//sector 15
//		if(status==MI_OK){   
//			writeTagBlockData(60,data60);
//			writeTagBlockData(61,data61);
//			writeTagBlockData(62,data62);	}							
		MFRC522_Halt();
		delay1s();							}}

/* Description: Write data to TAG's memory ************************************
 * Input parameter: block to be written, dataArray
 * Return: null					 */
void writeTagBlockData(int block, uchar *data) {
	uchar status;
	char string[31];
	sprintf(string, (const far rom char*)"Sector %02d successfully written", block);
	status = MFRC522_Write(block, data);
	if(status == MI_OK){putcUSART('\r');putsUSART(string);	}}	

/* Description: Send data read to serial monitor HEX format ********************
 * Input parameter: null
 * Return: null					 */
void readDataHEX(void){
	//4 bytes Serial number of card, the 5 bytes is verfiy bytes
	uchar serNum[5];
	//buffer A password, 16 buffer, the passowrd of every buffer is 6 byte 
	uchar sectorX_KeyA[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}; 
	uchar i,j;
	uchar status;
    uchar str[MAX_LEN];
    //Select operation buck address  0 - 63
	char msg1[]={"\nTAG's data in HEX format: "};
	setup();
	for(;;){
		//Search card, return card types
		status = MFRC522_Request(PICC_REQIDL, str);	
		if (status == MI_OK){  }//putsUSART(msg0);putcUSART('\r');Delay10KTCYx(10);   }
		//Prevent conflict, return the 4 bytes Serial number of the card
		status = MFRC522_Anticoll(str);
		for(i=0; i<5; i++){	 serNum[i]=str[i];  }	//memcpy(serNum, str, 5);
		if (status == MI_OK){  
			putsUSART(msg1);putcUSART('\r');Delay10KTCYx(10);   	}
		status = MFRC522_SelectTag(serNum);	
		status = MFRC522_Auth(0x60,0,sectorX_KeyA,serNum);	//sector 0
		for(j=0;j<4;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialHEX(j,status, str);	}
		status = MFRC522_Auth(0x60,4,sectorX_KeyA,serNum);	//sector 1
		for(j=4;j<8;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialHEX(j,status, str);	}
		status = MFRC522_Auth(0x60,8,sectorX_KeyA,serNum);	//sector 2
		for(j=8;j<12;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialHEX(j,status, str);	}
		status = MFRC522_Auth(0x60,12,sectorX_KeyA,serNum);	//sector 3
		for(j=12;j<16;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialHEX(j,status, str);	}
		status = MFRC522_Auth(0x60,16,sectorX_KeyA,serNum);	//sector 4
		for(j=16;j<20;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialHEX(j,status, str);	}
		status = MFRC522_Auth(0x60,20,sectorX_KeyA,serNum);	//sector 5
		for(j=20;j<24;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialHEX(j,status, str);	}
		status = MFRC522_Auth(0x60,24,sectorX_KeyA,serNum);	//sector 6
		for(j=24;j<28;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialHEX(j,status, str);	}
		status = MFRC522_Auth(0x60,28,sectorX_KeyA,serNum);	//sector 7
		for(j=28;j<32;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialHEX(j,status, str);	}
		status = MFRC522_Auth(0x60,32,sectorX_KeyA,serNum);	//sector 8
		for(j=32;j<36;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialHEX(j,status, str);	}
		status = MFRC522_Auth(0x60,36,sectorX_KeyA,serNum);	//sector 9
		for(j=36;j<40;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialHEX(j,status, str);	}
		status = MFRC522_Auth(0x60,40,sectorX_KeyA,serNum);	//sector 10
		for(j=40;j<44;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialHEX(j,status, str);	}
		status = MFRC522_Auth(0x60,44,sectorX_KeyA,serNum);	//sector 11
		for(j=44;j<48;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialHEX(j,status, str);	}
		status = MFRC522_Auth(0x60,48,sectorX_KeyA,serNum);	//sector 12
		for(j=48;j<52;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialHEX(j,status, str);	}
		status = MFRC522_Auth(0x60,52,sectorX_KeyA,serNum);	//sector 13
		for(j=52;j<56;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialHEX(j,status, str);	}
		status = MFRC522_Auth(0x60,56,sectorX_KeyA,serNum);	//sector 14
		for(j=56;j<60;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialHEX(j,status, str);	}
		status = MFRC522_Auth(0x60,60,sectorX_KeyA,serNum);	//sector 15
		for(j=60;j<64;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialHEX(j,status, str);	}
		MFRC522_Halt();
		delay1s();							}}

/* Description: Send data read to serial monitor ASCII format *******************
 * Input parameter: null
 * Return: null					 */
void readDataASCII(void){
	//4 bytes Serial number of card, the 5 bytes is verfiy bytes
	uchar serNum[5];
	//buffer A password, 16 buffer, the passowrd of every buffer is 6 byte 
	uchar sectorX_KeyA[]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}; 
	uchar i,j;
	uchar status;
    uchar str[MAX_LEN];
	char msg2[]={"\n TAG's data in ASCII format:"};
	setup();
	for(;;){
		//Search card, return card types
		status = MFRC522_Request(PICC_REQIDL, str);	
		if (status == MI_OK){  }//putsUSART(msg0);putcUSART('\r');Delay10KTCYx(10);   }
		//Prevent conflict, return the 4 bytes Serial number of the card
		status = MFRC522_Anticoll(str);
		for(i=0; i<5; i++){	 serNum[i]=str[i];  }	//memcpy(serNum, str, 5);
		if (status == MI_OK){  
			//putsUSART(msg1);putcUSART('\r');Delay10KTCYx(10);
			putsUSART(msg2);putcUSART('\r');Delay10KTCYx(10);   }
		status = MFRC522_SelectTag(serNum);	
		status = MFRC522_Auth(0x60,0,sectorX_KeyA,serNum);	//sector 0
		for(j=0;j<4;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialASCII(0,j,status, str);	}
		status = MFRC522_Auth(0x60,4,sectorX_KeyA,serNum);	//sector 1
		for(j=4;j<8;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialASCII(1,j,status, str);	}
		status = MFRC522_Auth(0x60,8,sectorX_KeyA,serNum);	//sector 2
		for(j=8;j<12;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialASCII(2,j,status, str);	}
		status = MFRC522_Auth(0x60,12,sectorX_KeyA,serNum);	//sector 3
		for(j=12;j<16;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialASCII(3,j,status, str);	}
		status = MFRC522_Auth(0x60,16,sectorX_KeyA,serNum);	//sector 4
		for(j=16;j<20;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialASCII(4,j,status, str);	}
		status = MFRC522_Auth(0x60,20,sectorX_KeyA,serNum);	//sector 5
		for(j=20;j<24;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialASCII(5,j,status, str);	}
		status = MFRC522_Auth(0x60,24,sectorX_KeyA,serNum);	//sector 6
		for(j=24;j<28;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialASCII(6,j,status, str);	}
		status = MFRC522_Auth(0x60,28,sectorX_KeyA,serNum);	//sector 7
		for(j=28;j<32;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialASCII(7,j,status, str);	}
		status = MFRC522_Auth(0x60,32,sectorX_KeyA,serNum);	//sector 8
		for(j=32;j<36;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialASCII(8,j,status, str);	}
		status = MFRC522_Auth(0x60,36,sectorX_KeyA,serNum);	//sector 9
		for(j=36;j<40;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialASCII(9,j,status, str);	}
		status = MFRC522_Auth(0x60,40,sectorX_KeyA,serNum);	//sector 10
		for(j=40;j<44;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialASCII(10,j,status, str);	}
		status = MFRC522_Auth(0x60,44,sectorX_KeyA,serNum);	//sector 11
		for(j=44;j<48;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialASCII(11,j,status, str);	}
		status = MFRC522_Auth(0x60,48,sectorX_KeyA,serNum);	//sector 12
		for(j=48;j<52;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialASCII(12,j,status, str);	}
		status = MFRC522_Auth(0x60,52,sectorX_KeyA,serNum);	//sector 13
		for(j=52;j<56;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialASCII(13,j,status, str);	}
		status = MFRC522_Auth(0x60,56,sectorX_KeyA,serNum);	//sector 14
		for(j=56;j<60;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialASCII(14,j,status, str);	}
		status = MFRC522_Auth(0x60,60,sectorX_KeyA,serNum);	//sector 15
		for(j=60;j<64;j++)					{
			status = MFRC522_Read(j, str);
			sendToSerialASCII(15,j,status, str);	}
		MFRC522_Halt();
		delay1s();	} }

/* Description: Send data read to serial monitor ASCII format ******************
 * Input parameter: sector, block, status and pointer to string read
 * Return: null					 */
void sendToSerialASCII(int sector, int block, uchar status, uchar *str){
	int i;
	char string[22];
	sprintf(string, (const far rom char*)"Sector %2d, block %2d: ", sector, block);
	if(status == MI_OK){
		putsUSART(string);
		for(i=0;i<16;i++)		{
			putcUSART(str[i]);
			Delay10KTCYx(5);	}
		putcUSART('\r');	   }}

/* Description: Send data read to serial monitor HEX format ********************
 * Input parameter: sector, block, status and pointer to string read
 * Return: null					 */
void sendToSerialHEX(int block, uchar status, uchar *str){
	int i;
	char string0[5];
	char string1[49];
	sprintf(string0, (const far rom char*)"%2d: ",block);
	sprintf(string1, (const far rom char*)"%2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x %2x", 
		str[0], str[1], str[2], str[3], str[4], str[5], str[6], str[7], str[8], str[9], str[10], str[11], str[12], str[13], str[14], str[15]);
	if(status == MI_OK){
		putsUSART(string0);
		putsUSART(string1);
		//putsUSART(string2);
		putcUSART('\r');	}}

/* Description: Shows TAG's serial number **************************************
 * Input parameter: null
 * Return: null					 */
void showSerialNumber(void){
	//4 bytes Serial number of card, the 5 bytes is verfiy bytes
	uchar serNum[5]; 
	uchar i;
	uchar status;
    uchar str[MAX_LEN];
	char string[22];
	char msg0[]={"Card detected\r"};
	char msg1[]={" , "};
	char msg2[]={"   "};
	char msg3[]={"The card's number is: \r"};	
	char msg4[]={"Hello master!"};	
	setup();
	for(;;){
		//Search card, return card types
		status = MFRC522_Request(PICC_REQIDL, str);	
		if (status == MI_OK){
			putcUSART('\r');
	    	//putsUSART(msg0);	//Serial.println("Card detected");
			//putcUSART(str[0]);	//Serial.print(str[0],BIN);
	        //putsUSART(msg1);	//Serial.print(" , ");
			//putcUSART(str[1]);	//Serial.print(str[1],BIN);
	        //putsUSART(msg2);	//Serial.println(" ");
		}
		//Prevent conflict, return the 4 bytes Serial number of the card
		status = MFRC522_Anticoll(str);
		for(i=0; i<5; i++){	 serNum[i]=str[i];  }	//memcpy(serNum, str, 5);
		if (status == MI_OK)	{
	    	putsUSART(msg3);		//Serial.println("The card's number is  : ");
			sprintf(string, (const far rom char*)"%2x %2x %2x %2x", str[0], str[1], str[2], str[3]);
			putsUSART(msg0);
			putcUSART('\r');
			putsUSART(string);  
			putcUSART('\r');
			//putcUSART('\n');  	
	        delay1s();			}	//delay(1000);
		MFRC522_Halt();	}}

/* Description: 1 s delay  *****************************************************
 * Input parameter: null
 * Return: null					 */
void delay1s(void){
	int i;
	for(i=0; i<5; i++){Delay10KTCYx(240);}}	// 1 s delay

/* Description: initilize RS232, SPI, pin **************************************
 * Input parameter: null
 * Return: null					 */
void setup(void) {
	OpenUSART(USART_TX_INT_OFF 	& 
		  USART_RX_INT_OFF 		& 
		  USART_ASYNCH_MODE 	& 
		  USART_EIGHT_BIT 		& 
		  USART_CONT_RX 		& 
		  USART_BRGH_LOW, 
		  77					);	//RFID reader SOUT pin connected to Serial RX pin
	OpenSWSPI();					//start the SPI library
	ClearCSSWSPI();					//Activate the RFID reader
	TRISBbits.TRISB4=0;				//Set digital pin, Not Reset and Power-Down
	RST=1;							//digitalWrite(NRSTPD,HIGH);							
	MFRC522_Init();				}

/* Description: initilize RC522 ************************************************
 * Input parameter: null
 * Return: null					 */
void MFRC522_Init(void) {
	RST=1;							//digitalWrite(NRSTPD,HIGH);
	MFRC522_Reset(); 	
	//Timer: TPrescaler*TreloadVal/6.78MHz = 24ms
    Write_MFRC522(TModeReg, 	0x8D);			//Tauto=1; f(Timer) = 6.78MHz/TPreScaler
    Write_MFRC522(TPrescalerReg,0x3E);			//TModeReg[3..0] + TPrescalerReg
    Write_MFRC522(TReloadRegL, 	30	);           
    Write_MFRC522(TReloadRegH, 	0	);	
	Write_MFRC522(TxAutoReg, 	0x40);			//100%ASK
	Write_MFRC522(ModeReg, 		0x3D);			//CRC initilizate value 0x6363	
	//ClearBitMask(Status2Reg, 	0x08);			//MFCrypto1On=0
	//Write_MFRC522(RxSelReg, 	0x86);			//RxWait = RxSelReg[5..0]
	//Write_MFRC522(RFCfgReg, 	0x7F);   		//RxGain = 48dB
	AntennaOn();				}				//turn on antenna

/* Description: write a byte data into one register of MFRC522 *****************
 * Input parameter: addr--register address; val--the value that need to write in
 * Return: Null						*/
void Write_MFRC522(uchar addr, uchar val) {
	ClearCSSWSPI();					//digitalWrite(chipSelectPin, LOW);	
	WriteSWSPI((addr<<1)&0x7E);		//address format: 0XXXXXX0
	WriteSWSPI(val);	
	SetCSSWSPI();				}	//digitalWrite(chipSelectPin, HIGH);	

/* Description: read a byte data into one register of MFRC522 ******************
 * Input parameter: addr--register address
 * Return: return the read value		*/
uchar Read_MFRC522(uchar addr) {
	uchar val;
	ClearCSSWSPI();							//digitalWrite(chipSelectPin, LOW);
	WriteSWSPI(((addr<<1)&0x7E) | 0x80);	//address format: 1XXXXXX0
	val =WriteSWSPI(0x00);	
	SetCSSWSPI();							//digitalWrite(chipSelectPin, HIGH);	
	return val;					}

/* Description: set RC522 register bit *****************************************
 * Input parameter:reg--register address; mask--value
 * Return: null						*/
void SetBitMask(uchar reg, uchar mask)  {
    uchar tmp;
    tmp = Read_MFRC522(reg);
    Write_MFRC522(reg, tmp | mask);  	}	// set bit mask

/* Description: clear RC522 register bit ***************************************
 * Input parameter: reg--register address; mask--value
 * Return: null 						*/
void ClearBitMask(uchar reg, uchar mask) {
    uchar tmp;
    tmp = Read_MFRC522(reg);
    Write_MFRC522(reg, tmp & (~mask));	 }  // clear bit mask

/* Description: Turn on antenna, every time turn on or shut down antenna need at least 1ms delay
 * Input parameter: null
 * Return: null						*/
void AntennaOn(void) {
	uchar temp;
	temp = Read_MFRC522(TxControlReg);
	if (!(temp & 0x03)){  SetBitMask(TxControlReg, 0x03);  } }

/* Description: Turn off antenna, every time turn on or shut down antenna need at least 1ms delay
 * Input parameter: null
 * Return: null						*/
void AntennaOff(void){	ClearBitMask(TxControlReg, 0x03);  }

/* Description: reset RC522 ****************************************************
 * Input parameter:null
 * Return:null					*/
void MFRC522_Reset(void){  Write_MFRC522(CommandReg, PCD_RESETPHASE);  }

/* Description: Searching card, read card type *********************************
 * Input parameter: reqMode -- search methods,
 *			 TagType--return card types
 *			 	0x4400 = Mifare_UltraLight
 *				0x0400 = Mifare_One(S50)
 *				0x0200 = Mifare_One(S70)
 *				0x0800 = Mifare_Pro(X)
 *				0x4403 = Mifare_DESFire
 * return:return MI_OK if successed		*/
uchar MFRC522_Request(uchar reqMode, uchar *TagType) {
	uchar status;  
	uint backBits;							//the data bits that received
	Write_MFRC522(BitFramingReg, 0x07);		//TxLastBists = BitFramingReg[2..0]
	TagType[0] = reqMode;
	status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);
	if ((status != MI_OK) || (backBits != 0x10)){  status = MI_ERR;  }
	return status;				}

/* Description: communicate between RC522 and ISO14443 *************************
 * Input parameter: command--MF522 command bits
 *			 sendData--send data to card via rc522
 *			 sendLen--send data length		 
 *			 backData--the return data from card
 *			 backLen--the length of return data
 * return: return MI_OK if successed				*/
uchar MFRC522_ToCard(uchar command, uchar *sendData, uchar sendLen, uchar *backData, uint *backLen){
    uchar status = MI_ERR;
    uchar irqEn = 	0x00;
    uchar waitIRq = 0x00;
    uchar lastBits;
    uchar n;
    uint i;
    switch (command) {
        case PCD_AUTHENT: 	{	//verify card password		
			irqEn = 0x12;
			waitIRq = 0x10;
			break;			}
		case PCD_TRANSCEIVE:{	//send data in the FIFO
			irqEn = 0x77;
			waitIRq = 0x30;
			break;			}
		default:	break; 	}
    Write_MFRC522(CommIEnReg, irqEn|0x80);	//Allow interruption
    ClearBitMask(CommIrqReg, 0x80);			//Clear all the interrupt bits
    SetBitMask(FIFOLevelReg, 0x80);			//FlushBuffer=1, FIFO initilizate
	Write_MFRC522(CommandReg, PCD_IDLE);	//NO action;cancel current command	
	//write data into FIFO
    for (i=0; i<sendLen; i++){ 	 Write_MFRC522(FIFODataReg, sendData[i]);   }
	//procceed it
	Write_MFRC522(CommandReg, command);
    if (command == PCD_TRANSCEIVE){   SetBitMask(BitFramingReg, 0x80);	} //StartSend=1,transmission of data starts  
	//waite receive data is finished
	i = 2000;	//i should adjust according the clock, the maxium the waiting time should be 25 ms
    do {
		//CommIrqReg[7..0]
		//Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
        n = Read_MFRC522(CommIrqReg);
        i--;
    }while ((i!=0) && !(n&0x01) && !(n&waitIRq));
    ClearBitMask(BitFramingReg, 0x80);			//StartSend=0	
    if (i != 0) {    
        if(!(Read_MFRC522(ErrorReg) & 0x1B))	//BufferOvfl Collerr CRCErr ProtecolErr
        {
            status = MI_OK;
            if (n & irqEn & 0x01){ 	status = MI_NOTAGERR;	}
            if (command == PCD_TRANSCEIVE){
               	n = Read_MFRC522(FIFOLevelReg);
              	lastBits = Read_MFRC522(ControlReg) & 0x07;
                if (lastBits){ 	*backLen = (n-1)*8 + lastBits;   }
                else{ 	*backLen = n*8;   }
                if (n == 0){  	n = 1;   }
                if (n > MAX_LEN){   n = MAX_LEN;   	}	
				//read the data from FIFO
                for (i=0; i<n; i++){   	backData[i] = Read_MFRC522(FIFODataReg); 	}
            }
        }
        else{	status = MI_ERR;  	}     
    }	
    //SetBitMask(ControlReg,0x80);           	//timer stops
    //Write_MFRC522(CommandReg, PCD_IDLE); 
    return status;					}

/* Description: Prevent conflict, read the card serial number ******************
 * Input parameter: serNum--return the 4 bytes card serial number, the 5th byte is recheck byte
 * return: return MI_OK if successed			*/
uchar MFRC522_Anticoll(uchar *serNum){
    uchar status;
    uchar i;
	uchar serNumCheck=0;
    uint unLen;
    //ClearBitMask(Status2Reg, 0x08);		//TempSensclear
    //ClearBitMask(CollReg,0x80);			//ValuesAfterColl
	Write_MFRC522(BitFramingReg, 0x00);		//TxLastBists = BitFramingReg[2..0]
    serNum[0] = PICC_ANTICOLL;
    serNum[1] = 0x20;
    status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);
    if (status == MI_OK){
		//Verify card serial number
		for (i=0; i<4; i++){	serNumCheck ^= serNum[i];	}
		if (serNumCheck != serNum[i]){ 	status = MI_ERR;    }
    }
    //SetBitMask(CollReg, 0x80);		//ValuesAfterColl=1
    return status;					} 				

/* Description: Use MF522 to caculate CRC **************************************
 * Input parameter: pIndata--the CRC data need to be read,len--data length,pOutData-- the caculated result of CRC
 * return: Null
 */
void CalulateCRC(uchar *pIndata, uchar len, uchar *pOutData){
    uchar i, n;
    ClearBitMask(DivIrqReg, 0x04);			//CRCIrq = 0
    SetBitMask(FIFOLevelReg, 0x80);			//Clear FIFO pointer
    //Write_MFRC522(CommandReg, PCD_IDLE);
	//Write data into FIFO	
    for (i=0; i<len; i++){ 	Write_MFRC522(FIFODataReg, *(pIndata+i));   }
    Write_MFRC522(CommandReg, PCD_CALCCRC);
	//waite CRC caculation to finish
    i = 0xFF;
    do {
        n = Read_MFRC522(DivIrqReg);
        i--;
    }while ((i!=0) && !(n&0x04));			//CRCIrq = 1
	//read CRC caculation result
    pOutData[0] = Read_MFRC522(CRCResultRegL);
    pOutData[1] = Read_MFRC522(CRCResultRegM);				}

/* Description: Select card, read card storage volume *************************
 * Input parameter :serNum--Send card serial number
 * return: return the card storage volume			 */
uchar MFRC522_SelectTag(uchar *serNum) {
    uchar i;
	uchar status;
	uchar size;
    uint recvBits;
    uchar buffer[9]; 
	//ClearBitMask(Status2Reg, 0x08);			//MFCrypto1On=0
    buffer[0] = PICC_SElECTTAG;
    buffer[1] = 0x70;
    for (i=0; i<5; i++){  buffer[i+2] = *(serNum+i);  }
	CalulateCRC(buffer, 7, &buffer[7]);		
    status = MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);
    if ((status == MI_OK) && (recvBits == 0x18)){ 	size = buffer[0]; 	}
    else{ 	size = 0;  	}
    return size;						}

/* Description:verify card password ********************************************
 * Input parameters:authMode--password verify mode
                 0x60 = verify A passowrd key 
                 0x61 = verify B passowrd key 
             BlockAddr--Block address
             Sectorkey--Block password
             serNum--Card serial number ,4 bytes
 * return:return MI_OK if successed				*/
uchar MFRC522_Auth(uchar authMode, uchar BlockAddr, uchar *Sectorkey, uchar *serNum) {
    uchar status;
    uint recvBits;
    uchar i;
	uchar buff[12]; 
	//Verify command + block address + buffer password + card SN
    buff[0] = authMode;
    buff[1] = BlockAddr;
    for (i=0; i<6; i++){	buff[i+2] = *(Sectorkey+i);   }
    for (i=0; i<4; i++){  	buff[i+8] = *(serNum+i);   	  }
    status = MFRC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);
    if ((status != MI_OK) || (!(Read_MFRC522(Status2Reg) & 0x08))){	 status = MI_ERR;  }
    return status;																	   }

/* Description: Read data ******************************************************
 * Input parameters: blockAddr--block address; recvData--the block data which are read
 * return: return MI_OK if successed						*/
uchar MFRC522_Read(uchar blockAddr, uchar *recvData) {
    uchar status;
    uint unLen;
    recvData[0] = PICC_READ;
    recvData[1] = blockAddr;
    CalulateCRC(recvData,2, &recvData[2]);
    status = MFRC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);
    if ((status != MI_OK) || (unLen != 0x90)) {  status = MI_ERR;  } 
    return status;									}

/* Description: write block data ***********************************************
 * Input parameters: blockAddr--block address; writeData--Write 16 bytes data into block
 * return: return MI_OK if successed						*/
uchar MFRC522_Write(uchar blockAddr, uchar *writeData) {
    uchar status;
    uint recvBits;
    uchar i;
	uchar buff[18];     
    buff[0] = PICC_WRITE;
    buff[1] = blockAddr;
    CalulateCRC(buff, 2, &buff[2]);
    status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);
    if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)){  status = MI_ERR;    }
    if (status == MI_OK){
        for (i=0; i<16; i++){   buff[i] = *(writeData+i);   }	//Write 16 bytes data into FIFO
        CalulateCRC(buff, 16, &buff[16]);
        status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);
		if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)){  status = MI_ERR; }
    }
    return status;										}

/* Description: Command the cards into sleep mode ******************************
 * Input parameters: null
 * return: null 						*/
void MFRC522_Halt(void){
	uchar status;
    uint unLen;
    uchar buff[4]; 
    buff[0] = PICC_HALT;
    buff[1] = 0;
    CalulateCRC(buff, 2, &buff[2]);
    status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff,&unLen);			}
