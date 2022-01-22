/*
 * @Descripttion: 数码管动态显示实现倒计时，并只保留有效位
 * @Author: 五月雨
 * @Date: 2022-01-13 16:36:08
 * @LastEditors: 五月雨
 * @LastEditTime: 2022-01-21 15:19:40
 * @Board: 清翔51开发板
 * @Chip: STC89C52
 */
#include <STC89C5xRC.H>

sbit SEG = P2^6;                    //段选
sbit DIG = P2^7;                    //位选

unsigned char code LedChar[] = {    //数码管显示字符转换表
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
    0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71
};

unsigned char LedBuf[] = {          //数码管显示缓冲区
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
bit flag_1s = 0;                //计时满1s标志

void LedDisplay(unsigned long t);
void LedScan(unsigned char x);

void main()
{
    unsigned long sec = 1000;       //计时初值可为0~9999999中的任意数值
    EA = 1;                         //使能总中断
    TMOD = 0x10;                    //使用定时器1工作模式1
    TH1 = 0xFC;                     //中断间隔为1ms
    TL1 = 0x67;
    ET1 = 1;                        //使能T1中断
    TR1 = 1;                        //开启T1定时器

    while(1)
    {
        if(flag_1s)    
        {
            flag_1s = 0;
            LedDisplay(sec);
            sec--;
        }
    }
}

/* 数码管显示字符转换函数 */
void LedDisplay(unsigned long t)
{
    signed char i;      //i=0时会进行i--操作，定义为有符号变量防止溢出
    unsigned char buf[8];

    //将秒数转换为数组表示，注意高低位的对应关系
    for(i = 7; i >= 0; i--)
    {
        buf[i] = t % 10;
        t /= 10;
    }
    
    //将转换后的数字存储到数码管显示缓冲区中，并去除有效位
    for(i = 0; i <= 6; i++)
    {
        if(buf[i] == 0)
            LedBuf[i] = 0x00;
        else
            break;
    }
    for( ; i <= 7; i++)
    {
        LedBuf[i] = LedChar[buf[i]];
    }
}

/* 数码管动态扫描刷新函数 */
void LedScan(unsigned char x)
{
    P0 = 0x00;          //消除交替重影
    SEG = 1;
    SEG = 0;
    
    P0 = 0xFF;          //位消隐
    DIG = 1;
    P0 &= ~(1 << x);
    DIG = 0;
    
    P0 = 0x00;          //段消隐
    SEG = 1;
    P0 = LedBuf[x];
    SEG = 0;
}

/* T1中断服务函数，完成数码管的动态扫描和显示刷新 */
void InterruptTimer1 () interrupt 3
{
    static unsigned int cnt = 0;    //中断计数变量
    static unsigned char idx = 0;   //数码管扫描索引
    TH1 = 0xFC;             //重载定时器值
    TL1 = 0x67;

    cnt++;                  //中断次数加1
    if(cnt >= 1000)         //计时满一秒，更新标志变量
    {
        flag_1s = 1;
        cnt = 0;
    }

    //由于每1ms中断一次，那么更新完8位数码管的时间在10ms内
    //则可以认为刷新率为100HZ，人眼看到就是不闪烁的连续显示
    LedScan(idx);           //处理数码管状态
    if(idx < 7)             //更新扫描索引
        idx++;
    else
        idx = 0;
}