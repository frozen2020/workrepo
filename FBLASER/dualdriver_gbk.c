/*双路光纤激光器控制器固件驱动程序       作者：许卫星
/* 晶振频率24M*/

#include<stdio.h>
#include<math.h>
#include<intrins.h>
#include<STC_NEW_8051.h>


#define  _Nop() _nop_()              // 定义空指令
#define  uchar unsigned char
#define  uint  unsigned int
typedef unsigned char BYTE;
#define  INBUF_LEN 15                //数据长度
#define comm  0
#define dat   1


uchar  inbuf0[INBUF_LEN];            //读取数据缓冲区
uchar  inbuf[INBUF_LEN];             //处理数据缓冲区
uint   idata AD_inbuf[10];           //AD采集数据
uint  count=1000;
//uchar code tab1[]={"光纤激光控制器"};
//uchar code tab2[]={"苏州德龙激光有限公司"};
//uchar code tab3[]={"正在初始化"};
//uchar code tab4[]={"请稍候......"};
//uchar code tab5[]={"初始化完成"};
//uchar code tab6[]={"SED:"};
//uchar code tab7[]={"AMP:"};
//uchar code tab8[]={"VOTEC:1.42v"};
//uchar code tab9[]={"设定电流"};
//uchar code tab10[]={"设定温度"};
//uchar code tab11[]={"℃"};
//uchar code tab12[]={"POW:"};
//uchar code tab13[]={"设定功率"};
//uchar code tab14[]={"℃ APC"};
//uchar code tab15[]={"℃ ACC"};
//uchar code tab16[]={"系统采集控制模式中  "};
//uchar code tab17[]={"...................."};
uchar idata  TEC_parameter1[6];
uchar idata  TEC_parameter2[6];
uchar idata  Diode_parameter1[6];
uchar idata  Diode_parameter2[6];
uchar idata  Power_parameter1[6];
uchar idata  Power_parameter2[6];
uchar  set_flag;
uchar RFON;
bit     Sys_mod;                            //系统工作模式  1:主动模式，0：系统采集模式

uint   idata MCU_AD_code;
uint   idata AD_code;
uint   idata Laser_Set_curent1;             //设定种子源激光二极管电流值
uint   idata Laser_Set_curent2;             //设定放大级激光二极管电流值
uint   idata Laser_Fed_curent1;             //实际种子源激光二极管电流值
uint   idata Laser_Fed_curent2;             //实际放大级激光二极管电流值
uint   idata Laser_meds_curent1;            //种子源中间设定电流
uint   idata Laser_meds_curent2;            //放大级中间设定电流 
uint   init_current1;                       //种子源初始电流
uint   init_current2;                       //放大级初始电流
uint   init_flag;                           //初始电流标志位 0：不启用 1：仅放大级 2：仅种子源 3：双路

uint   idata Laser_Set_power1;             //设定种子源激光功率
uint   idata Laser_Set_power2;             //设定放大级激光功率
uint   idata Laser_Fed_power1;             //种子源实际激光功率
uint   idata Laser_Fed_power2;             //放大级实际激光功率
uint   idata Laser_meds_power1;            //种子源中间设定功率
uint   idata Laser_meds_power2;            //放大级中间设定功率

float  idata TEC_Set_Tempture1;             //种子源设定冷却温度
float  idata TEC_Set_Tempture2;             //放大级设定冷却温度
float  idata TEC_Fed_Tempture1;             //种子源当前冷却温度
float  idata TEC_Fed_Tempture2;             //放大级当前冷却温度
uint   idata TEC_meds_tempture1;            //种子源中间设定温度
uint   idata TEC_meds_tempture2;            //放大级中间设定温度

uint   idata Key_V;                         //按键电压

uint   idata PDiode_V1;                     //监测种子源电流二极管反馈电压
uint   idata PDiode_V2;                     //监测放大级电流二极管反馈电压
uint   idata TEC_VO1;                       //种子源TEC两端电压值
uint   idata TEC_VO2;                       //放大级TEC两端电压值
uint   pwm_pulserate;
uint   pwm_pulsewidth;
uint   pwm_pulsedelay;


uchar  t=0;                                 // 缓冲区数据段

bit  read_flag=0;                           // 读取标志位
bit  las_di_ok=0;                           //激光二极管电流，1－OK


//////////////////////////输入控制位/////////////////////////////////////////////////////

sbit KEYB=P1^0;                      //3键盘输入（模拟量）



sbit TMPGD1=P2^3;                    //TEC温度OK ，H－OK          1为种子源驱动
sbit TMPGD2=P2^7;                    //TEC温度OK ，H－OK          2为放大级驱动
sbit DOUT=P1^6;                      //AD串行输出

sbit CONT_MOD=P4^3;                  //控制模式识别，1为APC，0为ACC

//////////////////////////输出控制位/////////////////////////////////////////////////////
sbit LDAC=P2^2;                      //DA装载信号
sbit CSDA=P4^0;                      //DA选通信号
sbit DINDA=P2^1;                     //DA串行数据输入
//sbit SCLK=P1^7;                    //DA时钟输入         09.02 P1.7修改成模拟量输入
sbit SCLK=P2^0;                      //DA时钟输入
sbit SHDN1=P3^6;                     //关闭TEC温控，L－关闭
sbit SHDN2=P2^6;                     //关闭TEC温控，L－关闭

sbit FS=P3^7;                        //DA帧信号
sbit ENAB=P4^4;                      //LCD使能信号，下降沿
sbit RW=P4^5;                        //LCD读写信号，H－read，L－write
sbit RS=P4^1;                        //LCD选通信号 ，H－date，L－instruction
sbit LIM1=P3^5;                      //激光二极管过流中断信号
sbit LIM2=P2^5;                      //激光二极管过流中断信号
//sbit INT1=P3^3;                    //外部中断信号1（备用）
sbit DCLK=P1^7;                      //AD时钟信号
sbit ADIN=P1^5;                      //AD串行数据输入
sbit CSAD=P2^4;                      //AD选通信号，L－选通 

uint AD_Convert(uchar Channel);      //AD转换
void DA_Convert(uchar Channel, uint Dcode);      //DA转换
//void Set_TEC_TEMP(uint tempture);              //设定冷却温度
void Set_laser_current(uchar Channel,uint current);    //设定激光二极管电流
void Set_laser_power(uchar Channel,uint power);        //设定激光功率
void MCU_ADC(BYTE CHN);                      //单片机内部AD转换
//void Key_Button();                           //按键判别
void TEC_V_Check();                          //TEC两端电压
void Diode_curent_Check();                   //激光二极管电流检测
void Laser_power_Check();                    //激光功率检测
void Pdiod_V_Check();                        //激光二极管监测电压
void TEC_Temp();                             //TEC温度
void Error_Process();                        //异常处理
void wr_lcd (uchar dat_comm,uchar content);
//void Lcd_disp();
void Pwm_Set(uint,uint,uint);
void Sdio_check();                     // 加电流时状态检查

void serial_initial();                 // 串口初始化
void stat_initial();                   // 状态初始化
void INT_0();                          // 外部中断0
void delay(uint);                      // 长延时
void delayR(uint);                     // 短延时
void RS232();                          // 串口中断



void command();                        // 指令判断
void setfault();                       // 设置错误
void setsucced();                      // 设置成功
void alarm();                          // 警告



/***************向串口发送一个字符串*****************/
void send_string_com(uchar *str,uchar strlen)
{
    uchar k=0;
    do
    {
        SBUF=*(str + k);
        while(TI==0);
        TI=0;
        k++;
    } while(k < strlen);
}
/************************************************************************/




/***********以下是串口初始化程序********************************************/
void serial_initial()
{


      PCON |= 0x80;		//使能波特率倍速位SMOD
      SCON = 0x50;		//8位数据,可变波特率
      BRT = 0x64;		//设定独立波特率发生器重装值
      AUXR |= 0x04;		//独立波特率发生器时钟为Fosc,即1T
      AUXR |= 0x01;		//串口1选择独立波特率发生器为波特率发生器
      AUXR |= 0x10;		//启动独立波特率发生器

      TMOD=0x21;                //T1工作在方式2（8位模式）T工作在方式1（16位模式）
      AUXR &= 0x3f;             //T0,T1工作在1T/12

      IPH=0x14;
      IP=0x24;
      ES=1;
      ET0=1;
      IT0=1;
      EX0=1;
      IT1=1;
      EX1=1;
      EA=1;
      TL0 = 0x30;   //设置定时初值
      TH0 = 0xF8;   //设置定时初值
      TF0 = 0;    //清除TF0标志
      TR0 = 0;    //定时器0开始计时

      return;
}

/**********************************************************************/




/***********以下是状态初始化程序*********************************/
void stat_initial()
{

     ADC_RES=0;
     ADC_RESL=0;

     read_flag=0;          //读取标志清零
     las_di_ok=0;
     t=0;
     set_flag=0;
     Sys_mod=1;

     AD_code=0;
//     Laser_Set_curent=0;
     Laser_Fed_curent1=0;
     Laser_Fed_curent2=0;
     Laser_Fed_power1=0;
     Laser_Fed_power2=0;

/**************DA引脚初始化*************************/
     CSDA=1;
     LDAC=1;
     FS=0;
     SCLK=0;
     DINDA=0;
/**************************************************/
     DA_Convert(2,1273);                          //设定种子源TEC温度为25度
     _Nop();
     DA_Convert(2,1273);
     _Nop();
     DA_Convert(1,1273);                          //设定放大级TEC温度为25度
     _Nop();
     DA_Convert(1,1273);
/**************************************************/
  if(CONT_MOD)                                  //CONT_MOD: 1 APC, 0 ACC

///////////////////////////////////APC模式设定初始功率//////////////////////////////////////////
 {
  Laser_Set_power1=3;                        //记录种子源当前设定功率值
  Laser_Set_power2=3;                        //记录放大级当前设定功率值

  DA_Convert(3,25);                          // 种子源
   _Nop();
  DA_Convert(3,25);

  DA_Convert(0,25);                          // 放大级
   _Nop();
  DA_Convert(0,25);
 }
//////////////////////////////////////////////////////////////////////////////////////////////

  else
//////////////////////////////////ACC模式设定初始电流//////////////////////////////////////////
 {
  Laser_Set_curent1=30;                          //记录种子源当前设定电流值
  Laser_Set_curent2=30;                          //记录放大级当前设定电流值

  DA_Convert(3,92);                            // 种子源
   _Nop();
  DA_Convert(3,92);

  DA_Convert(0,89);                            // 放大级
  _Nop();
  DA_Convert(0,89);
 }
//////////////////////////////////////////////////////////////////////////////////////////////



     P4SW=0x30;            //设定P4.4、P4.5为I/O功能
     P1M1=0x07;
     P1M0=0x00;

     CSAD=1;
     DCLK=1;
     DOUT=1;





/*************TEC控制器初始化*********************/
     SHDN1=1;
//   TMPGD1=1;
     LIM1=1;

     SHDN2=1;
//   TMPGD2=1;
     LIM2=1;
/*************************************************/

/*    Power_parameter[0]='P';
      Power_parameter[1]='O';
      Power_parameter[2]='W';
      Power_parameter[3]=':';

      Diode_parameter[0]='L';
      Diode_parameter[1]='D';
      Diode_parameter[2]='C';
      Diode_parameter[3]=':';
      Diode_parameter[8]=0x20;
      Diode_parameter[9]=0x20;

      TEC_parameter[0]='T';
      TEC_parameter[1]='E';
      TEC_parameter[2]='C';
      TEC_parameter[3]=':';
*/

     TR0=0;
     ET0=0;
     TR1=0;
     ET1=0;
     EX0=0;

     return;
}

/***************************************************************/



/****************************操作指令执行***********************/
void command()
{

    ///////////////////////////板卡初始化指令////////////////////////////////////////////
     if((inbuf[0]=='I')&&(inbuf[1]=='N')&&(inbuf[2]=='I')&&(inbuf[3]=='T')&&(inbuf[4]=='I'))
        {
           stat_initial();
           serial_initial();
           setsucced();
        }

///////////////////////////////////设定电流////////////////////////////////////////////
       else if((inbuf[0]=='L')&&(inbuf[1]=='D')&&((inbuf[2]=='1')||(inbuf[2]=='2'))&&(inbuf[3]=='='))

           {
               uint dio_cu;
               uchar CH;

               if(inbuf[5]==0x0d)                                        //格式为LD1(2)=N
              {
                 dio_cu=inbuf[4]-48;


              }
               else if(inbuf[6]==0x0d)                                   //格式为LD1(2)=NN
              {
                 dio_cu=(uint)((inbuf[4]-48)*10+(inbuf[5]-48));

              }
              else if(inbuf[7]==0x0d)                                    //格式为LD1(2)=NNN
              {
                 dio_cu=(uint)((inbuf[4]-48)*100+(inbuf[5]-48)*10+(inbuf[6]-48));

              }

              else                                                    //格式为LD1(2)=NNNN
              {
                  setfault();
                  goto setcf;
              }

              if(inbuf[2]=='1')
              {
                CH=3;                                     //种子源SEED
                Set_laser_current(CH,dio_cu);
                setsucced();
              }
              else if(inbuf[2]=='2')
              {
                CH=0;                                    //放大级AMP
                Set_laser_current(CH,dio_cu);
                setsucced();
              }
              else setfault();

setcf:       _Nop();

           }


///////////////////////////////////设定初始电流////////////////////////////////////////////
       else if((inbuf[0]=='I')&&(inbuf[1]=='N')&&(inbuf[2]=='I')&&(inbuf[3]=='C')&&(inbuf[4]=='='))

           {
               uint init_current;
      
               if(inbuf[7]==0x0d)                                        //格式为INIC=N
              {
                 init_current=inbuf[6]-48;


              }
               else if(inbuf[8]==0x0d)                                   //格式为INIC=NN
              {
                 init_current=(uint)((inbuf[6]-48)*10+(inbuf[7]-48));

              }
              else if(inbuf[9]==0x0d)                                    //格式为INIC=NNN
              {
                 init_current=(uint)((inbuf[6]-48)*100+(inbuf[7]-48)*10+(inbuf[8]-48));

              }

              else                                                    //格式为INIC=NNNN
              {
                  setfault();
                  goto seticf;
              }

              if(inbuf[5]=='S')
              {
                init_current1=init_current;
              }
              else if(inbuf[5]=='A')
              {
                init_current2=init_current;
              }
              else setfault();

seticf:       _Nop();

           }



///////////////////////////////////设定初始化模式 0/1/2/3 ////////////////////////////////////////////
       else if((inbuf[0]=='I')&&(inbuf[1]=='N')&&(inbuf[2]=='I')&&(inbuf[3]=='E')&&(inbuf[4]=='='))

           {   
               uint init_flag_temp;
           
               if(inbuf[6]==0x0d)                                        
              {
                 init_flag_temp=inbuf[5]-48;
                if ((init_flag_temp<0)||(init_flag_temp>3))
                  goto setief;
                else
                  init_flag=init_flag_temp;
              }
              else                                                    
              {
                  setfault();
                  goto setief;
              }

setief:       _Nop();

           }


 //////////////////////////////设定功率//////////////////////////////////////////////////
        else if((inbuf[0]=='P')&&((inbuf[1]=='1')||(inbuf[1]=='2'))&&(inbuf[2]=='='))
            {
               uint las_pow;
               uchar CHH;
               if(inbuf[4]==0x0d)                                        //格式为P1(2)=N
              {
                 las_pow=(uint)(inbuf[3]-48);

              }
               else if(inbuf[5]==0x0d)                                   //格式为P1(2)=NN
              {
                 las_pow=(uint)((inbuf[3]-48)*10+(inbuf[4]-48));

              }
              else if(inbuf[6]==0x0d)                                    //格式为P1(2)=NNN
              {
                 las_pow=(uint)((inbuf[3]-48)*100+(inbuf[4]-48)*10+(inbuf[5]-48));

              }

              else                                                      //格式为P1(2)=NNNN
              {
                  setfault();
                  goto setpf;
              }

              if(inbuf[2]=='1')
              {
                CH=3;                                     //种子源SEED
                Set_laser_power(CHH,las_pow);
                setsucced();
              }
              else if(inbuf[2]=='2')
              {
                CH=0;                                    //放大级AMP
                Set_laser_power(CHH,las_pow);
                setsucced();
              }
              else setfault();

setpf:       _Nop();


            }
 ////////////////////////////////////////////////////////////////////////////////////////
 //////////////////////////////设定温度//////////////////////////////////////////////////
        else if((inbuf[0]=='T')&&((inbuf[1]=='1')||(inbuf[1]=='2'))&&(inbuf[2]=='='))
           {
              uint  tmp_tec=0;
              uint kt=0;
              if(inbuf[5]==0x0d)
              {
                 tmp_tec=(inbuf[3]-48)*10+(inbuf[4]-48);




                 kt=((tmp_tec-15)*691+180)/2000+(tmp_tec-15)*118+90;

              }
              else if(inbuf[7]==0x0d)
              {
                 tmp_tec=(inbuf[3]-48)*10+(inbuf[4]-48);

                 kt=((tmp_tec-15)*691+(inbuf[6]-48)*69+180)/2000+(tmp_tec-15)*118+(inbuf[6]-48)*59/5+90;

              }

              else
              {
                 setfault();
                 goto settf;
              }

               if(inbuf[1]=='1')
              {
                DA_Convert(2,kt);                         //种子源SEED
                _Nop();
                DA_Convert(2,kt);
                setsucced();
              }
              else if(inbuf[1]=='2')
              {
                DA_Convert(1,kt);                         //放大级AMP
                _Nop();
                DA_Convert(1,kt);
                setsucced();
              }
              else setfault();

settf:       _Nop();


           }

 ////////////////////////////////////////////////////////////////////////////////////////
 //////////////////////////////设定PWM频率//////////////////////////////////////////////////
        else if((inbuf[0]=='P')&&(inbuf[1]=='U')&&(inbuf[2]=='R')&&(inbuf[3]=='A')&&(inbuf[4]=='='))
         {
               
             
               if(inbuf[6]==0x0d)                                        //格式为PURA=N
              {
                 pwm_pulserate=inbuf[5]-48;


              }
               else if(inbuf[7]==0x0d)                                   //格式为PURA=NN
              {
                 pwm_pulserate=(uint)((inbuf[5]-48)*10+(inbuf[6]-48));

              }
              else if(inbuf[8]==0x0d)                                    //格式为PURA=NNN
              {
                 pwm_pulserate=(uint)((inbuf[5]-48)*100+(inbuf[6]-48)*10+(inbuf[7]-48));

              }
            
              else                                                    //格式为PURA=NNNN
              {
                  setfault();
                  goto setprf;
              }
                Pwm_Set(pwm_pulserate,0,0);
                setsucced();
setprf:       _Nop();

           }
          ////////////////////////////////////////////////////////////////////////////////////////
 //////////////////////////////设定PWM脉宽//////////////////////////////////////////////////
               else if((inbuf[0]=='P')&&(inbuf[1]=='U')&&(inbuf[2]=='W')&&(inbuf[3]=='I')&&(inbuf[4]=='='))
         {
              
             
               if(inbuf[6]==0x0d)                                        //格式为PUWI=N
              {
                 pwm_pulsewidth=inbuf[5]-48;


              }
               else if(inbuf[7]==0x0d)                                   //格式为PUWI=NN
              {
                 pwm_pulsewidth=(uint)((inbuf[5]-48)*10+(inbuf[6]-48));

              }
              else if(inbuf[8]==0x0d)                                    //格式为PUWI=NNN
              {
                 pwm_pulsewidth=(uint)((inbuf[5]-48)*100+(inbuf[6]-48)*10+(inbuf[7]-48));

              }
            
              else                                                    //格式为PUWI=NNNN
              {
                  setfault();
                  goto setpwf;
              }
                  Pwm_Set(0,pwm_pulsewidth,0);
                setsucced();
setpwf:       _Nop();

           }

           ////////////////////////////////////////////////////////////////////////////////////////
 //////////////////////////////设定PWM延迟时间//////////////////////////////////////////////////
	else if((inbuf[0]=='D')&&(inbuf[1]=='E')&&(inbuf[2]=='L')&&(inbuf[3]=='Y')&&(inbuf[4]=='='))
         {
             
             
               if(inbuf[6]==0x0d)                                        //格式为DELY=N
              {
                 pwm_pulsedelay=inbuf[5]-48;


              }
               else if(inbuf[7]==0x0d)                                   //格式为DELY=NN
              {
                 pwm_pulsedelay=(uint)((inbuf[5]-48)*10+(inbuf[6]-48));

              }
              else                                                    //格式为DELY=NNNN
              {
                  setfault();
                  goto setpdf;
              }
                Pwm_Set(0,0,pwm_pulsedelay);
                setsucced();
setpdf:       _Nop();

           }
////////////////////////////////////////////////////////////////////////////////////////
 //////////////////////////////选脉冲 开启或关闭  //////////////////////////////////////////////////
          else if((inbuf[0]=='R')&&(inbuf[1]=='F')&&(inbuf[2]=='='))

           {
              if((inbuf[4]==0x0d)&&(inbuf[3]=='1'))             //RF ON
              {
                 RFON=1;
                 P0=0xf0;
                 RW=0;
                 RS=0;
                 delayR(10);
                 ENAB=1;
                 delayR(10);
                 ENAB=0;
                 delayR(10);
                 setsucced();
              }
              else if((inbuf[4]==0x0d)&&(inbuf[3]=='0'))        //RF OFF
              {
                 RFON=0;
                 P0=0x0f;
                 RW=0;
                 RS=0;
                 delayR(10);
                 ENAB=1;
                 delayR(10);
                 ENAB=0;
                 delayR(10);
                 setsucced();
              }
              else
                 setfault();

           }
/////////////////////////////////关闭或开启TEC控制器////////////////////////////////
       else if((inbuf[0]=='S')&&(inbuf[1]=='H')&&(inbuf[2]=='D')&&(inbuf[3]=='N')&&(inbuf[4]=='='))

           {
              if((inbuf[5]=='S')&&(inbuf[7]==0x0d))             //种子源TEC
              {
                 SHDN1=(bit)((inbuf[6]-48)&0x01);
                 setsucced();
              }
              else if((inbuf[5]=='A')&&(inbuf[7]==0x0d))        //放大级TEC
              {
                 SHDN2=(bit)((inbuf[6]-48)&0x01);
                 setsucced();
              }
              else
                 setfault();

           }

 //////////////////////////////////强制关电流///////////////////////////////////////

      else if((inbuf[0]=='D')&&(inbuf[1]=='L')&&(inbuf[2]=='I')&&(inbuf[3]=='M')&&(inbuf[4]=='='))

           {
              if(inbuf[7]==0x0d)
              {
                 if(inbuf[5]=='S')
                   LIM1=(bit)((inbuf[6]-48)&0x01);
                 else  if(inbuf[5]=='A')
                   LIM2=(bit)((inbuf[6]-48)&0x01);
                 else
                  {
                   setfault();
                   goto setdf;
                  }
                 if (!LIM1)
                 {
                   Set_laser_current(3,50);
                   setsucced();
                  }
                 else if(!LIM2)
                 {
                   Set_laser_current(0,50);
                   setsucced();
                  }
                 else
                  {
                   setfault();
                   goto setdf;
                  }
              }
              else
                 setfault();
setdf:       _Nop();
           }
//////////////////////////////////总查询（电流、温度、功率）/////////////////////////////////
      else if((inbuf[0]=='S')&&(inbuf[1]=='T')&&((inbuf[2]=='A')&&(inbuf[3]=='T'))&&(inbuf[4]=='E')&&(inbuf[5]=='?'))
 {
              uint ky;
//              TEC_Temp();
              inbuf[0]= 0x0d;
              send_string_com(inbuf,1);
                inbuf[0] ='L';
                inbuf[1] ='D';
                inbuf[2] ='1';
                inbuf[3] ='=';
            if(Laser_Fed_curent1<10)
              { inbuf[4]=(uchar)(Laser_Fed_curent1)+48;
                inbuf[5]=0x0d;
                inbuf[6]=0x0a;
                send_string_com(inbuf,7);
              }else if (Laser_Fed_curent1<100)
              { 
                inbuf[4]=(uchar)(Laser_Fed_curent1 /10)+48;
                inbuf[5]=(uchar)(Laser_Fed_curent1 %10)+48;
                inbuf[6]=0x0d;
                inbuf[7]=0x0a;
                send_string_com(inbuf,8);
              }else if (Laser_Fed_curent1<1000)
              {
                inbuf[4]=(uchar)(Laser_Fed_curent1 /100)+48;
                inbuf[5]=(uchar)((Laser_Fed_curent1 /10)%10)+48;
                inbuf[6]=(uchar)(Laser_Fed_curent1 %10)+48;
                inbuf[7]=0x0d;
                inbuf[8]=0x0a;
                send_string_com(inbuf,9);
              }
              else
                setfault();

                inbuf[0] ='L';
                inbuf[1] ='D';
                inbuf[2] ='2';
                inbuf[3] ='=';
            if(Laser_Fed_curent2<10)
              {
                inbuf[4]=(uchar)(Laser_Fed_curent2)+48;
                inbuf[5]=0x0d;
                inbuf[6]=0x0a;
                send_string_com(inbuf,7);
              }else if (Laser_Fed_curent2<100)
              {
                inbuf[4]=(uchar)(Laser_Fed_curent2 /10)+48;
                inbuf[5]=(uchar)(Laser_Fed_curent2 %10)+48;
                inbuf[6]=0x0d;
                inbuf[7]=0x0a;
                send_string_com(inbuf,8);
              }else if (Laser_Fed_curent2<1000)
              {
                inbuf[4]=(uchar)(Laser_Fed_curent2 /100)+48;
                inbuf[5]=(uchar)((Laser_Fed_curent2 /10)%10)+48;
                inbuf[6]=(uchar)(Laser_Fed_curent2 %10)+48;
                inbuf[7]=0x0d;
                inbuf[8]=0x0a;
                send_string_com(inbuf,9);
              }
              else
                setfault();
              inbuf[0]= 0x0d;
              send_string_com(inbuf,1);
                inbuf[0] ='P';
                inbuf[1] ='1';
                inbuf[2] ='=';
          if(Laser_Fed_power1<10)
              {
                inbuf[3]=(uchar)(Laser_Fed_power1)+48;
                inbuf[4]=0x0d;
                inbuf[5]=0x0a;
                send_string_com(inbuf,6);
              }else if (Laser_Fed_power1<100)
              {
                inbuf[3]=(uchar)(Laser_Fed_power1 /10)+48;
                inbuf[4]=(uchar)(Laser_Fed_power1 %10)+48;
                inbuf[5]=0x0d;
                inbuf[6]=0x0a;
                send_string_com(inbuf,7);
              }else if (Laser_Fed_power1<1000)
              {
                inbuf[3]=(uchar)(Laser_Fed_power1 /100)+48;
                inbuf[4]=(uchar)((Laser_Fed_power1 /10)%10)+48;
                inbuf[5]=(uchar)(Laser_Fed_power1 %10)+48;
                inbuf[6]=0x0d;
                inbuf[7]=0x0a;
                send_string_com(inbuf,8);
              }
              else
                setfault();
                
                inbuf[0] ='P';
                inbuf[1] ='2';
                inbuf[2] ='=';
              if(Laser_Fed_power2<10)
              {
                inbuf[3]=(uchar)(Laser_Fed_power2)+48;
                inbuf[4]=0x0d;
                inbuf[5]=0x0a;
                send_string_com(inbuf,6);
              }else if (Laser_Fed_power2<100)
              {
                inbuf[3]=(uchar)(Laser_Fed_power2 /10)+48;
                inbuf[4]=(uchar)(Laser_Fed_power2 %10)+48;
                inbuf[5]=0x0d;
                inbuf[6]=0x0a;
                send_string_com(inbuf,7);
              }else if (Laser_Fed_power2<1000)
              {
                inbuf[3]=(uchar)(Laser_Fed_power2 /100)+48;
                inbuf[4]=(uchar)((Laser_Fed_power2 /10)%10)+48;
                inbuf[5]=(uchar)(Laser_Fed_power2 %10)+48;
                inbuf[6]=0x0d;
                inbuf[7]=0x0a;
                send_string_com(inbuf,8);
              }
              else
                setfault();

                inbuf[0]= 0x0d;
                send_string_com(inbuf,1);
                ky=(uint)(TEC_Fed_Tempture1);
                inbuf[0] ='T';
                inbuf[1] ='1';
                inbuf[2] ='=';
              if (ky<1000)
              {
                inbuf[3]  =ky/100+48;
                inbuf[4]  =(ky/10)%10+48;
                inbuf[5]  ='.';
                inbuf[6]  =ky%10+48;
                inbuf[7] =0x0d;
                inbuf[8] =0x0a;
                send_string_com(inbuf,9);
              }
              else
               alarm();
            
               
                ky=(uint)(TEC_Fed_Tempture2);
                inbuf[0] ='T';
                inbuf[1] ='2';
                inbuf[2] ='=';
              if (ky<1000)
              {
                inbuf[3]  =ky/100+48;
                inbuf[4]  =(ky/10)%10+48;
                inbuf[5]  ='.';
                inbuf[6]  =ky%10+48;
                inbuf[7] =0x0d;
                inbuf[8] =0x0a;
                send_string_com(inbuf,9);
              }
              else
               alarm();
               inbuf[0]= 0x0d;
              send_string_com(inbuf,1);
             if (TMPGD1)
              {
                inbuf[0]='T';
                inbuf[1]='1';
                inbuf[2]='O';
                inbuf[3]='K';
                inbuf[4]=0x0d;
                inbuf[5]=0x0a;
                send_string_com(inbuf,6);
              }

              else
              {
                inbuf[0]='T';
                inbuf[1]='1';
                inbuf[2]='N';
                inbuf[3]='O';
                inbuf[4]=0x0d;
                inbuf[5]=0x0a;
                send_string_com(inbuf,6);
              }

              if (TMPGD2)
              {
                inbuf[0]='T';
                inbuf[1]='2';
                inbuf[2]='O';
                inbuf[3]='K';
                inbuf[4]=0x0d;
                inbuf[5]=0x0a;
                send_string_com(inbuf,6);
              }

              else
              {
                inbuf[0]='T';
                inbuf[1]='2';
                inbuf[2]='N';
                inbuf[3]='O';
                inbuf[4]=0x0d;
                inbuf[5]=0x0a;
                send_string_com(inbuf,6);
              }

               inbuf[0] ='R';
                inbuf[1] ='A';
                inbuf[2] ='T';
                inbuf[3] ='E';
                inbuf[4] ='=';
                inbuf[5]  =pwm_pulserate/10+48;
                inbuf[6]  =pwm_pulserate%10+48;
                inbuf[7]  ='K';
                inbuf[8] =0x0d;
                inbuf[9] =0x0a;
                send_string_com(inbuf,10);
              
                inbuf[0] ='W';
                inbuf[1] ='I';
                inbuf[2] ='D';
                inbuf[3] ='T';
                inbuf[4] ='H';
                inbuf[5]  ='=';
                inbuf[6]  =pwm_pulsewidth/10+48;
                inbuf[7]  =pwm_pulsewidth%10+48;
                
                inbuf[8]  ='L';
                inbuf[9] =0x0d;
                inbuf[10] =0x0a;
                send_string_com(inbuf,11);
                
                inbuf[0] ='D';
                inbuf[1] ='E';
                inbuf[2] ='L';
                inbuf[3] ='A';
                inbuf[4] ='Y';
                inbuf[5]  ='=';
                inbuf[6]  =pwm_pulsedelay/10+48;
                inbuf[7]  =pwm_pulsedelay%10+48;
                
                inbuf[8]  ='L';
                inbuf[9] =0x0d;
                inbuf[10] =0x0a;
                send_string_com(inbuf,11);
                  inbuf[0] ='R';
                  inbuf[1] ='F';
                  inbuf[2] ='P';
                  inbuf[3] ='O';
                  inbuf[4] ='W';
                  inbuf[5] ='E';
                  inbuf[6] ='R';
                  inbuf[7] =' ';
                  if (RFON==1)
                  {
                  inbuf[8]  ='O';
                  inbuf[9] = 'N';
                  inbuf[10] = ' ';
                  }
                  else
                  {inbuf[8]  ='O';
                  inbuf[9]  = 'F';
                  inbuf[10] = 'F';
                  } 
                  inbuf[11] =0x0d;
                  inbuf[12] =0x0a;
                  send_string_com(inbuf,13);

}



 //////////////////////////////////查询当前实际电流/////////////////////////////////
      else if((inbuf[0]=='L')&&(inbuf[1]=='D')&&((inbuf[2]=='1')||(inbuf[2]=='2'))&&(inbuf[3]=='?'))
           {
            if(inbuf[2]=='1')
            {   inbuf[0] ='L';
                inbuf[1] ='D';
                inbuf[2] ='1';
                inbuf[3] ='=';
            if(Laser_Fed_curent1<10)
              { inbuf[4]=(uchar)(Laser_Fed_curent1)+48;
                inbuf[5]=0x0d;
                inbuf[6]=0x0a;
                send_string_com(inbuf,7);
              }else if (Laser_Fed_curent1<100)
              { 
                inbuf[4]=(uchar)(Laser_Fed_curent1 /10)+48;
                inbuf[5]=(uchar)(Laser_Fed_curent1 %10)+48;
                inbuf[6]=0x0d;
                inbuf[7]=0x0a;
                send_string_com(inbuf,8);
              }else if (Laser_Fed_curent1<1000)
              {
                inbuf[4]=(uchar)(Laser_Fed_curent1 /100)+48;
                inbuf[5]=(uchar)((Laser_Fed_curent1 /10)%10)+48;
                inbuf[6]=(uchar)(Laser_Fed_curent1 %10)+48;
                inbuf[7]=0x0d;
                inbuf[8]=0x0a;
                send_string_com(inbuf,9);
              }
              else
                setfault();

               }
 //////////////////////////////////////////////////////////
            else if(inbuf[2]=='2')
              {  inbuf[0] ='L';
                inbuf[1] ='D';
                inbuf[2] ='2';
                inbuf[3] ='=';
            if(Laser_Fed_curent2<10)
              {
                inbuf[4]=(uchar)(Laser_Fed_curent2)+48;
                inbuf[5]=0x0d;
                inbuf[6]=0x0a;
                send_string_com(inbuf,7);
              }else if (Laser_Fed_curent2<100)
              {
                inbuf[4]=(uchar)(Laser_Fed_curent2 /10)+48;
                inbuf[5]=(uchar)(Laser_Fed_curent2 %10)+48;
                inbuf[6]=0x0d;
                inbuf[7]=0x0a;
                send_string_com(inbuf,8);
              }else if (Laser_Fed_curent2<1000)
              {
                inbuf[4]=(uchar)(Laser_Fed_curent2 /100)+48;
                inbuf[5]=(uchar)((Laser_Fed_curent2 /10)%10)+48;
                inbuf[6]=(uchar)(Laser_Fed_curent2 %10)+48;
                inbuf[7]=0x0d;
                inbuf[8]=0x0a;
                send_string_com(inbuf,9);
              }
              else
                setfault();
           }
         }


 //////////////////////////////////查询当前实际功率/////////////////////////////////
      else if((inbuf[0]=='P')&&((inbuf[1]=='1')||(inbuf[1]=='2'))&&(inbuf[2]=='?'))

           {
            if(inbuf[1]=='1')
              {
               inbuf[0] ='P';
                inbuf[1] ='1';
                inbuf[2] ='=';
              if(Laser_Fed_power1<10)
              {
                inbuf[3]=(uchar)(Laser_Fed_power1)+48;
                inbuf[4]=0x0d;
                inbuf[5]=0x0a;
                send_string_com(inbuf,6);
              }else if (Laser_Fed_power1<100)
              {
                inbuf[3]=(uchar)(Laser_Fed_power1 /10)+48;
                inbuf[4]=(uchar)(Laser_Fed_power1 %10)+48;
                inbuf[5]=0x0d;
                inbuf[6]=0x0a;
                send_string_com(inbuf,7);
              }else if (Laser_Fed_power1<1000)
              {
                inbuf[3]=(uchar)(Laser_Fed_power1 /100)+48;
                inbuf[4]=(uchar)((Laser_Fed_power1 /10)%10)+48;
                inbuf[5]=(uchar)(Laser_Fed_power1 %10)+48;
                inbuf[6]=0x0d;
                inbuf[7]=0x0a;
                send_string_com(inbuf,8);
              }
              else
                setfault();
                
              }
              else if(inbuf[1]=='2')
               {
                inbuf[0] ='P';
                inbuf[1] ='2';
                inbuf[2] ='=';
              if(Laser_Fed_power2<10)
              {
                inbuf[3]=(uchar)(Laser_Fed_power2)+48;
                inbuf[4]=0x0d;
                inbuf[5]=0x0a;
                send_string_com(inbuf,6);
              }else if (Laser_Fed_power2<100)
              {
                inbuf[3]=(uchar)(Laser_Fed_power2 /10)+48;
                inbuf[4]=(uchar)(Laser_Fed_power2 %10)+48;
                inbuf[5]=0x0d;
                inbuf[6]=0x0a;
                send_string_com(inbuf,7);
              }else if (Laser_Fed_power2<1000)
              {
                inbuf[3]=(uchar)(Laser_Fed_power2 /100)+48;
                inbuf[4]=(uchar)((Laser_Fed_power2 /10)%10)+48;
                inbuf[5]=(uchar)(Laser_Fed_power2 %10)+48;
                inbuf[6]=0x0d;
                inbuf[7]=0x0a;
                send_string_com(inbuf,8);
              }
              else
                setfault();
            }
           }

///////////////////////////////////查询PWM情况////////////////////////////////
      else if((inbuf[0]=='P')&&((inbuf[1]=='W')&&(inbuf[2]=='M'))&&(inbuf[3]=='?'))
           {

                inbuf[0] ='R';
                inbuf[1] ='A';
                inbuf[2] ='T';
                inbuf[3] ='E';
                inbuf[4] ='=';
                inbuf[5]  =pwm_pulserate/10+48;
                inbuf[6]  =pwm_pulserate%10+48;
                inbuf[7]  ='K';
                inbuf[8] =0x0d;
                inbuf[9] =0x0a;
                send_string_com(inbuf,10);
              
               inbuf[0] ='W';
                inbuf[1] ='I';
                inbuf[2] ='D';
                inbuf[3] ='T';
                inbuf[4] ='H';
                inbuf[5]  ='=';
                inbuf[6]  =pwm_pulsewidth/10+48;
                inbuf[7]  =pwm_pulsewidth%10+48;
                
                inbuf[8]  ='L';
                inbuf[9] =0x0d;
                inbuf[10] =0x0a;
                send_string_com(inbuf,11);
                
                inbuf[0] ='D';
                inbuf[1] ='E';
                inbuf[2] ='L';
                inbuf[3] ='A';
                inbuf[4] ='Y';
                inbuf[5]  ='=';
                inbuf[6]  =pwm_pulsedelay/10+48;
                inbuf[7]  =pwm_pulsedelay%10+48;
                
                inbuf[8]  ='L';
                inbuf[9] =0x0d;
                inbuf[10] =0x0a;
                send_string_com(inbuf,11);
                inbuf[0] ='R';
                inbuf[1] ='F';
                inbuf[2] ='P';
                inbuf[3] ='O';
                inbuf[4] ='W';
                inbuf[5] ='E';
                inbuf[6] ='R';
                inbuf[7] =' ';
                if (RFON==1)
                {
                 inbuf[8]  ='O';
                 inbuf[9] = 'N';
                 inbuf[10] = ' ';
                }
                else
                  {inbuf[8]  ='O';
                  inbuf[9]  = 'F';
                  inbuf[10] = 'F';
                } 
                inbuf[11] =0x0d;
                inbuf[12] =0x0a;
                send_string_com(inbuf,13);


           }
         

///////////////////////////////////查询当前冷却温度////////////////////////////////
      else if((inbuf[0]=='T')&&((inbuf[1]=='1')||(inbuf[1]=='2'))&&(inbuf[2]=='?'))
           {

              uint ky;
//              TEC_Temp();
              if (inbuf[1]=='1')
              {  ky=(uint)(TEC_Fed_Tempture1);
                inbuf[0] ='T';
                inbuf[1] ='1';
                inbuf[2] ='=';
              if (ky<1000)
              {
                inbuf[3]  =ky/100+48;
                inbuf[4]  =(ky/10)%10+48;
                inbuf[5]  ='.';
                inbuf[6]  =ky%10+48;
                inbuf[7] =0x0d;
                inbuf[8] =0x0a;
                send_string_com(inbuf,9);
              }
              else
               alarm();  
              }
            else if (inbuf[1]=='2')
              { 
                ky=(uint)(TEC_Fed_Tempture2);
                inbuf[0] ='T';
                inbuf[1] ='2';
                inbuf[2] ='=';
              if (ky<1000)
              {
                inbuf[3]  =ky/100+48;
                inbuf[4]  =(ky/10)%10+48;
                inbuf[5]  ='.';
                inbuf[6]  =ky%10+48;
                inbuf[7] =0x0d;
                inbuf[8] =0x0a;
                send_string_com(inbuf,9);
              }
              else
               alarm();
           }
         }
///////////////////////////////////查询当前监测二极管反馈电压/////////////////////
      else if((inbuf[0]=='P')&&(inbuf[1]=='D')&&(inbuf[2]=='I')&&(inbuf[3]=='C')&&(inbuf[4]=='?'))

           {
              inbuf[6]=(uchar)(PDiode_V1/4095);
              inbuf[8]=(uchar)((PDiode_V1 %4095)*10/4095);
              inbuf[9]=(uchar)((((PDiode_V1 % 4095)*10)%4095)*10/4095);

              inbuf[4]='S';
              inbuf[5]='=';
              inbuf[6]=inbuf[6]+48;
              inbuf[7]='.';
              inbuf[8]=inbuf[8]+48;
              inbuf[9]=inbuf[9]+48;
              inbuf[10]=0x0d;
              inbuf[11]=0x0a;

              send_string_com(inbuf,12);

              inbuf[6]=(uchar)(PDiode_V2/4095);
              inbuf[8]=(uchar)((PDiode_V2 %4095)*10/4095);
              inbuf[9]=(uchar)((((PDiode_V2 % 4095)*10)%4095)*10/4095);

              inbuf[4]='A';
              inbuf[5]='=';
              inbuf[6]=inbuf[6]+48;
              inbuf[7]='.';
              inbuf[8]=inbuf[8]+48;
              inbuf[9]=inbuf[9]+48;
              inbuf[10]=0x0d;
              inbuf[11]=0x0a;

              send_string_com(inbuf,12);


           }

///////////////////////////////////查询当前TEC两端的电压//////////////////////////
      else if((inbuf[0]=='V')&&(inbuf[1]=='O')&&(inbuf[2]=='T')&&(inbuf[3]=='C')&&(inbuf[4]=='?'))

           {



              inbuf[6]=(uchar)(TEC_VO1 /1024);
              inbuf[8]=(uchar)((TEC_VO1 %1024)*10/1024);
              inbuf[9]=(uchar)((((TEC_VO1 %1024)*10)%1024)*10/1024);

              inbuf[4]='S';
              inbuf[5]='=';
              inbuf[6]=inbuf[6]+48;
              inbuf[7]='.';
              inbuf[8]=inbuf[8]+48;
              inbuf[9]=inbuf[9]+48;
              inbuf[10]=0x0d;
              inbuf[11]=0x0a;

              send_string_com(inbuf,12);

              inbuf[6]=(uchar)(TEC_VO2 /1024);
              inbuf[8]=(uchar)((TEC_VO2 %1024)*10/1024);
              inbuf[9]=(uchar)((((TEC_VO2 %1024)*10)%1024)*10/1024);

              inbuf[4]='A';
              inbuf[5]='=';
              inbuf[6]=inbuf[6]+48;
              inbuf[7]='.';
              inbuf[8]=inbuf[8]+48;
              inbuf[9]=inbuf[9]+48;
              inbuf[10]=0x0d;
              inbuf[11]=0x0a;

              send_string_com(inbuf,12);

           }
//////////////////////////////////查询当前TEC温度是否OK//////////////////////////
      else if((inbuf[0]=='T')&&(inbuf[1]=='P')&&(inbuf[2]=='G')&&(inbuf[3]=='D')&&(inbuf[4]=='?'))

           {
              if (TMPGD1)
              {
                inbuf[4]='S';
                inbuf[5]=' ';
                inbuf[6]='O';
                inbuf[7]='K';
                inbuf[8]=0x0d;
                inbuf[9]=0x0a;
                send_string_com(inbuf,10);
              }

              else
              {
                inbuf[4]='S';
                inbuf[5]=' ';
                inbuf[6]='N';
                inbuf[7]='O';
                inbuf[8]=0x0d;
                inbuf[9]=0x0a;
                send_string_com(inbuf,10);
              }

              if (TMPGD2)
              {
                inbuf[4]='A';
                inbuf[5]=' ';
                inbuf[6]='O';
                inbuf[7]='K';
                inbuf[8]=0x0d;
                inbuf[9]=0x0a;
                send_string_com(inbuf,10);
              }

              else
              {
                inbuf[4]='A';
                inbuf[5]=' ';
                inbuf[6]='N';
                inbuf[7]='O';
                inbuf[8]=0x0d;
                inbuf[9]=0x0a;
                send_string_com(inbuf,10);
              }

           }

///////////////////////////////////查询TEC控制器是否关闭//////////////////////////
      else if((inbuf[0]=='U')&&(inbuf[1]=='T')&&(inbuf[2]=='E')&&(inbuf[3]=='C')&&(inbuf[4]=='?'))

           {
              if (SHDN1)
              {
                inbuf[0]='T';
                inbuf[1]='E';
                inbuf[2]='C';
                inbuf[3]='1';
                inbuf[4]=' ';
                inbuf[5]='O';
                inbuf[6]='N';
                inbuf[7]=0x0d;
                inbuf[8]=0x0a;
                send_string_com(inbuf,9);
              }

              else
              {
                inbuf[0]='T';
                inbuf[1]='E';
                inbuf[2]='C';
                inbuf[3]='1';
                inbuf[4]=' ';
                inbuf[5]='O';
                inbuf[6]='F';
                inbuf[7]='F';
                inbuf[8]=0x0d;
                inbuf[9]=0x0a;
                send_string_com(inbuf,10);
              }

              if (SHDN2)
              {
                inbuf[0]='T';
                inbuf[1]='E';
                inbuf[2]='C';
                inbuf[3]='2';
                inbuf[4]=' ';
                inbuf[5]='O';
                inbuf[6]='N';
                inbuf[7]=0x0d;
                inbuf[8]=0x0a;
                send_string_com(inbuf,9);
              }

              else
              {
                inbuf[0]='T';
                inbuf[1]='E';
                inbuf[2]='C';
                inbuf[3]='2';
                inbuf[4]=' ';
                inbuf[5]='O';
                inbuf[6]='F';
                inbuf[7]='F';
                inbuf[8]=0x0d;
                inbuf[9]=0x0a;
                send_string_com(inbuf,10);
              }
           }

///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////控制模式查询////////////////////////////////////
       else if((inbuf[0]=='C')&&(inbuf[1]=='M')&&(inbuf[2]=='O')&&(inbuf[3]=='D')&&(inbuf[4]=='?'))

           {
              if (CONT_MOD)
              {
                inbuf[0]='M';
                inbuf[1]='O';
                inbuf[2]='D';
                inbuf[3]='=';
                inbuf[4]='A';
                inbuf[5]='P';
                inbuf[6]='C';
                inbuf[7]=0x0d;
                inbuf[8]=0x0a;
                send_string_com(inbuf,9);
              }

              else
              {
                inbuf[0]='M';
                inbuf[1]='O';
                inbuf[2]='D';
                inbuf[3]='=';
                inbuf[4]='A';
                inbuf[5]='C';
                inbuf[6]='C';
                inbuf[7]=0x0d;
                inbuf[8]=0x0a;
                send_string_com(inbuf,9);
              }
           }


/////////////////////////////////////系统工作模式切换////////////////////////////////////////////
      else if((inbuf[0]=='S')&&(inbuf[1]=='M')&&(inbuf[2]=='O')&&(inbuf[3]=='D')&&(inbuf[4]=='='))
      {
           if(inbuf[6]==0x0d)
              {
                 Sys_mod=(bit)((inbuf[5]-48)&0x01);        //1:主动模式，0：系统采集模式
                 setsucced();
              }
           else
              setfault();
      }
////////////////////////////////////////////////////////////////////////////////////////////////

      else
       setfault();                              //设置错误



     return;

}
/*******************************************************************************/





/*****************************设置显示****************************************
void Lcd_disp()
{
 wr_lcd (comm,0x30);                    //设定工作方式
 delay(1);
 wr_lcd (comm,0x01);                    //清屏
 delay(1);
 wr_lcd (comm,0x0d);                    //开显示
 delay(1);
 wr_lcd (comm,0x10);
 delay(30);

 if(CONT_MOD)                          //CONT_MOD: 1 APC, 0 ACC

//*****************APC模式显示设定功率*********************************
 {
   chn_disp (tab13,8,0x80);
   delayR(50);
   chn_disp (Power_parameter1,6,0x84);
 }

//********************************************************************
 else
//****************ACC模式显示电流设定**********************************
 {
  chn_disp (tab9,8,0x80);
  delayR(50);
  chn_disp (Diode_parameter1,6,0x84);

 }
//*********************************************************************

 delayR(50);

//*****************显示设定温度****************************************

 delayR(50);
 chn_disp (TEC_parameter1,4,0x94);
 delayR(50);
 chn_disp (tab11,2,0x96);
 delayR(50);
// wr_lcd (comm,0x90);
//********************************************************************

}
/**************************************************************************


/*****************************判断按键电压*********************************
void Key_Button()
{
  uchar kk=10;
  uchar tec_a=0;
  uchar tec_b=0;
  Key_V=0;




  return;
}

/**************************************************************************/

/**********************二极管电流监测**************************************/
void Diode_curent_Check()
{
    uchar k=16;
//    float a,b;
    AD_inbuf[0]=0;
    AD_inbuf[1]=0;
    do
    {
      AD_code=AD_Convert(0);                  //种子源电流
      AD_inbuf[0]= AD_code+AD_inbuf[0];

      AD_code=AD_Convert(3);                  //放大级电流
      AD_inbuf[1]= AD_code+AD_inbuf[1];

      k--;
    }while(k);


    AD_inbuf[0]=AD_inbuf[0]/16;
    AD_inbuf[1]=AD_inbuf[1]/16;

    Laser_Fed_curent1=(AD_inbuf[0]*5/6-AD_inbuf[0]*15/768)/2;     //R=1.5欧
    Laser_meds_curent1=Laser_Fed_curent1;
    if (Laser_Fed_curent1<10)
      { Diode_parameter1[0]=Laser_Fed_curent1+48;
        Diode_parameter1[1]=0x6d;
        Diode_parameter1[2]=0x41;
        Diode_parameter1[3]=0x20;
        Diode_parameter1[4]=0x20;

      } else if(Laser_Fed_curent1<100)

      { Diode_parameter1[0]=Laser_Fed_curent1 /10+48;
        Diode_parameter1[1]=Laser_Fed_curent1 %10+48;
        Diode_parameter1[2]=0x6d;
        Diode_parameter1[3]=0x41;
        Diode_parameter1[4]=0x20;
      } else if(Laser_Fed_curent1<1000)

      { Diode_parameter1[0]=Laser_Fed_curent1 /100+48;
        Diode_parameter1[1]=(Laser_Fed_curent1 %100)/10+48;
        Diode_parameter1[2]=Laser_Fed_curent1 %10+48;
        Diode_parameter1[3]=0x6d;
        Diode_parameter1[4]=0x41;
      }

    Laser_Fed_curent2=(AD_inbuf[1]*5/6-AD_inbuf[1]*15/768)/2;     //R=1.5欧
    Laser_meds_curent2=Laser_Fed_curent2;
    if (Laser_Fed_curent2<10)
      { Diode_parameter2[0]=Laser_Fed_curent2+48;
        Diode_parameter2[1]=0x6d;
        Diode_parameter2[2]=0x41;
        Diode_parameter2[3]=0x20;
        Diode_parameter2[4]=0x20;

      } else if(Laser_Fed_curent2<100)

      { Diode_parameter2[0]=Laser_Fed_curent2 /10+48;
        Diode_parameter2[1]=Laser_Fed_curent2 %10+48;
        Diode_parameter2[2]=0x6d;
        Diode_parameter2[3]=0x41;
        Diode_parameter2[4]=0x20;
      } else if(Laser_Fed_curent2<1000)

      { Diode_parameter2[0]=Laser_Fed_curent2 /100+48;
        Diode_parameter2[1]=(Laser_Fed_curent2 %100)/10+48;
        Diode_parameter2[2]=Laser_Fed_curent2 %10+48;
        Diode_parameter2[3]=0x6d;
        Diode_parameter2[4]=0x41;
      }

    return;
}
/**************************************************************************/
/*****************************激光功率检测*********************************/
void Laser_power_Check()
{

    uchar k=10;
    float a,b;
    AD_inbuf[8]=0;
    AD_inbuf[9]=0;
    do
    {
      AD_code=AD_Convert(1);                  //种子源
      AD_inbuf[8]= AD_code+AD_inbuf[8];

      AD_code=AD_Convert(4);                  //放大级
      AD_inbuf[9]= AD_code+AD_inbuf[9];
      k--;
    }while(k);



    AD_inbuf[8]=AD_inbuf[8]/10;
    AD_inbuf[9]=AD_inbuf[9]/10;

    AD_code=AD_inbuf[8];
    a=(float)(AD_code);
    a=a*2.5;
    a=a/4096+0.02015;
    b=a*a*117.314+275.543*a-0.806;

    Laser_Fed_power1=(uint)(b);
    Laser_meds_power1=Laser_Fed_power1;

    if (Laser_Fed_power1<10)
      { Power_parameter1[0]=Laser_Fed_power1+48;
        Power_parameter1[1]=0x6d;
        Power_parameter1[2]=0x77;
        Power_parameter1[3]=0x20;
        Power_parameter1[4]=0x20;
        Power_parameter1[5]=0x20;

      } else if(Laser_Fed_power1<100)

      { Power_parameter1[0]=Laser_Fed_power1/10+48;
        Power_parameter1[1]=Laser_Fed_power1 %10+48;
        Power_parameter1[2]=0x6d;
        Power_parameter1[3]=0x77;
        Power_parameter1[4]=0x20;
        Power_parameter1[5]=0x20;
      } else if(Laser_Fed_power1<1000)

      { Power_parameter1[0]=Laser_Fed_power1 /100+48;
        Power_parameter1[1]=(Laser_Fed_power1 %100)/10+48;
        Power_parameter1[2]=Laser_Fed_power1 %10+48;
        Power_parameter1[3]=0x6d;
        Power_parameter1[4]=0x77;
        Power_parameter1[5]=0x20;
      }

    AD_code=AD_inbuf[9];
    a=(float)(AD_code);
    a=a*2.5;
    a=a/4096+0.02015;
    b=a*a*117.314+275.543*a-0.806;

    Laser_Fed_power2=(uint)(b);
    Laser_meds_power2=Laser_Fed_power2;

    if (Laser_Fed_power2<10)
      { Power_parameter2[0]=Laser_Fed_power2+48;
        Power_parameter2[1]=0x6d;
        Power_parameter2[2]=0x77;
        Power_parameter2[3]=0x20;
        Power_parameter2[4]=0x20;
        Power_parameter2[5]=0x20;

      } else if(Laser_Fed_power2<100)

      { Power_parameter2[0]=Laser_Fed_power2/10+48;
        Power_parameter2[1]=Laser_Fed_power2 %10+48;
        Power_parameter2[2]=0x6d;
        Power_parameter2[3]=0x77;
        Power_parameter2[4]=0x20;
        Power_parameter2[5]=0x20;
      } else if(Laser_Fed_power2<1000)

      { Power_parameter2[0]=Laser_Fed_power2 /100+48;
        Power_parameter2[1]=(Laser_Fed_power2 %100)/10+48;
        Power_parameter2[2]=Laser_Fed_power2 %10+48;
        Power_parameter2[3]=0x6d;
        Power_parameter2[4]=0x77;
        Power_parameter2[5]=0x20;
      }

    return;



}
/**************************************************************************/


/*****************************当前冷却温度*********************************/
void TEC_Temp()
{
    uint  tx1=0;
    uint  tx2=0;
    uchar kt=16;
    AD_inbuf[6]=0;
    AD_inbuf[7]=0;
    do
    {
      AD_code=AD_Convert(2);        //种子源TEC温度
      AD_inbuf[6]= AD_code+AD_inbuf[6];

      AD_code=AD_Convert(5);        //放大级TEC温度
      AD_inbuf[7]= AD_code+AD_inbuf[7];
      kt--;
    }while(kt);

     AD_inbuf[6]=AD_inbuf[6]/16;
     AD_inbuf[7]=AD_inbuf[7]/16;
     TEC_meds_tempture1=AD_inbuf[6];
     TEC_meds_tempture2=AD_inbuf[7];

     TEC_Fed_Tempture1=(float)( TEC_meds_tempture1)/23.6749+142.4;
     TEC_Fed_Tempture2=(float)( TEC_meds_tempture2)/23.6749+142.4;
     tx1= (uint)(TEC_Fed_Tempture1);
     TEC_meds_tempture1=tx1;
     tx2= (uint)(TEC_Fed_Tempture2);
     TEC_meds_tempture2=tx2;

     TEC_parameter1[0]=tx1 /100+48;
     TEC_parameter1[1]=(tx1 /10)%10+48;
     TEC_parameter1[2]=0x2e;
     TEC_parameter1[3]=tx1 %10+48;


     TEC_parameter2[0]=tx2 /100+48;
     TEC_parameter2[1]=(tx2 /10)%10+48;
     TEC_parameter2[2]=0x2e;
     TEC_parameter2[3]=tx2 %10+48;


  return;
}

/**************************************************************************/

/*****************************当前TEC两端电压******************************/
void TEC_V_Check()                                                 //???????????????????????
{
    uchar kk=20;
    AD_inbuf[4]=0;
    AD_inbuf[5]=0;
    do
    {
      MCU_ADC(1);
      AD_inbuf[4]= MCU_AD_code+ AD_inbuf[4];
      MCU_ADC(2);
      AD_inbuf[5]= MCU_AD_code+ AD_inbuf[5];
      kk--;
    }while(kk);


     TEC_VO1=AD_inbuf[4]/20 ;
     TEC_VO1=TEC_VO1*5;
     TEC_VO2=AD_inbuf[4]/20 ;
     TEC_VO2=TEC_VO2*5;

//     TEC_parameter[0]=(uchar)(TEC_VO/1024)+48;
//     TEC_parameter[1]=0x2e;
//     TEC_parameter[2]=(uchar)((TEC_VO%1024)*10/1024)+48;
//     TEC_parameter[3]=(uchar)((((TEC_VO%1024)*10)%1024)*10/1024)+48;
//     TEC_parameter[4]=0xa0;
//     TEC_parameter[5]=0x56;
    return;
}

/**************************************************************************/

/*****************************监测二极管电压******************************/
void Pdiod_V_Check()
{
    uchar kk=10;
    AD_inbuf[2]=0;
    AD_inbuf[3]=0;
    do
    {
      AD_code=AD_Convert(1);             //种子源激光二极管
      AD_inbuf[2]= AD_code+AD_inbuf[2];

      AD_code=AD_Convert(4);             //放大级激光二极管
      AD_inbuf[3]= AD_code+AD_inbuf[3];
      kk--;
    }while(kk);

     PDiode_V1= AD_inbuf[2]/10;
     PDiode_V2= AD_inbuf[3]/10;
     PDiode_V1=(PDiode_V1*5)/2;
     PDiode_V2=(PDiode_V2*5)/2;
    return;
}
/**************************************************************************/



/*******************************设定温度值*************************************

void Set_TEC_TEMP(uint tempture)
{
     uint kt;

      kt=(uint)((tempture-15)*4096/35);
      DA_Convert(1,kt);




   return;
}


/******************************************************************************/

/*****************************设定功率值**********************************/
void Set_laser_power(uchar Channel,uint power)
{
  float  u,v;
  uint   temp_p;

  if (power<3) power=3;
  if (power>600) power=600;
  u=(float)(power);
  v=(u*u/1000)*3.46;
  v=6.5068*u+29.61-v;
  temp_p=(uint)(v);


  if(Channel==3)                                         //设定种子源功率
  {

    if(SHDN1)
     {

          DA_Convert(3,temp_p);
          _Nop();
          DA_Convert(3,temp_p);
     }

     Laser_Set_power1=power;
   }


    else if(Channel==0)
  {

    if(SHDN2)                                             //设定放大级功率
     {

          DA_Convert(0,temp_p);
          _Nop();
          DA_Convert(0,temp_p);
     }

     Laser_Set_power2=power;
   }

   else  _Nop();

   Laser_power_Check();


    return;

}


/*************************************************************************/

/*****************************设定电流值**********************************/

void Set_laser_current(uchar Channel,uint current)
{

    uint tmp_c1;

    if (current<30) current=30;
    if (current>900) current=900;



      tmp_c1=current*3+current*9/125;                     //R=1.5欧





    if(Channel==3)
    {
       if(SHDN1)
       {

            DA_Convert(3,tmp_c1);
            _Nop();
            DA_Convert(3,tmp_c1);

            Laser_Set_curent1=current;
       }

       _Nop();
    }

    else if(Channel==0)
    {
       if(SHDN2)
       {

            DA_Convert(0,tmp_c1);
            _Nop();
            DA_Convert(0,tmp_c1);

            Laser_Set_curent2=current;
       }

       _Nop();
    }




/*
  if (tmp_c1>tmp_c0)
    {
      do
      {

        while(!TMPGD)
        {
          TEC_Temp();
          if(tx>280)
        {
          uchar inbuf1[11]={'t','m','p','_','e','r','r',0x0d,0x0a};
          send_string_com(inbuf1,9);
          t=0;
          SHDN=0;
          tmp_c0=Laser_Set_curent*8+Laser_Set_curent*24/125;
          tmp_c1=246;
          Laser_Set_curent=30;
           do
         {
          tmp_c0=tmp_c0-2;
          DA_Convert(0,tmp_c0);
          m++;

           if(m==240)
           {
            Diode_curent_Check();
            TEC_Temp();
            chn_disp (tab6,6,0x80);
            delayR(500);
            chn_disp (Diode_parameter,6,0x83);
            delayR(500);
            chn_disp (tab7,4,0x90);
            delayR(500);
            chn_disp (TEC_parameter,4,0x92);
            delayR(100);
            chn_disp (tab11,2,0x94);
            m=0;
           }
           delayR(10000);
           }while(tmp_c0-tmp_c1);
           chn_disp (inbuf1,7,0x96);
           goto quit;
         }
        }


        


     if(m==160)
    {
     TEC_Temp();
     Diode_curent_Check();
     chn_disp (tab6,6,0x80);
     delayR(500);
     chn_disp (Diode_parameter,6,0x83);
     delayR(500);
     chn_disp (tab7,4,0x90);
     delayR(500);
     chn_disp (TEC_parameter,4,0x92);
     delayR(100);
     chn_disp (tab11,2,0x94);
     m=0;
    }







       delayR(10000);
      }while((tmp_c1-tmp_c0)&& SHDN);

       delayR(180);

    }
  else if(tmp_c1<tmp_c0)

    {
       do
      {
        
     if(m==160)
     {
     Diode_curent_Check();
     TEC_Temp();
     chn_disp (tab6,6,0x80);
     delayR(500);
     chn_disp (Diode_parameter,6,0x83);
     delayR(500);
     chn_disp (tab7,4,0x90);
     delayR(500);
     chn_disp (TEC_parameter,4,0x92);
     delayR(100);
     chn_disp (tab11,2,0x94);
     m=0;
     }
     delayR(10000);
      }while(tmp_c0-tmp_c1);

     delay(100);

    }        */
    Diode_curent_Check();


    return;

}

/***************************************************************************/

/*****************************外部AD转换程序*******************************/
uint AD_Convert(uchar Channel)
{
   uchar i,Dath,Datl,temp,Contbit;
   uint Dat_ad;
   Dath=0;
   Datl=0;
   temp=0;
   Dat_ad=0;
   switch (Channel)                        //  内部REF
    {
        case 0:Contbit=0x00;break;
        case 1:Contbit=0x08;break;
        case 2:Contbit=0x10;break;
        case 3:Contbit=0x18;break;
        case 4:Contbit=0x20;break;
        case 5:Contbit=0x28;break;
        case 6:Contbit=0x30;break;
        case 7:Contbit=0x38;break;
        default: break;
    }

    CSAD=0;
    _Nop();
    DCLK=1;

   for(i=0;i<8;i++)
   {
      ADIN=(bit)(Contbit&(0x80>>i));   //串行数据位

      DCLK=0;
      _Nop();
      DCLK=1;
      _Nop();

   }

    CSAD=1;
    _Nop();

    CSAD=0;
    _Nop();

    for(i=0;i<8;i++)                   //写入控制字，并读取高四位数据
    {
      ADIN=(bit)(Contbit&(0x80>>i));

      DCLK=0;
      _Nop();
      DCLK=1;
      _Nop();
      Dath=DOUT;
      Dath=Dath<<(7-i);
      temp=temp|Dath;
    }

     Dath=temp;
     temp=0;
    for(i=0;i<8;i++)
    {
      DCLK=0;
      _Nop();
      DCLK=1;
      Datl=DOUT;
      Datl=Datl<<(7-i);
      temp=temp|Datl;
      _Nop();

    }

     Datl=temp;
     Dat_ad=(Dath*256+Datl)&0x0fff;

     CSAD=1;



      _Nop();

      return(Dat_ad);
}



/**************************************************************************/
/*****************************外部PWM设置程序*******************************/
void Pwm_Set(uint pwm_pulserate, uint pwm_pulsewidth,uint pwm_pulsedelay)
{ 
  uchar pulserate,pulsewidth,pulsedelay;

  if (pwm_pulserate>0)
  {
    //pulserate=1000000/pwm_pulserate/12.5/32;             //80M晶振
    //pulserate=1000000/pwm_pulserate/8/32;             //125M晶振
    pulserate=1000000/pwm_pulserate/5/32;             //200M晶振
    if(pulserate>255)
      pulserate=255;
    P0=pulserate;
    RW=1;

    RS=0;
    delayR(10);
    ENAB=1;
    delayR(10);
    ENAB=0;
    delayR(10);
    RW=0;
    RS=0;
  }
  
  if (pwm_pulsewidth>0)
  {
    pulsewidth=pwm_pulsewidth   ;      //80M晶振
    if (pulsewidth>15)
      pulsewidth=15;
    P0=pulsewidth;
    RW=0;
    RS=1;
    delayR(10);
    ENAB=1;
    delayR(10);
    ENAB=0;
    delayR(10);
    RW=0;
    RS=0;
  }
  
  if (pwm_pulsedelay>0)
  {
    pulsedelay= pwm_pulsedelay ;                 //80M晶振
    if (pulsedelay>31)
      pulsedelay=31;
    P0=pulsedelay;
    RW=1;
    RS=1;
    delayR(10);
    ENAB=1;
    delayR(10);
    ENAB=0;
    delayR(10);
    RW=0;
    RS=0;
    


  }



}
/**************************************************************************/
/*****************************外部DA转换程序*******************************/
void DA_Convert(uchar Channel, uint Dcode)
{

  uint  Comdat=0;
   uchar i;
   switch (Channel)
    {
        case 0:Comdat=Dcode+0x1000;break;
        case 1:Comdat=Dcode+0x5000;break;
        case 2:Comdat=Dcode+0x9000;break;
        case 3:Comdat=Dcode+0xd000;break;
        default: break;
    }

    LDAC=1;                               //数据刷新
    _Nop();
    CSDA=0;                               //片选有效
    FS=1;
    _Nop();
    FS=0;
   for(i=0;i<16;i++)
  {
       DINDA=(bit)(Comdat&0x8000);
       SCLK=1;
       Comdat=Comdat<<1;
       _Nop();
       SCLK=0;

   }

   LDAC=0;
   _Nop();
   CSDA=1;                             //片选无效
   Dcode=0;
  return;

}
/**************************************************************************/


/*****************************单片机内部AD********************************/
 void MCU_ADC(BYTE CHN)
 {
    BYTE    AD_finished;
    switch (CHN)
    {
        case 0: P1ASF=0x01;break;
        case 1: P1ASF=0x02;break;
        case 2:P1ASF=0x04;break;
        case 3:P1ASF=0x08;break;
        case 7:P1ASF=0x80;break;

        default: break;
    }
    AUXR1 &= ~0x04;                     //0000,0100, 令 ADRJ=0
    ADC_RES=0;
    ADC_RESL=0;

    CHN &= 0x07;                        //0000,0111 清0高5位

    ADC_CONTR =ADC_SPEEDH ;
    _nop_();
    ADC_CONTR |= CHN ;                  //选择 A/D 当前通道
    _nop_();
    ADC_CONTR |= 0x80;                  //启动 A/D 电源
    delayR(10);
    ADC_CONTR |= 0x08;
    delayR(10);
    AD_finished = 0;
    while (AD_finished ==0 )            //等待A/D转换结束
    {
      AD_finished = (ADC_CONTR & 0x10);
    }
    ADC_CONTR &= 0xE7;

    P1ASF=0x00;
    MCU_AD_code=ADC_RES*4+ADC_RESL;

    return;
 }

/*************************************************************************/



void Error_Process()
{

 _Nop();
 return;

}



/*********以下是长延时程序***********************************************/

void delay(uint delay_k)
{
    uint k0=0;
    do
    {
    do{k0--;}while(k0);
    delay_k--;
    }while(delay_k);

}

/*********以下是短延时程序***********************************************/
void delayR(uint delay_k)
{
    do{delay_k--;}while(delay_k);


}

/*********************************************************************/

/**************************以下是外部中断0程序************************/
 void INT_0() interrupt 0 using 0
 {
    if (!LIM1)
    Error_Process();
    else  _Nop();

    if (!LIM2)
    Error_Process();
    else  _Nop();
    return;
 }
/*********************************************************************/

/***************************以下是外部中断1程序***********************/
void INT_1() interrupt 2 using 1
{
  _Nop();
  //chn_disp (tab16,20,0x80);
  //delayR(10000);
  //chn_disp (tab17,20,0x90);
  //delay(1000);
}
/***********************************************************************/





/*****************以下是串口中断程序*******************************/

void RS232() interrupt 4 using 3
{

   if(RI)
   {
        RI=0;

        if (SBUF==0x0A)
        {
             read_flag=1;

          }

        else
        {


             inbuf[t]=SBUF;
             t++;



        }

   }

     return;
}

/**********************************************************************/

/*****************定时器中断程序*******************************/

void tm0_isr() interrupt 1 using 1
{		
     TL0 = 0x30;   //设置定时初值
     TH0 = 0xF8;   //设置定时初值
     if (count-- == 0)               //1ms * 1000 -> 1s
     {
     count = 1000;               //reset counter
     }
}

/*************************设置错误***************************/
void setfault()
{

  uchar inbuf1[10]={'s','e','t','f','a','u','l','t',0x0d,0x0a};

  send_string_com(inbuf1,10);

  t=0;
}
/*************************************************************/


/*************************设置成功***************************/
void setsucced()
{

  uchar inbuf1[11]={'s','e','t','s','u','c','c','e','d',0x0d,0x0a};

  send_string_com(inbuf1,11);
  t=0;
  return;
}
/*************************************************************/






/********************************警告*************************/
void alarm()
{
   uchar t=6;
   uchar inbuf1[7]={'A','l','a','r','m',0x0d,0x0a};

   send_string_com(inbuf1,7);
  do
  {
   delay(50);
   t--;
  }while(t);
   send_string_com(inbuf1,5);
   t=6;
   do
  {
   delay(50);
   t--;
  }while(t);
   send_string_com(inbuf1,5);

}
/*************************************************************/



/************************主程序*******************************/
void main()
{
   uchar  i=10;
   uchar  tmp0=0;
// setsucced();
// delay(5000);
// setsucced();
// delay(5000);
// setsucced();
   stat_initial();                               //状态初始划
   serial_initial();                             //串口初始化
// delay(20000);
// setsucced();
// delay(5000);
// setsucced();
// delay(5000);
// setsucced();
  if(CONT_MOD)                                  //CONT_MOD: 1 APC, 0 ACC

///////////////////////////////////APC模式设定初始功率//////////////////////////////////////////
 {
  Laser_Set_power1=3;                        //记录种子源当前设定功率值
  Laser_Set_power2=3;                        //记录放大级当前设定功率值

  DA_Convert(3,25);                          // 种子源
   _Nop();
  DA_Convert(3,25);

  DA_Convert(0,25);                          // 放大级
   _Nop();
  DA_Convert(0,25);
 }
//////////////////////////////////////////////////////////////////////////////////////////////

  else
//////////////////////////////////ACC模式设定初始电流//////////////////////////////////////////
 {
  Laser_Set_curent1=30;                          //记录种子源当前设定电流值
  Laser_Set_curent2=30;                          //记录放大级当前设定电流值

  DA_Convert(3,88);                            // 种子源
   _Nop();
  DA_Convert(3,88);

  DA_Convert(0,88);                            // 放大级
  _Nop();
  DA_Convert(0,88);
 }
//////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////上电开始显示//////////////////////////////////////////////tang
 /*  init_lcd ();
   delay(5);
   wr_lcd (comm,0x30);
   delay(30);                                    //需要足够的等待延时
   chn_disp (tab1,14,0x80);
   delay(5);
   chn_disp (tab2,20,0x90);
   delay(5);
   clrram();
   delay(5);
   chn_disp (tab3,10,0x80);
   chn_disp (tab4,12,0x90);

   clrram();
   delay(10);
   chn_disp (tab5,10,0x80);
   delay(5);
*/////////////////////////////////////////////////////////////////////////////////////////////



  do
   {

     delay(1);

     if(read_flag)
      {
        command();                        //指令识别
        read_flag=0;                      //取数标志清0
        t=0;
      }

     _Nop();


    if(Sys_mod)
 {


/***************************检查按键**************************************/

 // Key_Button();  tang


///////////////////////////////////////////////////////////////////////////

/***************************检查当前实际功率值****************************/
 //Laser_power_Check(); tang


/*************************************************************************/
/***************************检测当前实际电流值****************************/

 Diode_curent_Check();




///////////////////////////////////////////////////////////////////////////


/***************************检测当前冷却温度******************************/
   TEC_Temp();



   if(TEC_meds_tempture1>350)

   {

     uchar inbuf1[9]={'t','m','p','_','e','r','r',0x0d,0x0a};
     if(CONT_MOD)
     {
     Set_laser_power(0,3);       //tang
     Set_laser_power(3,3);
     }
     else
     {
     Set_laser_current(0,30);
     Set_laser_current(3,30);
     }
     SHDN1=0;
     SHDN2=0;
     send_string_com(inbuf1,9);
     t=0;
     }

   if(TEC_meds_tempture2>350)

   {

     uchar inbuf1[9]={'t','m','p','_','e','r','r',0x0d,0x0a};
     if(CONT_MOD)
     {
     Set_laser_power(0,3);       //tang
     Set_laser_power(3,3);
     }
     else
     {
     Set_laser_current(0,30);
     Set_laser_current(3,30);
     }
     SHDN1=0;
     
     SHDN2=0;
     send_string_com(inbuf1,9);
     t=0;
   }
//////////////////////////////////////////////////////////////////////////


/***************************检测监测二极管反馈电压************************/

 Pdiod_V_Check();



//////////////////////////////////////////////////////////////////////////


/***************************检测TEC温控是否完成**************************/





/////////////////////////////////////////////////////////////////////////


/***************************检测TEC两端电压是否正常*********************/

  TEC_V_Check();

  if( TEC_VO1>3.1) Error_Process();
  if( TEC_VO2>3.1) Error_Process();

////////////////////////////////////////////////////////////////////////


/****************************LCD显示***********************************


     chn_disp (tab6,4,0x80);
     delayR(50);
     chn_disp (Diode_parameter1,6,0x82);      //显示种子源电流
     delayR(50);
     chn_disp (TEC_parameter1,4,0x85);        //显示种子源TEC温度


     chn_disp (tab7,4,0x90);
     delayR(50);
     chn_disp (Diode_parameter2,6,0x92);      //显示放大级电流
     delayR(50);
     chn_disp (TEC_parameter2,4,0x95);        //显示放大级TEC温度





     if(CONT_MOD)
     {
        chn_disp (tab14,6,0x87);
        delayR(50);
        chn_disp (tab14,6,0x97);
      }
     else
     {
        chn_disp (tab15,6,0x87);
        delayR(50);
        chn_disp (tab15,6,0x97);
     }

//////////////////////////////////////////////////////////////////////*/
}

else
{
    // chn_disp (tab16,20,0x80);
   // delayR(10000);
    // chn_disp (tab17,20,0x90);
}

    }while(1);


 }
