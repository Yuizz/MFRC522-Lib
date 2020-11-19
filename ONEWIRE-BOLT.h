//ONEWIRE-BOLT.h
//ARCHIVO AUXILIAR CON LAS FUNCIONES PARA LEER LA 
//TEMPERATURA DEL SENSOR DS18B20 DEL SISTEMA BOLT.
//USA EL PIN RA3 PARA CONECTARSE AL DS18B20 
//LA FUNCION ds1820_read() REGRESA UN VALOR
//DE PUNTO FLOTANTE CON LA TEMPERATURA LEIDA.
//VALOR CON DOS ENTEROS Y UN DECIMAL.
//NO OLVIDE INSERTAR EL SENSOR DS18B20 EN SU BASE
//DE 3 PINES EN EL MODULO BOLT.
//PROGRAMA INICIALMENTE DESARROLLADO PARA 
//EL COMPILADOR CCS,CON MODIFICACIONES Y 
//ADAPTACIONES PARA EL COMPILADOR C18,Y EL SISTEMA 
//18F2550, REALIZADA POR PUNTO FLOTANTE S.A.

float ds1820_read(void);
void onewire_reset(void);
void onewire_write(int data);
int onewire_read(void);


float ds1820_read(void) 
{ 
    int busy=0, temp1, temp2; 
    long signed int temp3,temp4; 
    float result;  
    onewire_reset(); 
    onewire_write(0xCC);            //Skip ROM, address all devices 
    onewire_write(0x44);            //Start temperature conversion 
    while(busy == 0)                //Wait while busy (bus is low) 
    busy = onewire_read(); 
    onewire_reset(); 
    onewire_write(0xCC);            //Skip ROM, address all devices 
    onewire_write(0xBE);            //Read scratchpad 
    temp1 = onewire_read(); 
    temp2 = onewire_read(); 
    temp3 = temp1+(temp2*256);									  
    result=(float)temp3/16;													    
    delay_ms(200); 
    return result;  
}

void onewire_reset(void) 
{
	TRISA=0X00; 						//RA4 salida
    	RA3=0;						      // RA4=0 
    	delay_ms(1); 
	TRISA=0X08; 				//Alta impedancia 
	delay_ms(1);				//espera fin de pulso
} 

void onewire_write(int data) 
{ 
    int i,count; 
    	for(count = 0; count < 8; ++count) 
			{
        	TRISA=0X00; 			 
			RA3=0;
        	delay_us(2);                // pull 1-wire low to initiate write time-slot. 
			i=(data&0x01);				//output_bit(ONE_WIRE_PIN, shift_right(&data, 1, 0)); // set output bit on 1-wire 
				if(i==1)RA3=1;
				if(i==0)RA3=0;			  								
        	delay_us(60);               // wait until end of write slot. 
        	TRISA=0X08; 			//output_float(ONE_WIRE_PIN); // set 1-wire high again, 
        	delay_us(2);                // for more than 1 us minimum. 
			data=(data>>1);  
    		} 
} 

int onewire_read(void) 
{ 
    int count, data; 
    for(count = 0; count < 8; ++count) 
		{ 
		TRISA=0X00; 									//RA4 es salida; 
		RA3=0;									//output_low(ONE_WIRE_PIN); 
        delay_us(2);                // pull 1-wire low to initiate read time-slot. 
		TRISA=0X08;					//RA4 ES ENTRADA
        delay_us(8);                // let device state stabilise, 
		data=data>>1;				//shift_right(&data, 1, input(ONE_WIRE_PIN)); // and load result. 
			if(RA3==1)
			{
			data=(data|0x80);			//MSB=1
			}
			if(RA3==0)
			{
			data=(data&0x7F);			//MSB=0
			}
        delay_us(120);              // wait until end of read slot. 
    	} 
    return data; 
} 

