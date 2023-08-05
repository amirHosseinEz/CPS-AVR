#include<avr/io.h>
#include<util/delay.h>
#include<avr/interrupt.h>
#include<LCD.h>
unsigned char k[4][3]={{0x01,0x02,0x03},{0x04,0x05,0x06},{0x07,0x08,0x09},{0x0A,0x00,0x0C}};
int mod = 0;
int getADCVal();
// void showValLCD(float);
float convertADCValueToTemperture(int);
int main() {
    //DDB5 : MO,  DDB6 : MI
    DDRB = (1<<DDB7) | (0<<DDB6) | (1<<DDB5) | (1<<DDB4) | (1<<DDB3);
    PORTB = (1<<PORTB4) | (1<<PORTB3); // slave sel

    DDRD |= (1 << 0);
    PORTD |= (1 << 0);


    // SPI initialization
    // SPI Type: Master
    // SPI Clock Rate: 8MHz / 128 = 62.5 kHz
    // SPI Clock Phase: Cycle Half
    // SPI Clock Polarity: Low
    // SPI Data Order: MSB First
    SPCR = (1<<SPE) | (0<<DORD) | (1<<MSTR) | (0<<CPOL) | (0<<CPHA) | (1<<SPR1) | (1<<SPR0);
    SPSR = (0<<SPI2X);

	unsigned char c,r;
	DDRC=0xf0;
	PORTC=0xff;

    GICR |= (1<<INT0);
    MCUCR = 0x03;

    sei();

	ADMUX = (0 << REFS1) | (1 << REFS0);
	
  	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0) ; // teperture
	//OC0 pin
  	DDRB |= (1 << PORTB3);
	// Fast PWM
	TCCR0 |= (1 << WGM00) | (1 << WGM01);

	// Clear OC0 on compare match
	TCCR0 |= (1 << COM01);

	// 1/8 clck prescaling
	TCCR0 |= (1 << CS01);

    while(1) {
        //key pad

        //_delay_ms(10);
		int tmp;
		if(mod == 0){
			do
			{
				PORTC&=0x0f;
				r=PINC&0x0f;
			} while(r!=0x0f);
		

			do
			{
				//_delay_ms(20);
				r=PINC&0x0f;
			}	  while(r==0x0f);
			
			while(1)
			{
				PORTC=0xef;     //0b11101111
				r=PINC&0x0f;
				if(r!=0x0f)
				{
					c=0;
					break;
				}
				PORTC=0xdf;     //0b11011111
				r=PINC&0x0f;
				if(r!=0x0f)
				{
					c=1;
					break;
				}
				PORTC=0xbf;     //0b10111111
				r=PINC&0x0f;
				if(r!=0x0f)
				{
					c=2;
					break;
				}

			}
			
			if(r==0x0e)
			tmp=k[0][c];
			else if(r==0x0d)
			tmp=k[1][c];
			else if(r==0x0b)
			tmp=k[2][c];
			else
			tmp=k[3][c];
		}
        //int tmp = PORTD;
        char tmpch;
        //if(tmp <= 9)
        //    tmpch = '0' + tmp;
       // else
        tmpch = tmp;
        /////////////////////////////////
		if(mod == 1){// sending temperture and lighting data

			tmpch = 0X07;
			ADMUX = (0 << REFS1) | (1 << REFS0);
			ADCSRA |= ((1 << ADSC) | (1 << ADIF));
    		while((ADCSRA & (1 << ADIF)) == 0){};
			int val = getADCVal();
    		int temp = convertADCValueToTemperture(val);
			
			PORTB &= ~(1<<PORTB4); // Select Slave #1

			
			SPDR = temp + 20; // send 'A'
			while(((SPSR >> SPIF) & 1) == 0);
      		  //ignore = SPDR;


       		PORTB |= (1<<PORTB4); // Deselect Slave #1

			_delay_ms(50);
			//////////////

			ADMUX = (0 << REFS1) | (1 << REFS0) | (1 << MUX0);
			ADCSRA |= ((1 << ADSC) | (1 << ADIF));
    		while((ADCSRA & (1 << ADIF)) == 0){};
			int val2 = getADCVal();
    		int temp2 = val2;
			
			PORTB &= ~(1<<PORTB4); // Select Slave #1

			
			SPDR = temp2; // send 'A'
			while(((SPSR >> SPIF) & 1) == 0);
			PORTB |= (1<<PORTB4); // Deselect Slave #1
			_delay_ms(50);

			
		}
		else{
			PORTB &= ~(1<<PORTB4); // Select Slave #1

			
			SPDR = tmpch; // send 'A'
			while(((SPSR >> SPIF) & 1) == 0);
			if(SPDR == 0XFF)
				mod = 1;
        //ignore = SPDR;


       		 PORTB |= (1<<PORTB4); // Deselect Slave #1

		}
        


        // PORTB &= ~(1<<PORTB4); // Select Slave #2

        // SPDR='B'; // send 'B'
        // while(((SPSR >> SPIF) & 1) == 0);
        // ignore = SPDR;

        // PORTB |= (1<<PORTB4); // Deselect Slave #2
    }
	//_delay_ms(100);
}
ISR (INT0_vect){
    PORTD ^= (1 << 0);
    PORTB &= ~(1<<PORTB4); // Select Slave #1

    SPDR = 'T'; // send 'A'
    while(((SPSR >> SPIF) & 1) == 0);
    //ignore = SPDR;

    PORTB |= (1<<PORTB4); // Deselect Slave #1

}


int getADCVal(){
  int result = (int)ADCL + ((int)ADCH * 256);  
  // if(ADCL == 0)
  //   return 77;
  return result;
}

// void showValLCD(float temp){
//   char str[100];
//   dtostrf(temp, 4, 2, str);
//   LCD_cmd(0x01);
//   _delay_ms(2);
//   for(int i = 0; str[i] != 0; i++)
//     LCD_write((char)str[i]);
  

// }

float convertADCValueToTemperture(int val){
  //return (float)val;
  return val * (5 / 1024.0) * 100;

}