#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "key.h"
#include "timers.h"


//任务优先级
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		128  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

//任务优先级
#define LED1_TASK_PRIO		2
//任务堆栈大小	
#define LED1_STK_SIZE 		50  
//任务句柄
TaskHandle_t LED1Task_Handler;
//任务函数
void led1_task(void *pvParameters);


TimerHandle_t Swtmr1_Handle =NULL;
TimerHandle_t Swtmr2_Handle =NULL;

static uint32_t TmrCb_Count1 = 0; /* 记录软件定时器1回调函数执行次数 */
static uint32_t TmrCb_Count2 = 0; /* 记录软件定时器2回调函数执行次数 */

static void Swtmr1_Callback(void* parameter);
static void Swtmr2_Callback(void* parameter);

/*******************************************************************************
* 函 数 名         : main
* 函数功能		   : 主函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
int main()
{
	SysTick_Init(72);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4
	LED_Init();
	KEY_Init();
	USART1_Init(115200);
	printf("FreeRTOS软件定时器实验\r\n");
	//创建开始任务
    xTaskCreate((TaskFunction_t )start_task,            //任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄              
    vTaskStartScheduler();          //开启任务调度
}

//开始任务任务函数
void start_task(void *pvParameters)
{
	taskENTER_CRITICAL();           //进入临界区
    
	//创建定时器1
	Swtmr1_Handle=xTimerCreate((const char*		)"AutoReloadTimer",
                            (TickType_t			)1000,/* 定时器周期 1000(tick) */
                            (UBaseType_t		)pdTRUE,/* 周期模式 */
                            (void*				  )1,/* 为每个计时器分配一个索引的唯一ID */
                            (TimerCallbackFunction_t)Swtmr1_Callback); 
	if(Swtmr1_Handle != NULL) 
	{
		xTimerStart(Swtmr1_Handle,0);	//开启周期定时器
	}
	
	//创建定时器2
	Swtmr2_Handle=xTimerCreate((const char*			)"OneShotTimer",
                             (TickType_t			)5000,/* 定时器周期 5000(tick) */
                             (UBaseType_t			)pdFALSE,/* 单次模式 */
                             (void*					  )2,/* 为每个计时器分配一个索引的唯一ID */
                             (TimerCallbackFunction_t)Swtmr2_Callback); 
	if(Swtmr2_Handle != NULL) 
	{
		xTimerStart(Swtmr2_Handle,0);	//开启周期定时器
	}
							 
    //创建LED1任务
    xTaskCreate((TaskFunction_t )led1_task,     
                (const char*    )"led1_task",   
                (uint16_t       )LED1_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )LED1_TASK_PRIO,
                (TaskHandle_t*  )&LED1Task_Handler); 
			
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
} 

//LED1任务函数
void led1_task(void *pvParameters)
{
    while(1)
    {
        LED1=0;
        vTaskDelay(200);
        LED1=1;
        vTaskDelay(800);
    }
}

//定时器1回调函数
//软件定时器不要调用阻塞函数，也不要进行死循环，应快进快出
void Swtmr1_Callback(void* parameter)
{		
	TickType_t tick_num1;

	TmrCb_Count1++;						/* 每回调一次加一 */

	tick_num1 = xTaskGetTickCount();	/* 获取滴答定时器的计数值 */

	LED2=!LED2;

	printf("Swtmr1_Callback函数执行 %d 次\n", TmrCb_Count1);
	printf("滴答定时器数值=%d\n", tick_num1);
}

//定时器2回调函数
//软件定时器不要调用阻塞函数，也不要进行死循环，应快进快出
void Swtmr2_Callback(void* parameter)
{		
	TickType_t tick_num2;

	TmrCb_Count2++;						/* 每回调一次加一 */

	tick_num2 = xTaskGetTickCount();	/* 获取滴答定时器的计数值 */

	printf("Swtmr2_Callback函数执行 %d 次\n", TmrCb_Count2);
	printf("滴答定时器数值=%d\n", tick_num2);
}
