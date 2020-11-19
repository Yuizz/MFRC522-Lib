//FUNCIONES PARA EL PUERTO SERIAL DEL SISTEMA BOLT 18F2550
//9600 BPS PARA BOLT A 20 MHZ.VELOCIDAD EFECTIVA DE 48 MHZ.
//SE REQUIERE #include <usart.h> EN EL MAIN.

//FUNCIONES DISPONIBLES:
//DE LAS LIBRERIAS ESTANDAR C18 USE ():

// putcUSART(65); ESCRIBE LA LETRA "A" EN EL PUERTO SERIAL.

//DE ESTE ARCHIVO:

void activa_serial(void); //INICIALIZA EL PUERTO SERIAL A 9600 BPS.
char getcECO(void);	  //LEE UN CARACTER DEL PUERTO SERIAL. ENVIA EL ECO.
char getcNOECO(void);	  //LEE UN CARACTER DEL PUERTO SERIAL. NO ENVIA ECO.
char timed_getc(void);	  //LEE UN CARACTER DEL PUERTO SERIAL 100 MILISEGUNDOS TIMEOUT.


void activa_serial(void)
{
OpenUSART(USART_TX_INT_OFF & USART_RX_INT_ON &
USART_ASYNCH_MODE & USART_EIGHT_BIT &
USART_CONT_RX & USART_BRGH_LOW, 77);				//SPBRG = 77 PARA 9600 BPS
} 

char timed_getc(void) 					//equivalente a getcECO(), pero con timeout de 100 milisegundos.
{							//si se vence el timeout, la subrutina regresa el valor 0.
long int timeout;
	for(timeout=0;timeout<200000;timeout++)
	{
		if(DataRdyUSART()==1)
		{
		return(getcUSART());
		}
		else continue;
	}							//¿hay caracter en buffer?  
	
	return(0);	
}	


/*FUNCION ESPERA UN CARACTER DEL PUERTO SERIAL, LO LEE Y*/
/*REGRESA CON EL VALOR DEL CARACTER LEIDO*/

char getcNOECO(void)
{
char a;		
		while(!DataRdyUSART());			//ESPERA BANDERA LISTA
		{
		a=getcUSART();						//LEE UN CARACTER DEL USART,NO HAY ECO
		}
return a;								
}

/*FUNCION ESPERA UN CARACTER DEL PUERTO SERIAL, LO LEE Y ENVIA EL ECO*/
/*REGRESA CON EL VALOR DEL CARACTER LEIDO*/

char getcECO(void)
{
char a;				
		while (!DataRdyUSART());			//ESPERA BANDERA LISTA
		{
		a=getcUSART();						//LEE REGISTRO DE RECEPCION DEL USART
		putcUSART(a);						//ENVIA EL ECO
		}
return a;								//REGRESA DE LA FUNCION CON CARACTER LEIDO
}
