#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "key.h"


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

//任务优先级
#define Take_TASK_PRIO		3
//任务堆栈大小	
#define Take_STK_SIZE 		512  
//任务句柄
TaskHandle_t TakeTask_Handler;
//任务函数
void Take_task(void *pvParameters);

//任务优先级
#define Give_TASK_PRIO		4
//任务堆栈大小	
#define Give_STK_SIZE 		512  
//任务句柄
TaskHandle_t GiveTask_Handler;
//任务函数
void Give_task(void *pvParameters);



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
	printf("FreeRTOS任务通知代替计数信号量实验\r\n");
	printf("车位默认值为0个，按下KEY1申请车位，按下KEY2释放车位！\n\n");
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
							 
    //创建LED1任务
    xTaskCreate((TaskFunction_t )led1_task,     
                (const char*    )"led1_task",   
                (uint16_t       )LED1_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )LED1_TASK_PRIO,
                (TaskHandle_t*  )&LED1Task_Handler); 
	
	//创建Take任务
    xTaskCreate((TaskFunction_t )Take_task,     
                (const char*    )"Take_task",   
                (uint16_t       )Take_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )Take_TASK_PRIO,
                (TaskHandle_t*  )&TakeTask_Handler);
				
	//创建Give任务
    xTaskCreate((TaskFunction_t )Give_task,     
                (const char*    )"Give_task",   
                (uint16_t       )Give_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )Give_TASK_PRIO,
                (TaskHandle_t*  )&GiveTask_Handler);
	
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

//Give任务函数
void Give_task(void *pvParameters)
{	
	BaseType_t xReturn = pdPASS;
	u8 key=0;
	
	while(1)
	{
		key=KEY_Scan(0);
		if(key==KEY2_PRESS)
		{
			xTaskNotifyGive(TakeTask_Handler);//发送任务通知
			if ( pdPASS == xReturn ) 
				printf( "KEY2被按下，释放1个停车位。\n" );
		}
		vTaskDelay(20);
	}
}

//Take任务函数
void Take_task(void *pvParameters)
{
	uint32_t take_num = pdTRUE;
	u8 key=0;
	
	while(1)
	{
		key=KEY_Scan(0);
		if(key==KEY1_PRESS)
		{
			//获取任务通知 ,没获取到则不等待
			take_num=ulTaskNotifyTake(pdFALSE,0);
			if(take_num > 0)
				printf( "KEY1被按下，成功申请到停车位，当前车位为 %d \n", take_num - 1);
			else
				printf( "KEY1被按下，车位已经没有了，请按KEY2释放车位\n" );  
		}
		vTaskDelay(20);
	}
}
