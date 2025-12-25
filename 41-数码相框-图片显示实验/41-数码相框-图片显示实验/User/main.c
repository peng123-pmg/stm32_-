#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "usart.h"
#include "tftlcd.h"
#include "key.h"
#include "malloc.h" 
#include "sd_sdio.h"
#include "flash.h"
#include "ff.h" 
#include "fatfs_app.h"
#include "key.h"
#include "font_show.h"
#include "piclib.h"
#include "string.h"		
#include "math.h"


int main()
{
	u8 i=0;		
	u8 key;
	u8 res;
 	DIR picdir;	 		//图片目录
	FILINFO *picfileinfo;//文件信息 
	u8 *pname;			//带路径的文件名
	u16 totpicnum; 		//图片文件总数
	u16 curindex;		//图片当前索引
	u8 pause=0;			//暂停标记
	u16 temp;
	u32 *picoffsettbl;	//图片文件offset索引表
	
	SysTick_Init(72);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //中断优先级分组 分2组
	LED_Init();
	USART1_Init(115200);
	TFTLCD_Init();			//LCD初始化
	KEY_Init();
	EN25QXX_Init();					 
	my_mem_init(SRAMIN);		//初始化内部内存池
	
	FRONT_COLOR=RED;
	while(SD_Init())//检测不到SD卡
	{
		LCD_ShowString(10,10,tftlcd_data.width,tftlcd_data.height,16,"SD Card Error!");
		delay_ms(500);
		LED2=!LED2;
	}
	
	FATFS_Init();				//为fatfs相关变量申请内存				 
  	f_mount(fs[0],"0:",1); 		//挂载SD卡
	f_mount(fs[1],"1:",1); 		//挂载SPI FLASH
	
	while(font_init()) 		        //检查字库
	{  
		LCD_ShowString(10,10,tftlcd_data.width,tftlcd_data.height,16,"Font Error!   ");
		delay_ms(500);
	} 
	LCD_ShowFontString(10,10,tftlcd_data.width,tftlcd_data.height,"普中科技-PRECHIN",16,0);
	LCD_ShowFontString(10,30,tftlcd_data.width,tftlcd_data.height,"www.prechin.net",16,0);
	LCD_ShowFontString(10,50,tftlcd_data.width,tftlcd_data.height,"数码相框-图片显示",16,0);
	
	while(f_opendir(&picdir,"0:/PICTURE"))//打开图片文件夹
 	{	    
		LCD_ShowFontString(10,70,tftlcd_data.width,tftlcd_data.height,"PICTURE文件夹错误!",16,0);
		delay_ms(200);
		LCD_Fill(10,70,tftlcd_data.width,86,WHITE);//清除显示
		delay_ms(200);
	}  
	totpicnum=fatfs_get_filetype_tnum("0:/PICTURE",TYPE_PICTURE); //得到总有效文件数
  	while(totpicnum==NULL)//图片文件为0		
 	{	    
		LCD_ShowFontString(10,70,tftlcd_data.width,tftlcd_data.height,"没有图片文件!",16,0);
		delay_ms(200);
		LCD_Fill(10,70,tftlcd_data.width,86,WHITE);//清除显示
		delay_ms(200);		
	} 
	picfileinfo=(FILINFO*)mymalloc(SRAMIN,sizeof(FILINFO));	//申请内存
 	pname=mymalloc(SRAMIN,_MAX_LFN*2+1);	//为带路径的文件名分配内存
 	picoffsettbl=mymalloc(SRAMIN,4*totpicnum);	//申请4*totpicnum个字节的内存,用于存放图片索引
 	while(!picfileinfo||!pname||!picoffsettbl)	//内存分配出错
 	{	    	
		LCD_ShowFontString(10,70,tftlcd_data.width,tftlcd_data.height,"内存分配失败!",16,0);
		delay_ms(200);				  
		LCD_Fill(10,70,tftlcd_data.width,86,WHITE);//清除显示
		delay_ms(200);				  
	}  	
	//记录索引
    res=f_opendir(&picdir,"0:/PICTURE"); //打开目录
	if(res==FR_OK)
	{
		curindex=0;//当前索引为0
		while(1)//全部查询一遍
		{
			temp=picdir.dptr;								//记录当前dptr偏移
	        res=f_readdir(&picdir,picfileinfo);       		//读取目录下的一个文件
	        if(res!=FR_OK||picfileinfo->fname[0]==0)break;	//错误了/到末尾了,退出	 	 
			res=f_typetell((u8*)picfileinfo->fname);	
			if((res&0XF0)==TYPE_PICTURE)//取高四位,看看是不是图片文件	
			{
				picoffsettbl[curindex]=temp;//记录索引
				curindex++;
			}	    
		} 
	}   
	LCD_ShowFontString(10,70,tftlcd_data.width,tftlcd_data.height,"开始显示...",16,0); 
	delay_ms(1500);
	piclib_init();										//初始化画图	   	   
	curindex=0;											//从0开始显示
   	res=f_opendir(&picdir,(const TCHAR*)"0:/PICTURE"); 	//打开目录
	while(res==FR_OK)//打开成功
	{	
		dir_sdi(&picdir,picoffsettbl[curindex]);			//改变当前目录索引	   
        res=f_readdir(&picdir,picfileinfo);       		//读取目录下的一个文件
        if(res!=FR_OK||picfileinfo->fname[0]==0)break;	//错误了/到末尾了,退出
		strcpy((char*)pname,"0:/PICTURE/");				//复制路径(目录)
		strcat((char*)pname,(const char*)picfileinfo->fname);//将文件名接在后面
 		LCD_Clear(BLACK);
 		picfile_display(pname,0,0,tftlcd_data.width,tftlcd_data.height,1);//显示图片    
		LCD_ShowFontString(2,2,tftlcd_data.width,16,pname,16,1); 				//显示图片名字
		i=0;
		while(1) 
		{
			key=KEY_Scan(0);		//扫描按键
			if(i>250)
				key=1;			//模拟一次按下K_DOWN    
			if(i%10==0)
				LED1=!LED1;//D1闪烁,提示程序正在运行.
			if(key==KEY_UP_PRESS)		//上一张
			{
				if(curindex)curindex--;
				else curindex=totpicnum-1;
				break;
			}
			else if(key==KEY1_PRESS)//下一张
			{
				curindex++;		   	
				if(curindex>=totpicnum)curindex=0;//到末尾的时候,自动从头开始
				break;
			}
			else if(key==KEY0_PRESS)
			{
				pause=!pause;
				LED1=!pause; 	//暂停的时候LED1亮.  
			}
			if(pause==0)
				i++;
			delay_ms(10); 
		}					    
		res=0;  
	} 							    
	myfree(SRAMIN,picfileinfo);			//释放内存						   		    
	myfree(SRAMIN,pname);				//释放内存			    
	myfree(SRAMIN,picoffsettbl);			//释放内存
}
