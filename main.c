#include <main.h>

#define IC_DATA PIN_D0
#define IC_CLK PIN_D1
#define IC_LATCH PIN_D2
#define Out_dir PIN_C5
#define In_dir PIN_B1
#define dt 10e-3


long pulse, f;
long count;
float x;

float round(float A)
{
   A = A*10;
   if(A>=0){
      if((int)A%10 >= 5) {
         A+=10;
      }     
   } else {
      if((int)-A%10 >= 5) {
         A-=10;
      }
   }
   A = A/10;
   return A;
}

#INT_EXT
void EXT_isr(void) // pulse count 
{
   pulse++; 
}

#INT_TIMER0
void  TIMER0_isr(void) //1.0ms over
{
   count++;
   if(count>1250)
   {
      count=0;
      f=pulse; 
      pulse=0;// pulse remove
   }
}

void set_pin(int16 pin) 
{
   output_high(pin);
}

void reset_pin(int16 pin) 
{
   output_low(pin);
}

void CLK() // shift clock 
{  
   reset_pin(IC_CLK);
   delay_us(500);
   set_pin(IC_CLK);
   delay_us(500);
}

void LATCH() // latch clock
{
   set_pin(IC_LATCH);
   delay_us(500);
   reset_pin(IC_LATCH);
   delay_us(500);
}

void Export_LED(unsigned int16 P) // export led
{
   #bit DB = P.15
   for(unsigned int i=0; i<16; i++)
   {
      output_bit(IC_DATA, DB);
      CLK();
      P=P<<1;
   }
   LATCH();
}

void LED(float D) // Led
{
   unsigned int16 array[11] = { 0x07E0, 0x03E0, 0x01E0, 0x00E0, 0x0060, 0x0020,
                                0x0030, 0x0038, 0x003C, 0x003E, 0x003F};
   float i;
   i = round(D/100 + 5);
   Export_LED(array[(int)i]);
}


void set_dir(int1 dir) // dir = 0 : CW , dir = 1 : CCW
{
   switch(dir)
   {
      case 1:
      output_high(Out_dir);
      break;
      case 0:
      output_low(Out_dir);
      break;
   }
}

void set_duty(int8 duty) // duty ~ 8bit, 0 - 255
{
   setup_timer_2(T2_DIV_BY_16, 255, 1); 
   setup_ccp1(CCP_PWM); 
   set_pwm1_duty(duty); 
}

float Position(float x, long f) // feedback CP
{
//!   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_16|RTCC_8_bit);      //1.0 ms overflow
//!   enable_interrupts(INT_EXT);
//!   enable_interrupts(INT_TIMER0);
//!   enable_interrupts(GLOBAL);
//!   set_timer0(6); 
   x = x + f*dt/200;
   return x;
}

#include <lcd.c>


void main()
{

   set_dir(0);
   set_duty(255);

   LED(413);
   
//!   lcd_init();
     
   while(TRUE)
   {
   //TODO: User Code

//!   lcd_gotoxy(1,1);
//!   printf(lcd_putc, "Toc do: %frpm", v);
   }
}