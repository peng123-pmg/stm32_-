#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "beep.h"
#include "key.h"


int main()
{
	u8 key,i=0;
	
	SysTick_Init(72);
	LED_Init();
	BEEP_Init();
	KEY_Init();
	
	while(1)
	{
		key=KEY_Scan(0);   //扫描按键
		switch(key)
		{
			case KEY_UP_PRESS: LED2=0;break;      //点亮D2指示灯
			case KEY1_PRESS: LED2=1;break;    //熄灭D2指示灯
			case KEY2_PRESS: BEEP=1;break;    //蜂鸣器开
			case KEY0_PRESS: BEEP=0;break;    //蜂鸣器关
		}
		i++;
		if(i%20==0)
		{
			LED1=!LED1;      //LED1状态取反
		}
		delay_ms(10);
	}
}
