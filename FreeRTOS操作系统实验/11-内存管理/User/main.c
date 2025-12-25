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
#define KEY_TASK_PRIO		4
//任务堆栈大小	
#define KEY_STK_SIZE 		512  
//任务句柄
TaskHandle_t KEYTask_Handler;
//任务函数
void key_task(void *pvParameters);


uint8_t *Test_Ptr = NULL;


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
	printf("FreeRTOS内存管理实验\r\n");
	printf("按下KEY1申请内存，按下KEY2释放内存\n");
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
	
	//创建KEY任务
    xTaskCreate((TaskFunction_t )key_task,     
                (const char*    )"key_task",   
                (uint16_t       )KEY_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )KEY_TASK_PRIO,
                (TaskHandle_t*  )&KEYTask_Handler);
	
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

//KEY任务函数
void key_task(void *pvParameters)
{
    u8 key=0;
	uint32_t g_memsize;
	
	while(1)
    {
        key=KEY_Scan(0);
		if(key==KEY1_PRESS)
		{
			if(NULL == Test_Ptr)
			{
				/* 获取当前内存大小 */
				g_memsize = xPortGetFreeHeapSize();
				printf("系统当前内存大小为 %d 字节，开始申请内存\n",g_memsize);
				Test_Ptr = pvPortMalloc(1024);
				if(NULL != Test_Ptr)
				{
					printf("内存申请成功\n");
					printf("申请到的内存地址为%#x\n",(int)Test_Ptr);

					/* 获取当前内剩余存大小 */
					g_memsize = xPortGetFreeHeapSize();
					printf("系统当前内存剩余存大小为 %d 字节\n",g_memsize);
						  
					//向Test_Ptr中写入当数据:当前系统时间
					sprintf((char*)Test_Ptr,"当前系统TickCount = %d \n",xTaskGetTickCount());
					printf("写入的数据是 %s \n",(char*)Test_Ptr);
				}
			}
			else
				printf("请先按下KEY2释放内存再申请\n");
		}
		
		if(key==KEY2_PRESS)
		{
			if(NULL != Test_Ptr)
			{
				printf("释放内存\n");
				vPortFree(Test_Ptr);//释放内存
				Test_Ptr=NULL;
				/* 获取当前内剩余存大小 */
				g_memsize = xPortGetFreeHeapSize();
				printf("系统当前内存大小为 %d 字节，内存释放完成\n",g_memsize);
			}
			else
				printf("请先按下KEY1申请内存再释放\n");
		}
		vTaskDelay(20);
    }
}
