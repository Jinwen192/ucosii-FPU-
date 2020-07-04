#include "ucos_main.h"
#include "includes.h"
#include "gpio.h"
#include "main.h"
#include "oled.h"
#include "fatfs.h"
#include "string.h"
#include "roller.h"
#include "adc.h"

//任务优先级从5~30，分散分配

//START 任务
//设置任务优先级
#define START_TASK_PRIO			30  ///开始任务的优先级为最低
#define START_STK_SIZE			128
OS_STK START_TASK_STK[START_STK_SIZE];
void start_task(void *pdata);


//LED0任务
//设置任务优先级
#define LED0_TASK_PRIO			20
#define LED0_STK_SIZE			128
OS_STK LED0_TASK_STK[LED0_STK_SIZE];
void led0_task(void *pdata);

//LED1任务
//设置任务优先级
#define LED1_TASK_PRIO			21
#define LED1_STK_SIZE			128
OS_STK LED1_TASK_STK[LED1_STK_SIZE];
void led1_task(void *pdata);

//OLED任务
#define OLED_TASK_PRIO			27
#define OLED_STK_SIZE			128
OS_STK	OLED_TASK_STK[OLED_STK_SIZE];
void oled_task(void  *pdata);

//SD卡读写任务
#define SD_TASK_PRIO			10
#define SD_STK_SIZE				1024
OS_STK	SD_TASK_STK[SD_STK_SIZE];
void SD_task(void  *pdata);

//滚动编码器任务
#define roller_TASK_PRIO		5
#define roller_STK_SIZE			1024
__align(8) static OS_STK roller_TASK_STK[roller_STK_SIZE] ;
void roller_task(void  *pdata);


//AD采集电源任务
#define AD_TASK_PRIO			25
#define AD_STK_SIZE				1024
OS_STK AD_TASK_STK[AD_STK_SIZE] ;
void AD_task(void  *pdata);


void *MsgGrp[128];
OS_EVENT *ad_box;
OS_EVENT *roller_box;




void ucos_main (void)
{
	OSInit();
	OSTaskCreateExt((void(*)(void*) )start_task,                //任务函数
                    (void*          )0,                         //传递给任务函数的参数
                    (OS_STK*        )&START_TASK_STK[START_STK_SIZE-1],//任务堆栈栈顶
                    (INT8U          )START_TASK_PRIO,           //任务优先级
                    (INT16U         )START_TASK_PRIO,           //任务ID，这里设置为和优先级一样
                    (OS_STK*        )&START_TASK_STK[0],        //任务堆栈栈底
                    (INT32U         )START_STK_SIZE,            //任务堆栈大小
                    (void*          )0,                         //用户补充的存储区
                    (INT16U         )OS_TASK_OPT_STK_CHK|OS_TASK_OPT_STK_CLR|OS_TASK_OPT_SAVE_FP);//任务选项,为了保险起见，所有任务都保存浮点寄存器的值
	OSStart(); //开始任务
	
}

void start_task(void *pdata)
{
	pdata = pdata;
	OS_CPU_SR cpu_sr=0;
	OS_ENTER_CRITICAL();
	
//    OSTaskCreate(led0_task,
//           	     (void*)0,
//				  (OS_STK*)&LED0_TASK_STK[LED0_STK_SIZE-1],
//				  LED0_TASK_PRIO);
				 
    OSTaskCreate(led1_task,
           	     (void*)0,
				  (OS_STK*)&LED1_TASK_STK[LED1_STK_SIZE-1],
				  LED1_TASK_PRIO);
				 
    OSTaskCreate(oled_task,
           	     (void*)0,
				  (OS_STK*)&OLED_TASK_STK[LED0_STK_SIZE-1],
				  OLED_TASK_PRIO);	
				 
//	OSTaskCreate(SD_task,
//           	     (void*)0,
//				  (OS_STK*)&SD_TASK_STK[SD_STK_SIZE-1],
//				  SD_TASK_PRIO);	
	
	OSTaskCreate(roller_task,
           	     (void*)0,
				  (OS_STK*)&roller_TASK_STK[roller_STK_SIZE-1],
				  roller_TASK_PRIO);
				 		
	OSTaskCreate(AD_task,
           	     (void*)0,
				  (OS_STK*)&AD_TASK_STK[AD_STK_SIZE-1],
				  AD_TASK_PRIO);	
				 
	OSTaskDel(START_TASK_PRIO);
	OS_EXIT_CRITICAL();
				 
}
 //LED0任务
void led0_task(void *pdata)
{	 	
	pdata = pdata;
	while(1)
	{
		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
		OSTimeDlyHMSM(0,0,0,500);
		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
		OSTimeDlyHMSM(0,0,0,500);
	};
}

//LED1任务
void led1_task(void *pdata)
{	  
	pdata = pdata;
	while(1)
	{
		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
		OSTimeDlyHMSM(0,0,0,500);
		HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
		OSTimeDlyHMSM(0,0,0,500);
	}
}

//oled显示任务
void oled_task (void *pdata)
{
	INT8U err;
	uint16_t volt;
	uint8_t str[20];
	uint16_t AD_value;
	
	void *msg;

	
	OLED_Init();
	pdata = pdata;
	while (1)
	{
		//msg = OSMboxPend (roller_box,0,&err);
		msg = OSMboxAccept (roller_box);
		if (msg!=(void *)0)
			show_roller ((struct Roller *)msg);
		
		msg = OSMboxAccept (ad_box);
		if (msg!=(void*)0)
		{
			AD_value = *(uint16_t*)msg;
			volt = (AD_value * 2800)/4096;
			sprintf ((char*)&str, "%d", volt);
			OLED_ShowBat (107,0,123,1,volt);
		}
		
		//HAL_GPIO_TogglePin (LED1_GPIO_Port,LED1_Pin);

		OSTimeDlyHMSM(0,0,0,100);
	}
}

void SD_task (void *pdata)
{
	FRESULT f_res;
	FATFS fs;
	FIL fd;
	UINT fnum;

	uint8_t str[50] = {0};
	BYTE WriteBuffer[] = "1234";
	BYTE ReadBuffer[50] = {0};
	uint8_t count = 10;
	uint8_t read_len;
	
	pdata = pdata;
	OLED_Init();
	
	f_res = f_mount (&fs, (TCHAR const*)SDPath, 1);
	f_res = f_open(&fd, "zhou.txt",   FA_CREATE_ALWAYS|FA_WRITE | FA_READ);
    if ( f_res == FR_OK )
	{
		  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
		  strcmp ((char*)str, "Opened file suc");
		  OLED_ShowStr (0,0, str,1);
		  //OSTimeDlyHMSM(0,0,1,0);
	}
	else
	{
		  strcmp ((char*)str, "Open fail");
		  OLED_ShowStr (0,0, str,1);	
		  //OSTimeDlyHMSM(0,0,1,0);		
	}
	
	while (count--)
	{
		  f_res=f_write(&fd,WriteBuffer,sizeof(WriteBuffer),&fnum);
		  if(f_res==FR_OK)
		  {	
				//strcmp ((char*)str, "Written suc");
			   char *ch = "write OK";
				OLED_ShowStr (0,0, (uint8_t*)ch,1);	  
			   //OSTimeDlyHMSM(0,0,1,0);
		  }
		  else
		  {
				char *ch = "write Failed";
				OLED_ShowStr (0,0, (uint8_t*)ch,1);
				//OSTimeDlyHMSM(0,0,1,0);			  
		  }
	}
	f_sync (&fd);
	OSTimeDlyHMSM(0,0,0,500);	
	f_lseek(&fd,0);
	f_res = f_read (&fd, ReadBuffer, 20, (UINT*)&read_len);
	if (f_res == FR_OK)
	{
		char *ch = "read OK   ";
		OLED_ShowStr (0,0, (uint8_t*)ch,1);
		
	}
	else
	{
		sprintf ((char*)str, "f_res:%d      ",f_res);
		OLED_ShowStr (0,0, (uint8_t*)str,1);
	}
	
	//sprintf ((char*)ReadBuffer,"len:%d   ", read_len);
	
	//f_gets ((TCHAR*)ReadBuffer,512,&fd);
	OLED_ShowStr (0,2, (uint8_t*)ReadBuffer,1);  //读不出数据
	
	f_close(&fd);
	
	OSTaskDel(SD_TASK_PRIO);
}


//滚动编码器任务
void roller_task(void  *pdata)
{
	//Str_Q = OSQCreate(&MsgGrp[0], 128);
	uint8_t err;
	roller_box = OSMboxCreate ((void*)0);
	//Semp = OSMutexCreate (6, &err);
	 struct Roller roller;
	
	//OLED_Init();
	roller_init (&roller);
	
	pdata = pdata;
	while (1)
	{	

		get_roller_stat (&roller);
		run_roller (&roller);
		OSMboxPost (roller_box,(void*)&roller);

		OSTimeDlyHMSM(0,0,0,1);
	}
}

//AD电压采集任务
void AD_task (void *pdata)
{
	uint16_t AD_value;
//	uint16_t volt;
//	uint8_t str[20];
	
	ad_box = OSMboxCreate ((void*)0);

	//HAL_ADC_Start_DMA (&hadc1, (uint32_t*)&AD_value, 1);
	
	pdata = pdata;
	while (1)
	{

		HAL_ADC_Stop_DMA(&hadc1);
		HAL_ADC_Start_DMA (&hadc1, (uint32_t*)&AD_value, 1);
		OSMboxPost (ad_box, (void*)&AD_value);
		
		OSTimeDlyHMSM(0,0,0,100);
	}
}




