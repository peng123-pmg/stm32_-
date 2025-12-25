#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "key.h"
#include "limits.h"

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
#define Receive1_TASK_PRIO		3
//任务堆栈大小	
#define Receive1_STK_SIZE 		512  
//任务句柄
TaskHandle_t Receive1Task_Handler;
//任务函数
void Receive1_task(void *pvParameters);

//任务优先级
#define Receive2_TASK_PRIO		4
//任务堆栈大小	
#define Receive2_STK_SIZE 		512  
//任务句柄
TaskHandle_t Receive2Task_Handler;
//任务函数
void Receive2_task(void *pvParameters);

//任务优先级
#define Send_TASK_PRIO		5
//任务堆栈大小	
#define Send_STK_SIZE 		512  
//任务句柄
TaskHandle_t SendTask_Handler;
//任务函数
void Send_task(void *pvParameters);


#define  USE_CHAR  0  /* 测试字符串的时候配置为 1 ，测试变量配置为 0  */

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
	printf("FreeRTOS任务通知代替消息队列实验\r\n");
	printf("按下KEY_UP或者KEY1进行任务消息通知发送 \n");
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
	
	//创建Receive1任务
    xTaskCreate((TaskFunction_t )Receive1_task,     
                (const char*    )"Receive1_task",   
                (uint16_t       )Receive1_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )Receive1_TASK_PRIO,
                (TaskHandle_t*  )&Receive1Task_Handler);
				
	//创建Receive2任务
    xTaskCreate((TaskFunction_t )Receive2_task,     
                (const char*    )"Receive2_task",   
                (uint16_t       )Receive2_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )Receive2_TASK_PRIO,
                (TaskHandle_t*  )&Receive2Task_Handler);
	
	//创建Send任务
    xTaskCreate((TaskFunction_t )Send_task,     
                (const char*    )"Send_task",   
                (uint16_t       )Send_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )Send_TASK_PRIO,
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

//Receive1任务函数
void Receive1_task(void *pvParameters)
{
    BaseType_t xReturn = pdTRUE;
#if USE_CHAR
  char *r_char;
#else
  uint32_t r_num;
#endif
	
	while(1)
    {
		//获取任务通知 ,没获取到则一直等待
		xReturn=xTaskNotifyWait(0x0,			//进入函数的时候不清除任务bit
								ULONG_MAX,	  //退出函数的时候清除所有的bit
#if USE_CHAR
								(uint32_t *)&r_char,		  //保存任务通知值
#else
								&r_num,		  //保存任务通知值
#endif                        
								portMAX_DELAY);	//阻塞时间
		if( pdTRUE == xReturn )
#if USE_CHAR
			printf("Receive1_Task 任务通知消息为 %s \n",r_char);                      
#else
			printf("Receive1_Task 任务通知消息为 %d \n",r_num);                      
#endif  
		
	}
}

//Receive2任务函数
void Receive2_task(void *pvParameters)
{
	BaseType_t xReturn = pdTRUE;
#if USE_CHAR
  char *r_char;
#else
  uint32_t r_num;
#endif
	
	while(1)
    {
		//获取任务通知 ,没获取到则一直等待
		xReturn=xTaskNotifyWait(0x0,			//进入函数的时候不清除任务bit
								ULONG_MAX,	  //退出函数的时候清除所有的bit
#if USE_CHAR
								(uint32_t *)&r_char,		  //保存任务通知值
#else
								&r_num,		  //保存任务通知值
#endif                        
								portMAX_DELAY);	//阻塞时间
		if( pdTRUE == xReturn )
#if USE_CHAR
			printf("Receive2_Task 任务通知消息为 %s \n",r_char);                      
#else
			printf("Receive2_Task 任务通知消息为 %d \n",r_num);                      
#endif  
		
	}
}

//Send任务函数
void Send_task(void *pvParameters)
{
	BaseType_t xReturn = pdPASS;
	u8 key=0;
#if USE_CHAR
	char test_str1[] = "this is a mail test 1";/* 邮箱消息test1 */
	char test_str2[] = "this is a mail test 2";/* 邮箱消息test2 */
#else
	uint32_t send1 = 1;
	uint32_t send2 = 2;
#endif
	
	while(1)
	{
		key=KEY_Scan(0);
		if(key==KEY_UP_PRESS)
		{
			xReturn = xTaskNotify(Receive1Task_Handler, /*任务句柄*/
#if USE_CHAR 
								(uint32_t)&test_str1, /* 发送的数据，最大为4字节 */
#else
								send1, /* 发送的数据，最大为4字节 */
#endif
								eSetValueWithOverwrite );/*覆盖当前通知*/
			if( xReturn == pdPASS )
				printf("Receive1_Task_Handle 任务通知消息发送成功!\r\n");
		}
		else if(key==KEY1_PRESS)
		{
			xReturn = xTaskNotify(Receive2Task_Handler, /*任务句柄*/
#if USE_CHAR 
								(uint32_t)&test_str2, /* 发送的数据，最大为4字节 */
#else
								send2, /* 发送的数据，最大为4字节 */
#endif
								eSetValueWithOverwrite );/*覆盖当前通知*/
			if( xReturn == pdPASS )
				printf("Receive2_Task_Handle 任务通知消息发送成功!\r\n");
		}
		vTaskDelay(20);
	}
}
