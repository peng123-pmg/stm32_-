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


SemaphoreHandle_t BinarySem_Handle =NULL;


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
	printf("FreeRTOS二值信号量实验\r\n");
	printf("按下KEY_UP或者KEY1进行任务与任务间的同步\r\n");
	printf("Receive任务接收到消息在串口回显\r\n");
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
     
	/* 创建 BinarySem */
	BinarySem_Handle = xSemaphoreCreateBinary();	 
	
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
	
	while(1)
    {
        //获取二值信号量 xSemaphore,没获取到则一直等待
		xReturn = xSemaphoreTake(BinarySem_Handle,/* 二值信号量句柄 */
								portMAX_DELAY); /* 等待时间 */
		if(pdTRUE == xReturn)
			printf("BinarySem_Handle二值信号量获取成功!\n\n");
		LED2=!LED2;
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
			xReturn = xSemaphoreGive( BinarySem_Handle );//给出二值信号量
			if( xReturn == pdTRUE )
				printf("BinarySem_Handle二值信号量释放成功!\r\n");
			else
				printf("BinarySem_Handle二值信号量释放失败!\r\n");
		}
		else if(key==KEY1_PRESS)
		{
			xReturn = xSemaphoreGive( BinarySem_Handle );//给出二值信号量
			if( xReturn == pdTRUE )
				printf("BinarySem_Handle二值信号量释放成功!\r\n");
			else
				printf("BinarySem_Handle二值信号量释放失败!\r\n");
		}
		vTaskDelay(20);
    }
}
