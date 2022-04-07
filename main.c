/*---------------------------------------
THIS SCRIPT WAS MADE FOR ACADEMIC PURPOSE
BY TOMÁS FERREIRA AND GONÇALO MORAIS
ISEP (PORTUGAL)
----------------------------------------*/

/*----------------------------------------
SCRIPT FOR ATMEGA328P
READ THE VALUES OF 2 LDR SENSORS 
AND CONTROL A SERVO MOTOR 
----------------------------------------*/

#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/delay.h>
#include <stdio.h>

volatile uint8_t tot_overflow;
volatile unsigned int x=0;
volatile unsigned int y=0;

unsigned int adc_0 = 0;
unsigned int adc_1 = 0;
unsigned int adc_2 = 0;
unsigned int adc_3 = 0;

int adc_read(int ch)
{
	//FUNÇAO DE LEITURA DOS ADCS, ch SERVE PARA SO FUNCIONAR UM VALOR DE 0 A 7 QUE SÃO O Nº DE ADCS DISPONIVEIS
	//CORRE UM LOOP CONTINUAMENTE PARA ESTAR CONSTANTEMENTE A LER VALOR ANALOGICOS E CONVERTER EM DIGITAIS
	//retorna valor de 0 a 1023 (1023 - 5V e 0 - 0V)
	ch &= 0b00000111;
	ADMUX = (ADMUX & 0xF8)|ch;
	
	ADCSRA |= (1<<ADSC);
	
	while(ADCSRA & (1<<ADSC));
	
	return (ADC);
}

ISR(TIMER0_OVF_vect)
{
	tot_overflow++;
}


ISR(TIMER1_OVF_vect)
{
	
}


void adc_setup()
{
	
	//ADC SETUP
	ADMUX = 0b01100000 ; // AVCC ligado aos 5V/ AREF liga ground
	ADCSRA = 0b10000110; //(1 << ADEN) DIVISION FACTOR 64
	ADCSRA |= (1 << ADSC);	//Começa a conversão
	
}


void init()
{
	DDRD = 0b00000111;				//SAIDAS LED RGB
	PORTD = 0b00000000;
	
	DDRB=0b00000111;				//SAIDAS OCR1A E OCR1B PARA SERVO
	PORTB=0b00000110;
	
	//TIMER0
	//TIMER 8 BITS PROGRAMADO COM OVERFLOWS PARA CONFIGURACAO DE LED A PISCAR COM F = 1Hz
	TCCR0B |= (1 << CS02); //prescaler = 256
	TCNT0 = 0; //incializa counter
	TIMSK0 |= (1 << TOIE0); //ativa interrupcao overflow
	tot_overflow = 0; //variavel controlo overflow
	
	
	//TIMER 1
	//TIMER 16 BITS PARA FUNCIONAMENTO DO SERVO, PRESCALER 64, SERVO A FUNCIONAR A 50HZ LOGO ICR1 = 2499
	//ENABLE A OCR1A E OCR1B PARA CONTROLO NORTE-SUL (OCR1B) ESTE-OESTE (OCR1A)
	
	TCCR1A = 0;
	TCCR1A	|= (1 << COM1A1) | (0 << COM1A0) | (1 << COM1B1) | (0 << COM1B0) | (1 << WGM11) | (0 << WGM10);
	TCCR1B	|= (1 << WGM13) | (1 << WGM12) | (1 << CS11) | (1 << CS10);
	TIMSK1  |= (1 << TOIE1);
	ICR1  = 2499;
	TCNT1 = 0;
	OCR1A = 187;      // 0º
	OCR1B = 187;
	
	sei();	
}

int main(void)
{	
	init();
	adc_setup();
	
	while (1)
	{
		
		if(tot_overflow >= 122) //VALOR DE OVERFLOW PARA 1Hz
		{
			if (TCNT0 >= 18)
			{
				PORTB ^= (1 << 0); //ATIVA/DESATIVA LED
				TCNT0 = 0;
				tot_overflow = 0; //TOTAL OVERFLOW REPOSTO A 0, PARA RECOMECAR CONTAGEM
			}
		}
		
		//LEITURA 4 ADC ONDE CADA VARIAVEL VAI TER UM VALOR DE 0 A 1023
		adc_0 = adc_read(0);
		adc_1 = adc_read(1);
		adc_2 = adc_read(2);
		adc_3 = adc_read(3);
		
		//DIFERENÇA ABSOLUTA ENTRE OS SENSORES
		//SE A DIFERENÇA ENTRE ELES FOR MAIOR QUE 1000 = 4.88V O PAINEL RODA PARA O LADO DO SENSOR QUE TIVER MAIS LUZ
		x = (abs((adc_2-adc_3)));
		y = (abs((adc_0-adc_1)));
		
		
		//CONDIÇÃO PARA LUZ RGB FUNCIONAR
		//CADA TIPO DE ROTAÇÃO ESTÁ PARA UMA COR
		if ((x < 1000) | (y < 1000) )
		{
			PORTD = (0 << 0) | (0 << 1) | (0 << 2);
		}
		
		//Este - Oeste
		//CONDIÇÃO PARA MOVIMENTO DOS SERVOS EM FUNÇÃO DO LDR
		if ( ( x > 1000 ) )
		{
			if (adc_2>adc_3)
			{
				OCR1A=OCR1A+1;
				_delay_ms(200);
				PORTD = (1 << 0) | (0 << 1) | (0 << 2);
			}
			
			if (adc_3>adc_2)
			{
				OCR1A=OCR1A-1;
				_delay_ms(200);
				PORTD = (0 << 0) | (1 << 1) | (0 << 2);
			}
			
			if (OCR1A<125 )
			{
				OCR1A=125;
				_delay_ms(200);
			}
			
			if (OCR1A>250)
			{
				OCR1A=250;
				_delay_ms(200);
			}
		}
		
		
		//NORTE - SUL
		if ( (y > 1000))
		{
			if (adc_0>adc_1)
			{
				OCR1B=OCR1B+1;
				_delay_ms(200);
				PORTD = (0 << 0) | (0 << 1) | (1 << 2);
			}
			if (adc_1>adc_0)
			{
				OCR1B=OCR1B-1;
				_delay_ms(200);
				PORTD = (0 << 0) | (1 << 1) | (1 << 2);
			}
			if (OCR1B<125)
			{
				OCR1B=125;
				_delay_ms(200);
			}
			if (OCR1B>250)
			{
				OCR1B=250;
				_delay_ms(200);
			}
		}
	}
}
