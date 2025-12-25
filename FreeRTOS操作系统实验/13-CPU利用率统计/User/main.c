#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "key.h"
#include "time.h"
#include "string.h"


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
#define LED2_TASK_PRIO		3
//任务堆栈大小	
#define LED2_STK_SIZE 		50  
//任务句柄
TaskHandle_t LED2Task_Handler;
//任务函数
void led2_task(void *pvParameters);


//任务优先级
#define CPU_TASK_PRIO		4
//任务堆栈大小	
#define CPU_STK_SIZE 		512  
//任务句柄
TaskHandle_t CPUTask_Handler;
//任务函数
void CPU_task(void *pvParameters);


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
	TIM6_Init(100-1,72-1);//定时0.1ms
	printf("FreeRTOS CPU利用率统计\r\n");
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

	//创建LED2任务
    xTaskCreate((TaskFunction_t )led2_task,     
                (const char*    )"led2_task",   
                (uint16_t       )LED2_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )LED2_TASK_PRIO,
                (TaskHandle_t*  )&LED2Task_Handler);
	
	//创建CPU任务
    xTaskCreate((TaskFunction_t )CPU_task,     
                (const char*    )"CPU_task",   
                (uint16_t       )CPU_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )CPU_TASK_PRIO,
                (TaskHandle_t*  )&CPUTask_Handler);
	
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
		printf("LED1_Task Running,LED2_ON\r\n");
        LED1=1;
        vTaskDelay(800);
		printf("LED1_Task Running,LED2_OFF\r\n");
    }
}

//LED2任务函数
void led2_task(void *pvParameters)
{
    while(1)
    {
        LED2=0;
        vTaskDelay(800);
		printf("LED2_Task Running,LED2_ON\r\n");
        LED2=1;
        vTaskDelay(200);
		printf("LED2_Task Running,LED2_OFF\r\n");
    }
}

//CPU任务函数
void CPU_task(void *pvParameters)
{
    uint8_t CPU_RunInfo[400];//保存任务运行时间信息
	
	while(1)
    {
		memset(CPU_RunInfo,0,400);//信息缓冲区清零
		vTaskList((char *)&CPU_RunInfo);  //获取任务运行时间信息
		
		printf("---------------------------------------------\r\n");
		printf("任务名      任务状态 优先级   剩余栈 任务序号\r\n");
		printf("%s", CPU_RunInfo);
		printf("---------------------------------------------\r\n");

		memset(CPU_RunInfo,0,400);				//信息缓冲区清零

		vTaskGetRunTimeStats((char *)&CPU_RunInfo);

		printf("任务名       运行计数         利用率\r\n");
		printf("%s", CPU_RunInfo);
		printf("---------------------------------------------\r\n\n");
		vTaskDelay(1000);   /* 延时1000个tick */
    }
}
