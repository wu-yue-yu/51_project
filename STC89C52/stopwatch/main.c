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
    TH0 = 0xFC;                     //中断间隔为1ms
    TL0 = 0x67;
    ET0 = 1;                        //使能T1中断
    TR0 = 1;                        //开启T1定时器

    while(1)
    {

            flag_1s = 0;

            sec--;

    }
}


/* T1中断服务函数，完成数码管的动态扫描和显示刷新 */
void InterruptTimer1 () interrupt 1
{
    static unsigned int cnt = 0;    //中断计数变量
    static unsigned char idx = 0;   //数码管扫描索引
    TH0 = 0xFC;             //重载定时器值
    TL0 = 0x67;




}