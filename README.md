> 该仓库存放个人在学习51单片机时写的练习程序，主要参考的学习资料为《手把手教你学51单片机 C语言版 第2版》、清翔电子的开发板相关资料。有一些写法还参考了德飞莱的例程。   
> 个人能力有限，程序中有任何错误，欢迎提issue指出。  
## 目录结构
- 程序代码存放在项目文件夹中。  
- HEX文件存放在项目文件夹的Build文件夹对应的Target文件夹中。     
## 开发工具
- 使用`KeilC51_v5`创建项目和调试；  
- 使用`VScode`的`Embedded IDE`插件进行代码编写和编译；  
- 使用烧录工具`STC-ISP`烧录程序；   
## 程序目录
1. [流水灯](./STC89C52/led_blink_timer)   
> 利用定时器计时，实现循环左右移动的流水灯效果。  
2. [8位数码管倒计时](./STC89C52/seven_seg_display_countdown)   
> 利用8为数码管实现0~99999999内任意数值的倒计时，并且只显示有效位。  
3. [矩阵键盘实现简易计算器](./STC89C52/key_matrix_calculator)
> 利用数码管和矩阵键盘实现简易的四则运算计算器，只能进行自然数之间的运算，且只能按照输入顺序进行计算，不支持运算符优先级判断。  
> 可以加入栈的数据结构来进行运算符优先级的判断，且代码中的多次运算判断的逻辑也可以用数据结构进行优化。不是目前的学习重点，暂时没有进行优化。  
4. [数码管+矩阵键盘实现定时炸弹]()