/********************************* Include ************************************/
#include <main.h>
#include <lcd.h>

/********************************* Define *************************************/
#fuses NOWDT, NOPROTECT, HS, NOLVP
#use delay(clock=20M)
#define NUT_1 PIN_B4
#define NUT_2 PIN_B5
#define NUT_3 PIN_B6
#define IC_DATA PIN_D0
#define IC_CLK PIN_D1
#define IC_LATCH PIN_D2
#define Out_dir PIN_C5
#define In_dir PIN_B1
#define dt (float) 0.01

#define NONSET 0
#define SET_HUNDREDS 1
#define SET_DOZENS 2
#define SET_UNITS 3
#define RUN 4
#define STOP 5
/****************************** Global Variable *******************************/
unsigned int16 flag=0;
float x1=0;
float x2=0;
float x3=0;
float vi_tri;
int temp,temp1,temp2;
float errorC, errorP,P,I,D,PID,I0;
float KP = 10;
float KI = 0;
float KD = 3;
float SP,CP;
long pulse, f;
long count;
float xSP;                            //Global position for setpoint
float xCP;                            //Global current position
int setPosion = 5;                        //Set position

/*************************** Function Declaration *****************************/
//Internal Function
float round(float A);
void CLK(void);
void LATCH(void);
void Export_LED(unsigned int16 P);
void LED(float D);
void set_dir(int1 dir);
void set_duty(int8 duty);
float cal_position(float x, long f);
float ReadButton();
float PIDOutput(float SP, float CP);
void display_position (float position);
void display_current_setting (int setPosition);
void set_speed (float speed);
float integrate_speed (float position, long pulse);
void display_current_point (float position);
void display_speed (long pulse);

//Init function
void timer1_init (void);
void timer0_init(void);

//Interrupt function
void EXT_isr(void);
void TIMER0_isr(void);
void TIMER1_isr(void);

/****************************** Main Function *********************************/
void main()
{
   //Initialize function
   lcd_init();
   timer0_init();
   timer1_init();
   while(TRUE) {
   //TODO: User Code
      xSP = ReadButton();
      display_position(xSP);
      display_current_setting(setPosion);
      display_current_point(xCP);
      display_speed(f);
      delay_ms(50);
   }
}

/*************************** Function Definition ******************************/
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

void CLK(void) // shift clock
{
   output_low(IC_CLK);
   delay_us(500);
   output_high(IC_CLK);
   delay_us(500);
}

void LATCH(void) // latch clock
{
   output_high(IC_LATCH);
   delay_us(500);
   output_low(IC_LATCH);
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

void set_speed (float speed) {
   float temp;
   if(speed > 0 ) {
      set_dir (0);
      set_duty((int8) speed);
   } else {
      temp = -speed;
      set_dir(1);
      set_duty((int8) temp);
   }
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

float cal_position(float x, long pulse) // feedback CP
{
   float temp;
   if(input(In_dir) == 0)
      temp = -(float) pulse*dt/4;
   else
      temp = (float) pulse*dt/4;
   x = x + temp;
   return x;
}

float integrate_speed (float position, long pulse) {
   float temp = position;
   position = cal_position(temp, pulse);
   if(position > 500)
      position = 500;
   if(position < -500)
      position = -500;
   return position;
}

void display_current_point (float position) {
   LED(position);
   lcd_gotoxy(1, 3);
   printf(lcd_putc, "Current: %04.0f", position);
}
float ReadButton() {
   if(input(NUT_3)==1) {
      if(temp==0) {
         flag=flag+1;
      }
   }
   switch(flag) {
      case 1:
         setPosion = SET_HUNDREDS;
         if(input(NUT_1)==1) {
            if(temp1==0) {
               x1=x1+1;
               if (x1>=6) {
                  x1=5;
               }
            }
         }
         if(input(NUT_2)==1) {
            if(temp2==0) {
               x1=x1-1;
               if(x1 <= -6) {
                  x1 = -5;
               }
            }
         }
      break;

      case 2:
         setPosion = SET_DOZENS;
         if(input(NUT_1)==1) {
            if(temp1) {
               x2=x2+1;
               if(x2 >= 10) {
                  x2=0;
               }
            }
         }
         if(input(NUT_2)==1) {
            if(temp2==0) {
               x2=x2-1;
               if(x2 <= -1) {
                  x2 = 9;
               }
            }
         }
      break;
      case 3:
         setPosion = SET_UNITS;
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
               if(x3 <= -1) {
                  x3 = 9;
               }
            }
         }
      break;
      case 4:
         setPosion = RUN;
      break;
      case 5:
         setPosion = STOP;
         flag=0;
      break;
   }
   if(x1>=0) {
      vi_Tri= x1*100+x2*10+x3;
   }

   if(x1<0) {
      vi_Tri= (-1)*(-x1*100+x2*10+x3);
   }
   if(vi_Tri > 500)
      vi_Tri = 500;
   if(vi_Tri < -500)
      vi_Tri = -500;

   temp=input(NUT_3);
   temp1=input(NUT_1);
   temp2=input(NUT_2);
   return vi_Tri;
}

void display_position (float position) {
   lcd_gotoxy(1,1);
   printf(lcd_putc,"VI TRI:");
   lcd_gotoxy(10,1);
   printf(lcd_putc,"%04.0f",vi_tri);
}

void display_speed (long pulse) {
   float temp;
   if(input(In_dir) == 0)
      temp = -(float) pulse*60/200;
   else
      temp = (float) pulse*60/200;
   lcd_gotoxy(1, 4);
   printf(lcd_putc, "SPEED: %03.0f RPM", temp);
}
void display_current_setting (int setPosition) {
   lcd_gotoxy(1, 2);
   switch(setPosion) {
      case NONSET:
         printf(lcd_putc, "            ");
      break;
      case SET_HUNDREDS:
         printf(lcd_putc, "SET_HUNDREDS");
      break;
      case SET_DOZENS:
         printf(lcd_putc, "SET_DOZENS  ");
      break;
      case SET_UNITS:
         printf(lcd_putc, "SET_UNITS   ");
      break;
      case RUN:
         printf(lcd_putc, "RUN         ");
      break;
      case STOP:
         printf(lcd_putc, "STOP        ");
      break;
   }
}
float PIDOutput(float SP, float CP)
{
   errorC = SP-CP;
   P= KP*errorC;
   I = I + KI*(errorC+errorP)/2*dt;
   D = KD*(errorC-errorP)/dt;
   PID = P+I+D;
   if(PID>250)
   {
      PID=250;
   }
   if(PID < -250)
   {
      PID=-250;
   }
   errorP = errorC;
   return PID;
}

/******************************* Init Function ********************************/
void timer1_init (void) {
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
   set_timer1(15536);
   enable_interrupts(INT_TIMER1);
   enable_interrupts(GlOBAL);
}

void timer0_init(void) {
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_16|RTCC_8_bit);      //1.0 ms overflow
   enable_interrupts(INT_EXT);
   enable_interrupts(INT_TIMER0);
   enable_interrupts(GLOBAL);
   set_timer0(6);
}
/**************************** Interrupt Function ******************************/
#INT_TIMER1
void TIMER1_isr(void)
{
   xCP = integrate_speed(xCP, f);
   output_toggle(PIN_C6);
   if (setPosion == RUN) {
      PID = PIDOutput(xSP, xCP);
      set_speed(PID);
   } else {
      PID = 0;
      errorP = 0;
      set_speed(0);
   }
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
