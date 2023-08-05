#include<avr/io.h>
#include<util/delay.h>
#include<LCD.h>
#include<stdio.h>


const char password[4] = "7474";
char str[100] = "";

int check_pass(int indx){
  LCD_cmd(0x01);
  char ok[30] = "Access is granted";
  char nok[30] = "Wrong password";
  LCD_cmd(0x01);         // clear the screen 
  //if(indx == 4)
    for(int i = 0; i < indx; i++)
      if(str[i] != password[i]){
        for(int j = 0; nok[j]; j++){
          LCD_write(nok[j]);
          // LCD_write('!');
          // LCD_write(password[i]);
          // LCD_write(str[i]);
        }
        return 0;
      }

  for(int i = 0; ok[i]; i++)
    LCD_write(ok[i]);
  SPDR = 0XFF;
  return 1;

}
int isl = 0;
int main() {
    DDRA = 0xFF;
    
    DDRB = (0<<DDB7) | (1<<DDB6) | (0<<DDB5) | (0<<DDB4);

    DDRD = 0xFF;
    DDRC = 0xFF;
    PORTD |= (1 << PD3);

    // SPI initialization
    // SPI Type: Slave
    // SPI Clock Rate: 8MHz / 128 = 62.5 kHz
    // SPI Clock Phase: Cycle Half
    // SPI Clock Polarity: Low
    // SPI Data Order: MSB First
    SPCR = (1<<SPE) | (0<<DORD) | (0<<MSTR) | (0<<CPOL) | (0<<CPHA) | (1<<SPR1) | (1<<SPR0);
    SPSR = (0<<SPI2X);


    init_LCD();
    int shown = 0, ind = 0;
    double duty = 0, lduty = 0;
    SPDR = '0';
    
    //OC0 pin
      DDRB |= (1 << PORTB3);
    
    // Fast PWM
    TCCR0 |= (1 << WGM00) | (1 << WGM01);

    // Clear OC0 on compare match
    TCCR0 |= (1 << COM01);

    // 1/8 clck prescaling
    TCCR0 |= (1 << CS01);


    //OC2 pin
    DDRD |= (1 << PORTD7);
    // Fast PWM
    TCCR2 |= (1 << WGM20) | (1 << WGM21);

    // Clear OC0 on compare match
    TCCR2 |= (1 << COM21);

    // 1/8 clck prescaling
    TCCR2 |= (1 << CS21);



    while(1) {
        
        while (((SPSR >> SPIF) & 1) == 0);
        char tmp = SPDR;

        unsigned int itmp = tmp;
        unsigned int temp = tmp;
        //LCD_write('tmp');
        // if(itmp <= 4){
        //   LCD_write('+');
        // }
        if(isl){ // light
          // if((PORTD >> PD4) & 1 == 0)
          //   LCD_write('F');
          isl ++;
          isl %= 2;
          //LCD_cmd(0x01);
          //LCD_write('G');
          // char ch[10];
          // sprintf(ch,"%u", itmp);
          // for(int i = 0; ch[i]; i++)
          //   LCD_write(ch[i]);

          if(itmp > 65528){
            lduty = 25;
            //LCD_write('A');
          }
          else if(itmp > 65468){
            lduty = 50;
            //LCD_write('B');
          }
          else if(itmp > 113){
            lduty = 75;
            //LCD_write('C');
          }
          else{
            lduty = 100;
            //LCD_write('D');
          }

          OCR2 = (lduty/100)*255;
        }
        else if(itmp >= 20 && itmp != 84 ){
          LCD_cmd(0X01);
          //char ch[10];
          temp -= 20;
          // sprintf(ch,"%d", temp);
          // for(int i = 0; ch[i]; i++)
          //   LCD_write(ch[i]);
          isl ++;
          isl %= 2;
           // temp
          
          /////////////
          //LCD_write('@');
          // if(temp == 27)
          //   LCD_write('?');
          if(temp >= 25 && temp <= 55){
            //cooler must be turned on 25deg -> 50% duty , 10%  for every 5 additional deg
            //it means that 2% for each deg
            //LCD_write('$');
            //PORTD |= (1 << PD4);
            PORTC |= (1 << PC4);
            // if((PIND >> PD4) & 1 == 1)
            //   LCD_write('F');
            int dif = temp - 25;
            dif *= 2;
            duty = 50 + dif;
            if(duty > 100)
              duty = 100;
          }
          else if(temp >= 3 && temp <= 20){
            //if((PORTD >> PD4) & 1 == 0)
            //PORTD &= ~(1 << PD4); // 11101111
            PORTC &= ~(1 << PC4); // 11101111
            //LCD_write('H');
            //heater must be turned on 0deg -> 100% duty , 20%  for every 5 additional deg
            //it means that 5% for each deg
            int dif = temp;
            dif *= 4;
            duty = 100 - dif;
            if(duty < 0)
              duty = 0;

          }
          else if(temp > 55){
            duty = 0;
            //if((PORTD >> PD5) & 1)
            //PORTD ^= (1 << PD5);
            PORTC ^= (1 << PC5);
          }
          else if(temp < 3){
            duty = 0;
            //if((PORTD >> PD5) & 1)
            //PORTD ^= (1 << PD6);
            PORTC ^= (1 << PC6);
          }
          else{
            duty = 0;
          }
          // duty cycle
          double xx = 50;
          OCR0 = (duty/100)*255;
          //   showValLCD(temp);
          //_delay_ms(50);
        }
        else if(itmp == 84){ // shown
          //LCD_write('%');
          shown ++;
          shown %= 2;
          LCD_cmd(0x01);         // clear the screen
          if(shown){
            for(int i = 0; i < ind; i++)
              LCD_write(str[i]);
          }
          else{
            for(int i = 0; i < ind; i++)
                LCD_write('*');
          }
          //_delay_ms(10);
        }
        else if(itmp >= 0 && itmp <= 9){ // it's key pad
          char tmpch;
          if(shown){
            tmpch = '0' + itmp; 
            str[ind] = tmpch;
          }
          else{
            tmpch = '*';
            str[ind] = '0' + itmp;
          }
          LCD_write(tmpch);

          ind ++;
        }
        else if(itmp == 10){
          check_pass(ind);
          ind = 0;
        }
        else if(itmp == 12){
          ind --;
          if(ind < 0)
            ind = 0;
          LCD_cmd(0x01);         // clear the screen 
          if(shown){
            for(int i = 0; i < ind; i++)
              LCD_write(str[i]);
          }
          else{
            for(int i = 0; i < ind; i++)
                LCD_write('*');
          }
          
        }


        //LCD_write('B');
        //LCD_write('B');
        //_delay_ms(20);

    }
}