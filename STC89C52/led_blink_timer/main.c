/*
 * @Descripttion: 使用定时器0实现流水灯循环左移右移
 * @Author: 五月雨
 * @Date: 2022-01-13 10:05:42
 * @LastEditors: 五月雨
 * @LastEditTime: 2022-01-13 10:48:44
 * @Board:  清翔51开发板
 * @Chip： STC89C52
 */
#include <STC89C5xRC.H>

void main()
{
    unsigned char cntOverflow = 0;    //记录定时器溢出次数
    unsigned char cntMove = 0;        //记录移位次数
    unsigned char leftFlag = 0;       //左移完成标志
    unsigned char rightFlag = 0;      //右移完成标志

    TMOD = 0x01;    //设置工作模式为1
    TH0 = 0xB8;     //定时20ms
    TL0 = 0x00;
    TR0 = 1;        //启用定时器

    while(1)
    {
        if(TF0 == 1)
        {
            TF0 = 0;    //定时器重载
            TH0 = 0xB8;
            TL0 = 0x00;
            cntOverflow++;
            if(cntOverflow >= 50)
            {
                cntOverflow = 0;    //清零溢出次数，重新开始计时
                if(leftFlag == 0)
                {
                    P1 = ~(0x01 << cntMove);    //左移
                    cntMove++;
                    if(cntMove == 8)    //左移完成，准备右移
                    {
                        leftFlag = 1;
                        rightFlag = 0;
                        cntMove = 0;
                    }
                }
                else if(rightFlag == 0)
                {
                    P1 = ~(0x80 >> cntMove);    //右移
                    cntMove++;
                    if(cntMove == 8)    //右移完成，准备左移
                    {
                        leftFlag = 0;
                        rightFlag = 1;
                        cntMove = 0;
                    }
                }
            }
        }
    }
}