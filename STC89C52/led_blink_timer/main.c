/*
 * @Descripttion: 使用定时器0实现流水灯循环左移右移
 * @Author: 五月雨
 * @Date: 2022-01-13 10:05:42
 * @LastEditors: 五月雨
 * @LastEditTime: 2022-02-03 21:59:17
 * @Board:  清翔51开发板
 * @Chip： STC89C52
 */
#include <STC89C5xRC.H>

void main()
{
    unsigned char cntOverflow = 0;    //记录定时器溢出次数
    unsigned char cntMove = 0;        //记录移位次数
    bit dir = 0;					  //流水灯移动方向标志变量，为0时左移

    TMOD = 0x01;    				  //设置工作模式为1
    TH0 = 0xB8;     				  //定时20ms
    TL0 = 0x00;
    TR0 = 1;        				  //启用定时器

    while(1)
    {
        if(TF0 == 1)
        {
            TF0 = 0;    						//定时器重载
            TH0 = 0xB8;
            TL0 = 0x00;
            cntOverflow++;
            if(cntOverflow >= 50)
            {
                cntOverflow = 0;    		    //清零溢出次数，重新开始计时
                if(!dir)
                {
                    P1 = ~(0x01 << cntMove);    //左移
                    cntMove++;
                    if(cntMove == 7)    	    //左移完成，准备右移
                    {
						dir = 1;
                        cntMove = 0;
                    }
                }
                else if(dir)
                {
                    P1 = ~(0x80 >> cntMove);    //右移
                    cntMove++;
                    if(cntMove == 7)    	    //右移完成，准备左移
                    {
						dir = 0;
                        cntMove = 0;
                    }
                }
            }
        }
    }
}