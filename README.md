电子书
平台：s3c2440 内核版本：linux-2.6.22.6
描述：使用命令行方式打开电子书应用。支持自定义显示设备（目前只支持lcd显示），支持选择字库文件，支持ASCII、UTF8、UTF16LE、UTF16BE编码格式的文本文件，使用freetype库通过Unicode码从字库文件中获取字体位图，支持显示字体大小设置，支持标准输入（stdin）、触摸屏输入（tslib库）与按键（Key）输入，并由这些输入控制电子书翻页，标准输入同时支持输入指定页码进行跳页。支持标准输出（stdout）与网络打印（socket）调试信息。

1)输入事件读取：对每种输入方式实现一个独立的子线程，在子线程中对输入事件进行阻塞读取（降低cpu占用），当读取到符合要求的输入事件之后，先获取互斥量，取得符合要求的输入事件，然后唤醒主线程，返回已经得到的输入事件。
按键驱动未使用输入子系统，使用的是自己写的一个驱动。
触摸屏控制支持滑动与点击指定区域进行翻页。
标准输入支持输入n为下一页 u为上一页 q退出


2)指定页码跳页功能：支持跳转到已显示过的页面。程序定义了一个双向链表，链表节点中记录了当前页码、当前页面内容在文本文件的起始地址以及下一页内容的起始地址。已经显示过的页面，都会放入此链表中。使用时根据页码检索到指定节点，然后进行跳转。

3)打印信息调试：支持标准输出（stdout）与网络打印（socket）调试信息。
网络打印选择使用UDP方式进行数据传输（打印数据的数据准确性要求不高），程序中使用了数据发送子进程和数据接收子进程。在每次打印数据时，会先将数据放入循环缓冲区，之后唤醒发送进程，发送进程会查看是否已经有客户端连接，如果已有客户端连接则发送打印数据。接收数据进程一直处于接收客户端信息状态，如果接收到客户端的开始打印信息会设置一个标志位告诉发送线程可以进行发送操作。在开启接收打印数据之前，也可先行设置打印等级，以及开关打印通道。

UDP的特性是：数据报，无连接，简单，不可靠，会丢包，会乱序（实际中遇到的主要是丢包）。
TCP的特性是：流式，有连接，复杂，可靠，延迟较大、带宽占用较大（均是相对于UDP来说）。
打印信息封装：可打印所属文件与行数，方便调试
DebugPrint("%s[%d]:"fmt,__FILE__,__LINE__,##__VA_ARGS__)
1)  __VA_ARGS__ 是一个可变参数的宏，很少人知道这个宏，这个可变参数的宏是新的C99规范中新增的，目前似乎只有gcc支持（VC6.0的编译器不支持）。宏前面加上##的作用在于，当可变参数的个数为0时，这里的##起到把前面多余的","去掉的作用,否则会编译出错, 你可以试试。
2) __FILE__ 宏在预编译时会替换成当前的源文件名
3) __LINE__ 宏在预编译时会替换成当前的行号



4)字符的显示：首先通过open()打开文本文件，使用fstat()获取文件大小，根据其大小使mmap()函数将文本文件内容映射到内存中。根据文件开头的数据（可参照第5点）得到符合的文件编码处理方式。显示时，获取一个字符的unicode码，判断是否是回车换行TAB等特殊字符。如果是一个普通的字符，使用freetype库提供的API，从字库文件中找到该字符的位图文件，如果页面还能继续显示字符，就将字体位图像素（支持1,8位像素格式）拷贝显存中。

5)判断文件编码格式的方式：文件起始位置上的几个字节数据标识了文件编码格式
utf8 - 0xEF 0xBB 0xBF
utf16le - 0xFF 0xFE
utf16be - 0xFE 0xFF
ACSII - 如果以上三者都不是的话 则是ASCII编码方式
如UTF8码为 0xe4 0xb8 0xad
0xe4 1110 0100   前面有三个1 代表该字符需要三个字节表示，00100需要保存下来
0xb8 1011 1000   10需要去掉 
0xad 1010 1101   10需要去掉

得到的Unicode编码是 00100  111000  101101   4E 2D

6)freetype库：使用提供的API，通过字符unicode码在字库文件中获取到的字体位图。位图文件的起始坐标位置在左下角，而LCD坐标系的起始位置在左上角，在写入到显存之前需要进行Y轴坐标系转换。通常为转化坐标Y值=屏幕分辨率-原始坐标Y值。对于单个字体而言，即转化坐标Y值=字体大小-原始坐标Y值。

7)lcd显示刷新：在程序运行的最开始，使用open()函数打开/dev/fb0帧缓冲设备文件，打开后使用ioctl()函数获得LCD的x,y分辨率以及像素深度（BPP,每个像素所占位数），然后根据以上信息使用mmap将帧缓冲设备映射到内存中，以后如果想在lcd上显示内容，可直接对这块内存（FrameBuffer）操作即可。 
https://www.cnblogs.com/xiaojianliu/p/8473095.html

8)使用getopt处理命令行指令：见链接。
