#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "key.h"
#include "semphr.h"


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
#define RECEIVE_TASK_PRIO		3
//任务堆栈大小	
#define RECEIVE_STK_SIZE 		50  
//任务句柄
TaskHandle_t ReceiveTask_Handler;
//任务函数
void receive_task(void *pvParameters);

//任务优先级
#define SEND_TASK_PRIO		4
//任务堆栈大小	
#define SEND_STK_SIZE 		50  
//任务句柄
TaskHandle_t SendTask_Handler;
//任务函数
void send_task(void *pvParameters);


SemaphoreHandle_t CountSem_Handle =NULL;


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
	printf("FreeRTOS计数信号量实验\r\n");
	printf("车位默认值为5个，按下KEY_UP申请车位，按下KEY1释放车位\r\n");
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
     
	/* 创建 CountSem */
	CountSem_Handle = xSemaphoreCreateCounting(5,5); 
	
    //创建LED1任务
    xTaskCreate((TaskFunction_t )led1_task,     
                (const char*    )"led1_task",   
                (uint16_t       )LED1_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )LED1_TASK_PRIO,
                (TaskHandle_t*  )&LED1Task_Handler); 
				
	//创建接收任务
    xTaskCreate((TaskFunction_t )receive_task,     
                (const char*    )"receive_task",   
                (uint16_t       )RECEIVE_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )RECEIVE_TASK_PRIO,
                (TaskHandle_t*  )&ReceiveTask_Handler);

	//创建发送任务
    xTaskCreate((TaskFunction_t )send_task,     
                (const char*    )"send_task",   
                (uint16_t       )SEND_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )SEND_TASK_PRIO,
                (TaskHandle_t*  )&SendTask_Handler);
				
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

//接收任务函数
void receive_task(void *pvParameters)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
	u8 key=0;
	
	while(1)
    {
        key=KEY_Scan(0);
		if(key==KEY1_PRESS)
		{
			xReturn = xSemaphoreTake( CountSem_Handle,0 );//获取计数信号量
			if( xReturn == pdTRUE )
				printf( "KEY1被按下，释放1个停车位。\r\n" );
			else
				printf( "KEY1被按下，但已无车位可以释放！\r\n" );
		}
		vTaskDelay(20);
    }
}

//发送任务函数
void send_task(void *pvParameters)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
	u8 key=0;
	
	while(1)
    {
		key=KEY_Scan(0);
		if(key==KEY_UP_PRESS)
		{
			xReturn = xSemaphoreGive( CountSem_Handle );//给出计数信号量
			if( xReturn == pdTRUE )
				printf("KEY_UP被按下，成功申请到停车位。\r\n");
			else
				printf("KEY_UP被按下，不好意思，现在停车场已满！\r\n");
		}
		vTaskDelay(20);
    }
}
