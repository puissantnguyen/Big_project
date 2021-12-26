#include<16f877A.h>
#device ADC=10
#include <lcd.c>
#fuses NOWDT, NOPROTECT, HS, NOLVP
#use delay(clock=20M)
#define NUT_1 PIN_B4
#define NUT_2 PIN_B5
#define NUT_3 PIN_B6
#INT_RDA

unsigned int16 flag=0;
float x1=0;
float x2=0;
float x3=0;
float vi_tri;
int temp,temp1,temp2;
float errorC, errorP,P,I,D,PID,I0;
float KP;
float KI;
float KD;
float IO=0;
float dt=0.0131072;
float SP,CP;

void ReadButton() {
   if(input(NUT_3)==1) {
      if(temp==0) {
         flag=flag+1;
      }
   }
   switch(flag) {
   case 1:
      if(input(NUT_1)==1) {
         if(temp1==0) {
            x1=x1+1;
            if (x1>=10) {
               x1=0;
            }
         }
      }
      if(input(NUT_2)==1) {
         if(temp2==0) {
            x1=x1-1;
         }     
      }
   lcd_gotoxy(1,2);
   printf(lcd_putc,"SET_TRAM");
   //delay_ms(100);
   lcd_init();
   break;

   case 2:
   if(input(NUT_1)==1) {
      if(temp1) {
         x2=x2+1;
         if(x2>=10) {
            x2=0;    
         }
      }
   }
   if(input(NUT_2)==1) {
      if(temp2==0) {
         x2=x2-1;
      }
   }
   lcd_gotoxy(1,2);
   printf(lcd_putc,"SET_CHUC");
   //delay_ms(100);
   lcd_init();
   break;

   case 3:
   if(input(NUT_1)==1) {
      if (temp1==0) {
         x3=x3+1;
         if(x3>=10) {
            x3=0;
         }
      }
   }
   if(input(NUT_2)==1) {
      if (temp2==0) {
         x3=x3-1;
      }
   }
   lcd_gotoxy(1,2);
   printf(lcd_putc,"SET_DV");
   //delay_ms(100);
   lcd_init();
   break;

   case 4:
      flag=0;
   break;
   }

   if(x1>=0) {
      vi_Tri= x1*100+x2*10+x3;
   }

   if(x1<0) {
      vi_Tri= (-1)*(-x1*100+x2*10+x3);
   }

   temp=input(NUT_3);
   temp1=input(NUT_1);
   temp2=input(NUT_2);
   lcd_gotoxy(1,1);
   printf(lcd_putc,"VI TRI:");
   lcd_gotoxy(10,1);
   printf(lcd_putc,"%3.0f",vi_tri);
   //delay_ms(100);
   lcd_init();
}

float PIDOutput(float SP, float CP)
{
   errorC = SP-CP;
   P= KP*errorC;
   I = IO+KI*(errorC+errorP)/2*dt;
   D = KD*(errorC-errorP)/dt;
   PID = P+I+D;
   if(PID>250)
   {
      PID=250;
   }
   if(PID<=-250)
   {
      PID=-250;
   }
   IO=I;
   return PID;
}

#int_TIMER1
void TIMER1_isr(void) 
{
  PIDOutput(SP,CP);
}
void main()
{
lcd_init();
delay_ms(500);
set_tris_b(0b11111111);
set_tris_a(0b111111);
set_tris_c(0x00);

setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
SET_TIMER1(0);
enable_interrupts(INT_TIMER1);
enable_interrupts(GlOBAL);


while(1)
{
    ReadButton();
}
}



