/*
 * @Descripttion: 简易四则运算计算器（参与运算的数、最终结果和中间结果都只能为自然数，不支持运算优先级判断，只能按照输入顺序依次计算）
 * @Author: 五月雨
 * @Date: 2022-01-21 15:33:38
 * @LastEditors: 五月雨
 * @LastEditTime: 2022-01-23 18:03:20
 * @Board: 清翔51开发板
 * @Chip: STC89C52
 */
#include <STC89C5xRC.H>

sbit SEG = P2^6;                        //数码管位选
sbit DIG = P2^7;                        //数码管段选
sbit KEY_ROW_1 = P3^0;
sbit KEY_ROW_2 = P3^1;
sbit KEY_ROW_3 = P3^2;
sbit KEY_ROW_4 = P3^3;
sbit KEY_COL_1 = P3^4;     
sbit KEY_COL_2 = P3^5;
sbit KEY_COL_3 = P3^6;
sbit KEY_COL_4 = P3^7;

unsigned char code LedChar[] = {        //数码管显示字符转换表
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 
    0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71
};
unsigned char LedBuf[] = {              //数码管显示缓冲区
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
unsigned char code KeyCodeMap[4][4] = { //按键映射
    {0x31, 0x32, 0x33, 0x25},           /*  1    2    3    +  */
    {0x34, 0x35, 0x36, 0x26},           /*  4    5    6    -  */
    {0x37, 0x38, 0x39, 0x27},           /*  7    8    9    *  */
    {0x30, 0x1B, 0x0D, 0x28}            /*  0    CE   =    /  */
};
unsigned char KeySta[4][4] = {          //按键当前状态存储
    {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}
};

//运算种类标记变量
bit flag_add = 0;
bit flag_sub = 0;
bit flag_mul = 0;
bit flag_div = 0;
//以下变量定义为全局变量方便函数的编写
unsigned long result = 0;                //保存运算结果
unsigned long op = 0;                    //保存参与运算的数字
unsigned char cnt_opration = 0;          //记录运算次数，用于多次运算中判断是否需要计算中间结果

void ShowNumber(unsigned long num);
void DoCalculate();
void KeyAction(unsigned char keycode);
void KeyDriver();
void LedScan(unsigned char x);
void KeyScan();

void main()
{
    //注意此处的中断时间需要考虑两个方面
    //一是数码管扫描刷新率要较高（扫描完8位最好在10ms之内完成，可以粗略认为扫描一位需要1ms）
    //那么中断时间在1~2ms的刷新率都是较高的
    //二是人按下按键在到松手的时间，一般在100ms左右，刻意加速也只能到40~50ms左右
    //所以中断扫描时间在1~2ms，也能够满足在这个时间内轮询完所有矩阵按键的状态
    //这里推荐中断时间采用1ms
    EA = 1;                     //使能总中断
    TMOD = 0x01;                //使用定时器0的工作模式1
    TH0 = 0xFC;                 //中断间隔时间为1ms
    TL0 = 0x67;
    ET0 = 1;                    //使能T0中断
    TR0 = 1;                    //开启T0
    LedBuf[7] = LedChar[0];     //上电显示0

    while (1)
    {
        KeyDriver();
    }
}

/* 将一个无符号长整形的数字显示到数码管上，num为待显示数字 */
void ShowNumber(unsigned long num)
{
    signed char i;              //注意使用有符号类型防止溢出
    unsigned char buf[8];       //显示转换缓冲区
    for(i = 7; i >= 0; i--)     //将num转换为数组表示，此处数组中逆序存放数字是为了便于后面遍历
    {
        buf[i] = num % 10;
        num /= 10;
    }
    for(i = 0; i <= 6; i++)      //去除前缀0
    {
        if(buf[i] == 0)
            LedBuf[i] = 0x00;
        else
            break;
    }
    for( ; i <= 7; i++)         //剩余低位转换为数码管显示字符
    {
        LedBuf[i] = LedChar[buf[i]];
    }
}

/* 运算执行函数，根据运算种类标记变量来执行 */
void DoCalculate()
{
    if(flag_add)
    {
        result += op;
        flag_add = 0;
    }
    else if(flag_sub)
    {
        result -= op;
        flag_sub = 0;
    }
    else if(flag_mul)
    {
        result *= op;
        flag_mul = 0;
    }
    else if(flag_div)
    {
        result /= op;
        flag_div = 0;
    }
}

/* 按键动作函数，根据键码执行相应的操作，keycode为按键键码 */
void KeyAction(unsigned char keycode)
{
    if((keycode >= 0x30) && (keycode <= 0x39))  //输入0~9的数字
    {
        op = (op * 10) + (keycode - 0x30);
        ShowNumber(op);
    }
    else if(keycode == 0x25)    //加法运算
    {
        if(cnt_opration > 0)    //若式子中运算次数大于一，先进行一次运算
        {
            DoCalculate();      //得到的运算结果用于进行下一次运算
            cnt_opration--;
        }
        else
        {
            result = op;        //将op的值存储到result中，便于第二个运算数的输入
        } 
        flag_add = 1;           //标记有一次加运算需要执行
        cnt_opration++;         //等待执行的运算数量加一
        op = 0;                 //清零op变量，以存储下一个运算数的输入
    }
    else if(keycode == 0x26)    //减法运算
    {
        if(cnt_opration > 0)
        {
            DoCalculate();
            cnt_opration--;
        }
        else
        {
            result = op;
        } 
        flag_sub = 1;
        cnt_opration++;
        op = 0; 
    }
    else if(keycode == 0x27)    //乘法运算
    {
        if(cnt_opration > 0)
        {
            DoCalculate();
            cnt_opration--;
        }
        else
        {
            result = op;
        } 
        flag_mul = 1;
        cnt_opration++;
        op = 0;
    }
    else if(keycode == 0x28)    //除法运算
    {
        if(cnt_opration > 0)
        {
            DoCalculate();
            cnt_opration--;
        }
        else
        {
            result = op;
        } 
        flag_div = 1;
        cnt_opration++;
        op = 0;
    }
    else if(keycode == 0x0D)    //等号
    {
        DoCalculate();
        ShowNumber(result);
        result = 0;
        cnt_opration = 0;
        op = 0;
    }
    else if (keycode == 0x1B)   //标准计算器中的CE操作
    {
        op = 0;
        ShowNumber(op);
    }
}

/* 按键驱动函数，检测按键动作，调度相应动作函数 */
void KeyDriver()
{
    unsigned char i, j;
    static unsigned char backup[4][4] = {    //按键状态备份，保存前一次的值
        {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}
    };

    //矩阵键盘按键状态循环检测
    for(i = 0; i < 4; i++)
    {
        for(j = 0; j < 4; j++)
        {
            if(backup[i][j] != KeySta[i][j]) //检测按键状态变化，若这一次与前一次状态不同，则按键产生了动作，从而根据需要来执行操作
            {
                if(backup[i][j] != 0)        //若该按键按下
                {
                    KeyAction(KeyCodeMap[i][j]);
                }
                backup[i][j] = KeySta[i][j];
            }
        }
    }
}

/* 按键扫描函数，需要在定时中断中调用 */
void KeyScan()
{
    unsigned char keycol;                                    //矩阵按键扫描列索引
    static unsigned char keyrow = 0;                         //矩阵按键扫描行索引
    static unsigned char keybuf[4][4] = {                    //矩阵按键状态缓冲区
        {0xFF, 0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF, 0xFF},
        {0xFF, 0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF, 0xFF}
    };

    //将当前行的每个按键状态存入缓冲区中
    keybuf[keyrow][0] = (keybuf[keyrow][0] << 1) | KEY_COL_1;
    keybuf[keyrow][1] = (keybuf[keyrow][1] << 1) | KEY_COL_2;
    keybuf[keyrow][2] = (keybuf[keyrow][2] << 1) | KEY_COL_3;
    keybuf[keyrow][3] = (keybuf[keyrow][3] << 1) | KEY_COL_4;

    //由于机械按键的抖动时间一般在10ms内
    //此处扫描针对于每个按键连续采样四次，采样时间为1ms
    //由于每次中断只扫描一行，下列判断成立的条件为
    //当一个按键连续16ms保持按下或弹起，则可认为按键就是按下或弹起状态
    //这样既检测到了按键状态，又能达到去抖效果
    for(keycol = 0; keycol < 4; keycol++)
    {
        if((keybuf[keyrow][keycol] & 0x0F) == 0x00)          //按键按下
            KeySta[keyrow][keycol] = 0;
        else if((keybuf[keyrow][keycol] & 0x0F) == 0x0F)     //按键弹起
            KeySta[keyrow][keycol] = 1;
    }

    keyrow++;                                                //准备进行下一行的扫描
    keyrow &= 0x03;                                  //控制行索引在0~3以内变化
    switch(keyrow)                                           //根据行索引改变电平，以进行对应某行按键的扫描
    {
        case 0: KEY_ROW_4 = 1; KEY_ROW_1 = 0; break;
        case 1: KEY_ROW_1 = 1; KEY_ROW_2 = 0; break;
        case 2: KEY_ROW_2 = 1; KEY_ROW_3 = 0; break;
        case 3: KEY_ROW_3 = 1; KEY_ROW_4 = 0; break;
        default: break;
    }
}

/* 数码管动态扫描刷新显示函数，需在定时器中断中调用 */
void LedScan(unsigned char x)
{
    P0 = 0x00;      //防止产生交替重影
    SEG = 1;
    SEG = 0;

    P0 = 0xFF;      //位消隐
    DIG = 1;
    P0 &= ~(1 << x);
    DIG = 0;
    
    P0 = 0x00;      //段消隐
    SEG = 1;
    P0 = LedBuf[x];
    SEG = 0;
}

/* T0中断服务函数，用于数码管显示扫描与按键扫描 */
void InterruptTimer0() interrupt 1
{
    static unsigned idx = 0;                        //数码管扫描位索引
    
    TH0 = 0xFC;                                     //T0值重载
    TL0 = 0x67;

    LedScan(idx);
    if(idx < 7)
        idx++;
    else 
        idx = 0;
    KeyScan();
}