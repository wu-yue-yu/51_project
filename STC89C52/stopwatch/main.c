/*
 * @Descripttion: 实现一个实用秒表（精确到0.01s），可以启停、复位秒表。只显示有效位。
 * @Author: 五月雨
 * @Date: 2022-01-13 16:36:08
 * @LastEditors: 五月雨
 * @LastEditTime: 2022-01-31 11:56:33
 * @Board: 清翔51开发板
 * @Chip: STC89C52
 */
#include <STC89C5xRC.H>

sbit SEG = P2^6;                    //段选
sbit DIG = P2^7;                    //位选
sbit KEY_S2 = P3^0;                 
sbit KEY_S3 = P3^1;
sbit KEY_S4 = P3^2;
sbit KEY_S5 = P3^3;

unsigned char code LedChar[] = {    //数码管显示字符转换表
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
    0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71
};
unsigned char LedBuf[] = {          //数码管显示缓冲区
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
unsigned char KeyStaCur[] = {       //按键当前状态
    1, 1, 1, 1
};

bit StopwatchRunning = 0;           //秒表运行状态标志
bit StopwatchRefresh = 1;           //秒表计数刷新标志，初始为1，以刷新显示初值
unsigned char DecimalPart = 0;      //秒表计时小数部分
unsigned int IntegerPart = 0;       //秒表计时整数部分
//下列写法便于更改定时器中断间隔（定时器初始化配置过程写为函数）
unsigned char T0RH = 0;             //T0重装值的高字节
unsigned char T0RL = 0;             //T0重装值的低字节

void ConfigTimer0ms(unsigned int ms);
void StopwatchDisplay();
void StopwatchAction();
void StopwatchReset();
void KeyDriver();
void LedScan();
void KeyScan();
void StopwatchCount();

void main()
{
    EA = 1;
    ConfigTimer0ms(2);  //T0定时为2ms（独立键盘扫描2ms足够快）

    while(1)
    {
        if(StopwatchRefresh)
        {
            StopwatchRefresh = 0;
            StopwatchDisplay();
        }
        KeyDriver();
    }
}

/* 定时器初始化函数 */
void ConfigTimer0ms(unsigned int ms)
{
    unsigned long tmp;
    tmp = 11059200 / 12;
    tmp = (tmp * ms) / 1000;
    tmp = 65536 - tmp;
    tmp += 32;              //补偿中断相应延时造成的误差，通过keil测得约为32个机器周期
    TMOD = 0x01;
    /*
    可以写成这种形式防止影响到tmod的其他位，但这里没必要
    TMOD &= 0xF0;
    TMOD |= 0x01;
    */
    T0RH = (unsigned char)(tmp >> 8);
    T0RL = (unsigned char)tmp;
    ET0 = 1;
    TR0 = 1;
}

/* 秒表计数转换为数码管显示字符函数 */
void StopwatchDisplay()
{
    unsigned char i;
    unsigned char buf[4];

    LedBuf[7] = LedChar[DecimalPart % 10];       //这里小数部分都需要显示
    LedBuf[6] = LedChar[DecimalPart / 10];       //所以不需要放到缓冲区

    buf[3] = IntegerPart % 10;          //放入缓冲区，避免影响Ledbuf中的数据
    buf[2] = (IntegerPart/10) % 10;
    buf[1] = (IntegerPart/100) % 10;
    buf[0] = (IntegerPart/1000) % 10;

    for(i = 0; i <= 2; i++)             //无效位不显示
    {
        if(buf[i] == 0)
            LedBuf[i+2] = 0x00;
        else
            break;
    }
    for( ; i <= 3; i++)                 //只显示有效位
    {
        LedBuf[i+2] = LedChar[buf[i]];
    }
    LedBuf[5] |= (1 << 7);              //点亮小数点
}

/* 秒表启停函数 */
void StopwatchAction()
{
    if(StopwatchRunning)
        StopwatchRunning = 0;
    else 
        StopwatchRunning = 1;
}

/* 秒表复位函数 */
void StopwatchReset()
{
    StopwatchRunning = 0;               //停止秒表
    DecimalPart = 0;                    //清0计数值
    IntegerPart = 0;
    StopwatchRefresh = 1;               //标记请求复位
}

/* 按键驱动函数 */
void KeyDriver()
{
    unsigned char i;
    static unsigned char backup[] = {1, 1, 1, 1};     //备份上一次按键的状态

    for(i = 0; i < 4; i++)
    {
        if(backup[i] != KeyStaCur[i])
        {
            if(KeyStaCur[i] == 0)                   //若按键按下
            {
                if(i == 1)                          //若按下复位键
                    StopwatchReset();               //复位秒表
                else if(i == 2)                     //若按下启停键
                    StopwatchAction();              //启停秒表
            }
            backup[i] = KeyStaCur[i];               //更新前一次备份值
        }
    }
}

/* 数码管动态扫描函数 */
void LedScan()
{
    static unsigned char idx = 0;

    P0 = 0x00;          //避免产生交替重影
    SEG = 1;
    SEG = 0;

    P0 = 0xFF;          //位消隐
    DIG = 1;
    P0 &= ~(1 << idx);
    DIG = 0;

    P0 = 0x00;          //段消隐
    SEG = 1;
    P0 = LedBuf[idx];
    SEG = 0;

    if(idx < 7)         //更新数码管扫描索引
        idx++;
    else 
        idx = 0;
}

/* 键盘动态扫描函数 */
void KeyScan()
{
    unsigned char i;
    static unsigned char keybuf[] = {
        0xFF, 0xFF, 0xFF, 0xFF
    };

    keybuf[0] = (keybuf[0] << 1) | KEY_S2;
    keybuf[1] = (keybuf[1] << 1) | KEY_S3;
    keybuf[2] = (keybuf[2] << 1) | KEY_S4;
    keybuf[3] = (keybuf[3] << 1) | KEY_S5;

    //扫描独立按键，只有4个键
    //取2ms采用间隔，8次连续采样的值来判断按键状态
    //一般按键时间不超过100ms，且不低于40~50ms
    //这里16ms即可扫描完所有独立按键
    //但是由于要保证数码管动态显示效果，定时时间最好不超过2ms
    for(i = 0; i < 4; i++)
    {
        if(keybuf[i] == 0x00)
            KeyStaCur[i] = 0;
        else if(keybuf[i] == 0xFF)
            KeyStaCur[i] = 1;
    }
}

/* 秒表计时函数 */
void StopwatchCount()
{
    if(StopwatchRunning)                //若秒表仍在计数，则更新计数值
    {   
        DecimalPart++;                  //满10ms，小数部分加1
        if(DecimalPart >= 100)          //满1000ms，即满1s
        {
            IntegerPart++;              //整数部分加1
            DecimalPart = 0;            //并把小数部分清零
            if(IntegerPart >= 10000)    //控制计时范围在0~10000秒
            {
                IntegerPart = 0;
            }
        }
        StopwatchRefresh = 1;  
    }         //计时值更新后，设置秒表刷新标志为1
}

/* 中断服务函数，完成秒表计时，数码管和按键的动态扫描*/
void InterruptTimer0() interrupt 1
{
    static unsigned char timer_2ms = 0;     
    
    TH0 = T0RH;                 
    TL0 = T0RL;
    
    LedScan();
    KeyScan();

    timer_2ms++;                //中断一次为2ms，5次即为10ms
    if(timer_2ms >= 5)          //计时满10ms
    {
        timer_2ms = 0;
        StopwatchCount();       //调用秒表计时函数
    }
}