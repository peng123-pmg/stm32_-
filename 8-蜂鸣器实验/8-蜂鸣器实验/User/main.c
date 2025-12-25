#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "beep.h"


/*******************************************************************************
* 函 数 名         : main
* 函数功能		   : 主函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
int main()
{
	
	HAL_Init();                     //初始化HAL库 
	SystemClock_Init(RCC_PLL_MUL9); //设置时钟,72M
	SysTick_Init(72);
	LED_Init();
	BEEP_Init();
	while(1)
	{
		LED1=!LED1;
		BEEP=!BEEP;
		delay_ms(1000);  //精确延时1s
	}
}
