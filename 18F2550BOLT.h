//LIBRERIA UNIVERSAL PARA EL SISTEMA BOLT CRISTAL EXTERNO 20 MHZ
//VELOCIDAD EFECTIVA DE 48 MHZ

//DEFINICIONES DE PINES:

#define RELE PORTAbits.RA0
#define SW1  PORTAbits.RA4
#define SW2  PORTAbits.RA5
#define SW3  PORTCbits.RC0
#define SW4  PORTCbits.RC1
#define RA1  PORTAbits.RA1	
#define RA2  PORTAbits.RA2
#define RA3  PORTAbits.RA3	
#define RB0  PORTBbits.RB0
#define RB1  PORTBbits.RB1
#define RB2  PORTBbits.RB2
#define RB3  PORTBbits.RB3
#define RB4  PORTBbits.RB4
#define RB5  PORTBbits.RB5
#define RB6  PORTBbits.RB6
#define RB7  PORTBbits.RB7
#define delay_us	Delay10TCYx	

//FUNCIONES DISPONIBLES:

void writeeeprom(unsigned char addr,unsigned char byte);  //escribe un dato en la EEPROM
unsigned char readeeprom(unsigned char addr); //lee un dato de la EEPROM
void init_bolt(void); //inicializa puertos leds y microswitches en BOLT
int microsw(void);	//lee los microswitches, la función toma el valor de 0...15
void delay_ms(int i); //delay con parámetro en milisegundos (i<32000)
void EnableLCD(void);	//pulso de escritura
void EnableLCD2(void);	//pulso de escritura
void InitLCD(void);	//inicializa LCD
void ClearScreen(void); //borra LCD
void GotoXY(unsigned char r, unsigned char c);	//posiciona cursor en celda seleccionada
void PutLCD(char c);	//escribe caracter en LCD parámetro en ASCII
void PrintString(const char* s);	//escribe string hasta 8 caracteres. Parámetro es el string.
unsigned char GetBot(void);		//lee 1 caracter del teclado. La función regresa con el valor ASCII de la tecla.
void fin(void);
void SetBot(void);			//lee un string del teclado terminando con "F". Lo guarda en el arreglo "bot[]"
void row(void);
void WrtBot(void);			//escribe el string almacenado en el arreglo "bot[]" en el LCD. 

//función también disponible: delay_us(); parámetro <250.

void delay_ms(int i)
{
	long int j;
	for(j=0;j<i;j++)
	{
	Delay1KTCYx(12); 	//A 48 MHZ MHZ, 1 MS APROX.
	}
}

//INICIALIZA LOS PUERTOS A, B Y C
void init_bolt(void)
{
ADCON1=0x0F; 	//DESHABILITA CONVERTIDORES A/D
CMCON=7;
TRISB=0;		//PORTB SON SALIDAS
TRISA=0X30;		//RA4,RA5 SON ENTRADAS (MICROSWITCHES). RA0,RA1,RA2,RA3 SON SALIDAS
TRISC=0X0F;		//RC0,RC1 SON ENTRADAS (MICROSWITCHES)
INTCON2bits.RBPU=0;	//SELECCIONA RESISTENCIAS DE PULL UP EN PUERTO B (RB4...RB7).
PORTB=0;
RELE=0;				//APAGA LEDS Y RELEVADOR
}

/*MICROSWITCHES*/
//LEE LOS MICROSWITCHES. REGRESA CON UN VALOR DE 0...15.
int microsw(void)
{
int i;
i=(SW1*1)+(SW2*2)+(SW3*4)+(SW4*8);
return i;
}


/*LCD*/
void EnableLCD(void) {

    RA1=0; delay_us(50);  //RS= 0
    RA2=1; delay_us(50);  //E = 1
    RA2=0; delay_us(50);  //E = 0
}

void EnableLCD2(void) {

    RA1=1;  delay_us(50);	//RS=1
    RA2=1;  delay_us(50);	//E =1
    RA2=0;  delay_us(50);	//E =0

}

void InitLCD(void) {

    PORTB=0x38; EnableLCD(); delay_ms(50);
    PORTB=0x01; EnableLCD(); delay_ms(50);
    PORTB=0x0d; EnableLCD(); delay_ms(50);
    PORTB=0x06; EnableLCD(); delay_ms(50);
}

void ClearScreen(void) {

    PORTB=0x01; 
	EnableLCD();               
	delay_ms(10);
}

void GotoXY(unsigned char r, unsigned char c) {

    PORTB=0x02; EnableLCD();               
	delay_ms(10);
    for(r=r*40+c, c=0; c<r; c++) {
        PORTB= 0x14;
	    EnableLCD(); 
		delay_us(50);
    }
}

void PutLCD(char c) {

    PORTB = c; EnableLCD2(); delay_us(50);
}

void PrintString(const char* s) {

    while( *s ) PutLCD(*(s++));
}

/*TECLADO*/
char key[16] = {0xEE,0xED,0XEB,0XE7,0XDE,0XDD,0XDB,0XD7,0XBE,0XBD,0XBB,0XB7,0X7E,0X7D,0X7B,0X77};
char asc[16] = {0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x41,0x42,0x43,0x44,0x45,0x46};
unsigned char aa,bb,cc,dd,ee,ff,xx,yy,zz,boton;
char bot[9];

unsigned char GetBot(void)			//la variable de salida se define como unsigned char
{
loop:
	ee=0;
	TRISA=0x30;				/*PUERTO A:RA0,RA1,RA2 SALIDAS; RA4,RA5 ENTRADAS*/
	TRISB=0XF0;
//	RBPU=0;					//REGISTRO OPTION, BIT7. Ver función init_bolt()
							
//	PORTA=0;					/*PUERTO B RB0..RB3 SALIDAS, RB4..RB7*/
	aa=0xEF;
	PORTB=aa;
	bb=4;
	while (bb>0) 			/*4 corrimientos hacia la derecha*/
	{aa=aa/2;
	PORTB=aa;				/*ESCRIBE EN PORTB RB0..RB3*/
							/*LEE PUERTO  RB4..RB7*/
	delay_ms(5);		
	if (RB4==0||RB5==0||RB6==0||RB7==0)fin();
	if (ee==1)return (ff);		/*regresa con el valor de la tecla en ASCII*/
	bb=bb-1;}
	goto loop;

}
void fin(void){			/*tecla oprimida*/
	
	TRISB=0XF0;
	dd=PORTB;
	cc=(dd & 0XF0);		/*bits RB0..RB3 =0, conserva RB4..RB7*/
	aa=(aa & 0x0F);		/*bits RB4..RB7 =0, conserva RB0..RB3*/
	aa=(aa | cc);			/*la variable "a" contiene el código rengón columna de la tecla oprimida*/

malla:	
	dd=PORTB;			
	cc=(dd & 0XF0);			/*bits RB0..RB3 =0, conserva RB4..RB7*/
	if(cc!=0xF0)goto malla; 	/*espera que la tecla se libere*/
	TRISB=0;
	row();	
	ff=xx;			/*convierte a código ASCII*/
	ee=1;
}				
 	void row(void)
	{

	for(yy=0;yy<16;yy=yy+1)
	{if (aa==key[yy])xx=asc[yy];}	/*la variable "x" contiene el valor ASCII*/
	PORTB=xx;
	delay_ms(100);
}

void SetBot(void)
{
for(zz=0;zz<7;zz++)
{boton=GetBot();
bot[zz]=boton;
if(bot[zz]==0x46)goto qu;
else PutLCD(boton);}
qu:bot[zz]=0x00;
}

void WrtBot(void)
{
for(zz=0;zz<7;zz++)
{PutLCD(bot[zz]);
if(bot[zz+1]==0)goto ka;}
ka:zz=zz;				//expresión dummy
}

/*FUNCIONES PARA ESCRIBIR Y LEER EN LA EEPROM INTERNA*/

void writeeeprom(unsigned char addr,unsigned char byte)
{
unsigned char i=INTCONbits.GIEH;
INTCONbits.GIEH = 0; //disable interrupts
EECON1bits.EEPGD=0; //Write to EEPROM
EECON1bits.CFGS=0; //EEPROM not config bits
EECON1bits.WREN=1; //Allows write

EEADR=addr;
EEDATA=byte;
EECON2=0x55;
EECON2=0xaa;
EECON1bits.WR=1;
while(EECON1bits.WR); //Wait until written
//while(!PIR2bits.EEIF);
//PIR2bits.EEIF=0;
EECON1bits.WREN=0; //No more write
INTCONbits.GIEH = i; //restore interrupts
}

unsigned char readeeprom(unsigned char addr)
{
EECON1bits.CFGS=0; //EEPROM not config bits
EECON1bits.EEPGD=0;
EEADR=addr;
EECON1bits.RD=1;
return (unsigned int) EEDATA;
}
