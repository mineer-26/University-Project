#include "MPU6050.c"
#include "math.h"

#use rtos (timer = 1, minor_cycle = 10ms)

void  Run();
void  left();
void  Right();
void  stop();

#task (rate=100ms, max=10ms)
void display() //HIEN THI LCD
{
  rtos_wait(sem); //co SEMAFOR, SEM=0;
  //
  lcd_gotoxy(1, 1); //hien thi goc
  printf(lcd_putc, "G = %.2f", X_angle_Real);
  lcd_putc(223); printf(lcd_putc, "                ");
  //
  lcd_gotoxy(1, 2);
  if (zone < 7)
    printf(lcd_putc, "Zone = %d        ", zone);
  else if (zone == 7)
    printf(lcd_putc, "Zone = VICTORY.        ");
  rtos_signal(sem); //SEM=1;
}

#task (rate=130ms, max=5ms)      //////////////////////////layzone=5
void dem_zone()
{
  rtos_wait(sem);

  if ((input(PIN_B7) == 1) && (set == 0) && (zone < 7))
  {
    zone++;

    if (zone == 4)
    {
      turn_ = 1;
      if (huong == 1)
        set_angle = 30;  //RE PHAI
      else if (huong == 0)
        set_angle = -28; //RE TRAI

    }

    set = 1;
  }
  else if (input(PIN_B7) == 0) //CHO THOAT RA KHOI VACH => SET=0;
  {
    set = 0;
  }

  rtos_signal(sem);
}

#task (rate=10ms, max=5ms)

void get_angle() //doc cb goc
{
  rtos_wait(sem);

  /*  accX = Mpu6050_GetData (MPU6050_RA_ACCEL_XOUT_H); //GIA TOC GOC
    accY = Mpu6050_GetData (MPU6050_RA_ACCEL_YOUT_H);
    accZ = Mpu6050_GetData (MPU6050_RA_ACCEL_ZOUT_H);
  */
  gyroX = Mpu6050_GetData(MPU6050_RA_GYRO_XOUT_H);  //
  gyroY = Mpu6050_GetData(MPU6050_RA_GYRO_YOUT_H);
  gyroZ = Mpu6050_GetData(MPU6050_RA_GYRO_ZOUT_H);

  X_angle_Real += (double)((gyroX - Offset) * 0.01 / (131.2)); //tinh goc , giá tri offset dduoc do và lay tu phep do

  rtos_signal(sem);
}

#task (rate=50ms, max=10ms)
void timer()
{
  stop();
  rtos_wait(sem);

  err = set_angle - X_angle_Real; //TINH SAI SO
  //he so ip theo tung sai so,


  if (abs(err) < 3)       {
    Ki = 0.3;
    Kp = 165;
  }
  else if (abs(err) < 6)  {
    Ki = 0.2;
    Kp = 144;
  }
  else if (abs(err) < 9)  {
    Ki = 0.05;
    Kp = 125;
  }
  else if (abs(err) < 12) {
    Ki = 0.02;
    Kp = 110;
  }
  else if (abs(err) < 15) {
    Ki = 0.01;
    Kp = 90;
  }
  else {
    Ki = 0;
    I = 0;
  }

  /* if (abs(err)<3)       {Ki=0.3; Kp=150;}
    else if (abs(err)<6)  {Ki=0.2; Kp=100;}
    else if (abs(err)<9)  {Ki=0.05; Kp=120;}
    else if (abs(err)<12) {Ki=0.02; Kp=100;}
    else if (abs(err)<15) {Ki=0.01; Kp=80;}
  */
  I += err * 0.05;
  output = Kp * err + Ki * I;
  if ((zone < 7) && (turn_ == 0))
  {
    //han che dau ra pi
    if (output > 100) output = 100;
    else if (output < -100) output = -100;

    run(); //CHAY THANG
  }
  else if ((zone == 4) && (turn_ == 1))
  {
    speed = 0;

    //HAN CHE
    if (output > 500) output = 500;
    else if (output < -500) output = -500;

    //ki?m tra goc can re
    if (huong == 0)
      left(); //RE TRAI
    else if (huong == 1)
      right(); //RE PHAI

    if (abs(err) < 5)
    {
      Stop(); turn_ = 0; I = 0; output = 0; speed = 550;
    }
  }
  else if (zone == 7)
    Stop();
  //
  set_pwm1_duty((int16)(abs(speed - output)));   //trai
  set_pwm2_duty((int16)(abs(speed + output))); //phai*/

  rtos_signal(sem); //sem=1
}


void main()
{

  Mpu6050_Init () ; //khoi tao cb goc

  //KHAI BAO SU DUNG PWM
  setup_timer_2 (T2_DIV_BY_4, 249, 1);
  setup_ccp1 (CCP_PWM) ;
  setup_ccp2 (CCP_PWM) ;

  // NGAT UART
  enable_interrupts(INT_RDA);
  //NGAT TOAN CUC
  enable_interrupts(GLOBAL);
  //SET PWM BAN DAU = 0
  set_pwm1_duty((INT16)0);
  set_pwm2_duty((INT16)0);

  lcd_init();
  lcd_gotoxy(1, 1);
  printf(lcd_putc, "RTS group");
  delay_ms(1000);

  ////re xe

  if (input(BTN_Re) == 1)
  {
    huong = 1;
  }
  delay_ms(500);
  if (input(BTN_Re) == 0)
  {
    huong = 0;

  }
  delay_ms(500);
  ////////////////////////////////////////////////
  sem = 1;
  rtos_run();
}


void  Run()
{
  if (input(BTN_Start) == 0)

  {
    output_low(IN2); //Dong Co 1
    output_high(IN1);

    output_low(IN3);//Dong co 2
    output_high(IN4);

  }
}

void  left()
{

  output_high(IN2); //Dong Co 1
  output_low(IN1);

  output_high(IN4);//Dong co 2
  output_low(IN3);
  delay_ms(50);
  stop();
  delay_ms(50);
}


void  right()
{

  output_high(IN1); //Dong Co 1
  output_low(IN2);

  output_high(IN3);//Dong co 2
  output_low(IN4);

  delay_ms(50);
  stop();
  delay_ms(50);

}

void  Stop()
{
  output_low(IN1); //Dong Co 1
  output_low(IN2);

  output_low(IN4);//Dong co 2
  output_low(IN3);
}
