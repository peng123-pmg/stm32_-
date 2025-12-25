#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "key.h"
#include "time.h"


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
#define Interrupt_TASK_PRIO		4
//任务堆栈大小	
#define Interrupt_STK_SIZE 		512  
//任务句柄
TaskHandle_t InterruptTask_Handler;
//任务函数
void Interrupt_task(void *pvParameters);


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
	TIM3_Init(10000-1,7200-1);//定时1S
	TIM4_Init(10000-1,7200-1);
	printf("FreeRTOS中断管理实验\r\n");
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
	
	//创建Interrupt任务
    xTaskCreate((TaskFunction_t )Interrupt_task,     
                (const char*    )"Interrupt_task",   
                (uint16_t       )Interrupt_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )Interrupt_TASK_PRIO,
                (TaskHandle_t*  )&InterruptTask_Handler);
	
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

//Interrupt任务函数
void Interrupt_task(void *pvParameters)
{
    static u32 total_num=0;
	
	while(1)
    {
		total_num+=1; 
		if(total_num==5)
		{
			printf("关闭中断.............\r\n");
			portDISABLE_INTERRUPTS(); //关闭中断
			delay_xms(5000); //延时5s，使用不影响任务调度的延时
			printf("打开中断.............\r\n"); //打开中断
			portENABLE_INTERRUPTS();
		}
		LED2=!LED2;
		vTaskDelay(1000);
    }
}
