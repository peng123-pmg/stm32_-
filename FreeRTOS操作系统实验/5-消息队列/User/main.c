#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "key.h"
#include "queue.h"


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


QueueHandle_t Test_Queue =NULL;

#define  QUEUE_LEN    4   /* 队列的长度，最大可包含多少个消息 */
#define  QUEUE_SIZE   4   /* 队列中每个消息大小（字节） */


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
	printf("FreeRTOS消息队列实验\r\n");
	printf("按下KEY_UP或者KEY1发送队列消息\r\n");
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
     
	/* 创建Test_Queue */
	Test_Queue = xQueueCreate((UBaseType_t ) QUEUE_LEN,/* 消息队列的长度 */
                            (UBaseType_t ) QUEUE_SIZE);/* 消息的大小 */
	
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
    BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdTRUE */
	uint32_t r_queue;	/* 定义一个接收消息的变量 */
	
	while(1)
    {
        xReturn = xQueueReceive( Test_Queue,    /* 消息队列的句柄 */
								&r_queue,      /* 发送的消息内容 */
								portMAX_DELAY); /* 等待时间 一直等 */
		if(pdTRUE == xReturn)
			printf("本次接收到的数据是%d\n\n",r_queue);
		else
			printf("数据接收出错,错误代码0x%lx\n",xReturn);
    }
}

//发送任务函数
void send_task(void *pvParameters)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
	uint32_t send_data1 = 1;
	uint32_t send_data2 = 2;
	u8 key=0;
	
	while(1)
    {
		key=KEY_Scan(0);
		if(key==KEY_UP_PRESS)
		{
			printf("发送消息send_data1！\n");
			xReturn = xQueueSend( Test_Queue, /* 消息队列的句柄 */
								&send_data1,/* 发送的消息内容 */
								0 );        /* 等待时间 0 */
			if(pdPASS == xReturn)
				printf("消息send_data1发送成功!\n\n");
		}
		else if(key==KEY1_PRESS)
		{
			printf("发送消息send_data2！\n");
			xReturn = xQueueSend( Test_Queue, /* 消息队列的句柄 */
								&send_data2,/* 发送的消息内容 */
								0 );        /* 等待时间 0 */
			if(pdPASS == xReturn)
				printf("消息send_data2发送成功!\n\n");
		}
		vTaskDelay(20);
    }
}
