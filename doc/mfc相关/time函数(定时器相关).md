time函数(定时器相关)

```C++
SetTimer函数的原型
UINT_PTR SetTimer(
HWND hWnd, // 窗口句柄
UINT_PTR nIDEvent, // 定时器ID，多个定时器时，可以通过该ID判断是哪个定时器
UINT nElapse, // 时间间隔,单位为毫秒
TIMERPROC lpTimerFunc // 回调函数 /当为空时,自动调用ontimer函数
);
```



Ontime函数(主要是刷新界面的函数)

```C++
void CLD_PowerDlg::OnTimer(UINT_PTR nIDEvent)           //定时读取数据
{
    switch (nIDEvent)   
    {  
        case 1:   //定时器1处理函数，定时发送数据进行更新
        {
            char *a=NULL;
            a = new char[12];
            a[0]=0x55;  a[1]=0xAA;  a[2]=0xdc;             //数据开头
            a[3]=0x06;
            a[4]=0x00;  
            a[5]=0x03;  
            a[6]=0x44;
            a[7]=0x44;
            a[8]=0xcc;  a[9]=0x33;  a[10]=0xc3; a[11]=0x3c;//数据结尾
            m_SerialPort.WriteData(a,12);   
            a=NULL;
            delete a;
        break; 
        }

        case 2:   //定时器2位为状态栏时间信息
        {
            CTime t1;
            t1=CTime::GetCurrentTime();//获取当前系统时间
            m_Statusbar.SetPaneText(2,t1.Format("%Y-%m-%d  %H:%M:%S")); //状态栏显示时间   
        break;   
        }
    }

    CDialogEx::OnTimer(nIDEvent);
}
```









