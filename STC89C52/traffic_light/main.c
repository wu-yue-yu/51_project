/*
 * @Descripttion: 实现交通信号灯切换效果，并在倒数3秒时利用PWM实现闪烁效果
 * @Author: 五月雨
 * @Date: 2022-02-01 08:23:16
 * @LastEditors: 五月雨
 * @LastEditTime: 2022-02-04 23:04:26
 * @Board: 清翔51开发板
 * @Chip: STC89C52
 */
#include <STC89C5xRC.H>

#define PWMOUT P1                   //定义P1口为pwm输出口

unsigned char code TrafficLightColor[3] = {0xFE, 0xFD, 0xFB};
unsigned long PeriodCnt = 0;        //PWM周期计数值
unsigned char HighRH = 0;           //高电平重载值的高字节
unsigned char HighRL = 0;           //高电平重载值的低字节
unsigned char LowRH = 0;            //低电平重载值的高字节
unsigned char LowRL = 0;            //低电平重载值的低字节
unsigned char T1RH = 0;             //T1重载值的高字节
unsigned char T1RL = 0;             //T1重载值的低字节
unsigned char timer = 9;            //倒计时计数变量，先让红灯亮10秒
unsigned char color = 0;            //交通信号灯颜色指示变量，也相当于颜色数组索引
bit flag_1s = 0;                    //1秒定时标志变量

void ConfigTimer1(unsigned int ms);
void ConfigPWM(unsigned int fr, unsigned char dc);
void AdjusDutyCycle(unsigned char dc);
void TrafficLight();

void main()
{
    EA = 1;                         //开总中断

    ConfigPWM(100, 10);             //用定时器T0配置并启动PWM
    ConfigTimer1(50);               //用定时器T1动态调整占空比
    while(1)
    {
        if(flag_1s)
        {
            flag_1s = 0;
            TrafficLight();
        }
    }
}

/* 配置并启动T1 */
void ConfigTimer1(unsigned int ms)
{
    unsigned long tmp;

    tmp = 11059200 / 12;
    tmp = (tmp * ms) / 1000;
    tmp = 65536 - tmp + 37;
    T1RH = (unsigned char)(tmp >> 8);
    T1RL = (unsigned char)tmp;
    TMOD &= 0x0F;                   //清零T1控制位
    TMOD |= 0x10;                   //设置T1工作模式
    TH1 = T1RH;                     //加载T1重载值
    TL1 = T1RL;    
    ET1 = 1;                        //使能T1中断
    TR1 = 1;                        //启动T1
}

/* 配置并启动PWM，fr为频率，dc为占空比 */
void ConfigPWM(unsigned int fr, unsigned char dc)
{
    unsigned int high, low;

    PeriodCnt = (11059200/12) / fr;                        //计算一个周期所需的计数值
    high = (PeriodCnt * dc) / 100;                         //计算高电平所需的计数值
    low = PeriodCnt - high;                                //计算低电平所需的计数值
    high = 65536 - high + 27;                              //计算高电平的定时器重载值，并补偿中断延时
    low = 65536 - low + 27;                                //计算低电平的定时器重载值，并补偿中断延时
    HighRH = (unsigned char)(high >> 8);                   //重载值拆分高低字节
    HighRL = (unsigned char)high;
    LowRH = (unsigned char)(low >> 8);
    LowRL = (unsigned char)low;
    TMOD &= 0xF0;                                          //清零T0控制位
    TMOD |= 0x01;                                          //配置T0为模式1
    TH0 = HighRH;
    TL0 = HighRL;                   
    ET0 = 1;
    TR0 = 1;
    PWMOUT = TrafficLightColor[color];                     //输出低电平，先亮红灯
}   

/* 占空比调整函数，频率不变只调整占空比 */
void AdjustDutyCycle(unsigned char dc)
{
    unsigned int high, low;

    high = (PeriodCnt * dc) / 100;
    low = PeriodCnt - high;
    high = 65536 - high + 27;
    low = 65536 - low + 27;
    HighRH = (unsigned char)(high >> 8);
    HighRL = (unsigned char)high;
    LowRH = (unsigned char)(low >> 8);
    LowRL = (unsigned char)low;
}

/* 交通灯状态刷新函数 */
void TrafficLight()
{
    if(timer == 0)
    {
        switch(color)
        {
            case 0:                 //切换到黄灯亮3秒
                color = 1;
                timer = 2;
                PWMOUT = TrafficLightColor[color];
                break;
            case 1:                 //切换到红灯亮10秒
                color = 2;
                timer = 9;
                PWMOUT = TrafficLightColor[color];
                break;
            case 2:                 //切换到绿灯亮10秒
                color = 0;
                timer = 9;
                PWMOUT = TrafficLightColor[color];
                break;
            default:
                break;
        }
    }
    else
    {
        timer--;
    }
}

/* T0中断服务函数，产生PWM输出 */
void InterruptTimer0() interrupt 1
{
    if(PWMOUT == 0xFF)                 //若当前正在输出高电平，装载低电平值并输出低电平(此时信号灯不亮)   
    {
        TH0 = LowRH;
        TL0 = LowRL;
        PWMOUT = TrafficLightColor[color];
    }
    else                               //若当前正在输出低电平，装载高电平值并输出高电平
    {
        TH0 = HighRH;
        TL0 = HighRL;
        PWMOUT = 0xFF;
    }
}

/* T1中断服务函数，进行1s的计时，并每50ms调整一次占空比 */
void InterruptTimer1() interrupt 3
{
    static unsigned char idx = 0;              //占空比调整表索引
    static bit dir = 0;                        //占空比变化方向标志变量      
    static unsigned char cnt_50ms = 0;
    // unsigned char code table[13] = {        //占空比调整表
    //     5, 18, 30, 41, 51, 60, 68, 75, 81, 86, 90, 93, 95
    // };
    unsigned char code table[10] = {           //占空比调整表
        10, 23, 35, 46, 56, 65, 73, 80, 86, 91
    };
    TH1 = T1RH;
    TL1 = T1RL;

    cnt_50ms++;
    if(cnt_50ms == 20)                         //计时满1秒
    {
        flag_1s = 1;
        cnt_50ms = 0;
    }

    if(timer <= 2)                             //进入3秒倒计时，动态调整占空比使得信号灯闪烁
    {    
        AdjustDutyCycle(table[idx]);
        if(!dir)                               //占空比朝增大方向变化
        {
            idx++;
            if(idx >= 9)
            {
                dir = 1;
            }
        }
        else                                   //占空比朝减小方向变化 
        {
            idx--;
            if(idx == 0)
            {
                dir = 0;
            }
        }
    }
}