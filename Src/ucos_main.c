#include "ucos_main.h"
#include "includes.h"
#include "gpio.h"
#include "main.h"
#include "oled.h"
#include "fatfs.h"
#include "string.h"
#include "roller.h"
#include "adc.h"
#include "usart.h"

//任务优先级从5~30，分散分配

//START 任务
//设置任务优先级
#define START_TASK_PRIO			30  ///开始任务的优先级为最低
#define START_STK_SIZE			128
OS_STK START_TASK_STK[START_STK_SIZE];
void start_task(void *pdata);


//LED1任务
//设置任务优先级
#define LED1_TASK_PRIO			20
#define LED1_STK_SIZE			128
OS_STK LED1_TASK_STK[LED1_STK_SIZE];
void led1_task(void *pdata);

//LED1任务
//设置任务优先级
#define LED2_TASK_PRIO			21
#define LED2_STK_SIZE			128
OS_STK LED2_TASK_STK[LED2_STK_SIZE];
void led2_task(void *pdata);

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

//KEY任务
#define KEY_TASK_PRIO			15
#define KEY_STK_SIZE			128
OS_STK KEY_TASK_STK[KEY_STK_SIZE] ;
void key_task(void  *pdata);

//发送串口任务
#define UART_TASK_PRIO			7
#define UART_STK_SIZE			128
OS_STK UART_TASK_STK[UART_STK_SIZE] ;
void uart_task(void  *pdata);

void *MsgGrp[128];
OS_EVENT *ad_box;
OS_EVENT *roller_box;
OS_EVENT *roller_uart_box;
OS_EVENT *key_sd_sem;
OS_EVENT *SD_signal_sem;
OS_EVENT *uart_sem;

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
	HAL_GPIO_WritePin (LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
	//创建LED1任务，用于提示单片机正常运行
    OSTaskCreate(led1_task,
           	     (void*)0,
				  (OS_STK*)&LED1_TASK_STK[LED1_STK_SIZE-1],
				  LED1_TASK_PRIO);

	//0.91寸OLED，用于显示电量和位移距离值			 
    OSTaskCreate(oled_task,
           	     (void*)0,
				  (OS_STK*)&OLED_TASK_STK[OLED_STK_SIZE-1],
				  OLED_TASK_PRIO);	
				 
	//SD卡任务，用于保存位移距离值			 
	OSTaskCreate(SD_task,
           	     (void*)0,
				  (OS_STK*)&SD_TASK_STK[SD_STK_SIZE-1],
				  SD_TASK_PRIO);	
				 
	//滚动编码器任务，用于获取信号
	OSTaskCreate(roller_task,
           	     (void*)0,
				  (OS_STK*)&roller_TASK_STK[roller_STK_SIZE-1],
				  roller_TASK_PRIO);
	
	//AD转换任务，用于读取电压
	OSTaskCreate(AD_task,
           	     (void*)0,
				  (OS_STK*)&AD_TASK_STK[AD_STK_SIZE-1],
				  AD_TASK_PRIO);	
				 
	//串口1任务，用于打印位移距离			 
	OSTaskCreate(uart_task,
           	     (void*)0,
				  (OS_STK*)&UART_TASK_STK[UART_STK_SIZE-1],
				  UART_TASK_PRIO);	
	
	//按键任务，用于选择打印位移距离方式
	OSTaskCreate(key_task,
			 (void*)0,
			  (OS_STK*)&KEY_TASK_STK[KEY_STK_SIZE-1],
			  KEY_TASK_PRIO);
		 
	OSTaskDel(START_TASK_PRIO);
	OS_EXIT_CRITICAL();
				 
}
//LED0任务:运行指示灯
void led1_task(void *pdata)
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

//LED1任务，单次执行，闪烁10次
void led2_task(void *pdata)
{	
	int i=10;
	pdata = pdata;
	HAL_GPIO_WritePin (LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
	while (i--)
	{
		HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
		OSTimeDlyHMSM(0,0,0,200);
	}
	HAL_GPIO_WritePin (LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
	OSTaskDel (LED2_TASK_PRIO);
}

//oled显示任务
void oled_task (void *pdata)
{
	uint16_t volt;
	float 	 s_volt;
	uint16_t AD_value;
	uint8_t str[20] = {0};
	void *msg;

	
	OLED_Init();
	
	pdata = pdata;
	while (1)
	{
		msg = OSMboxAccept (roller_box);			//无阻塞等待滚动编码器发送的邮箱
		if (msg!=(void *)0)
			show_roller ((struct Roller *)msg);
		
		msg = OSMboxAccept (ad_box);				//无阻塞等待电压采集任务发送的邮箱
		if (msg!=(void*)0)
		{
			AD_value = *(uint16_t*)msg;
			volt = (AD_value * 2600)/4096;
			s_volt = (volt*2.0)/1000;
			
			sprintf ((char*)&str, "%0.2fV", s_volt);
			OLED_ShowStr (90,3,str,1);				//显示实际电压值
			
			OLED_ShowBat (107,0,123,1,volt);		//显示电量图形
		}

		OSTimeDlyHMSM(0,0,0,100);
	}
}

//SD卡任务
void SD_task (void *pdata)
{
	FRESULT f_res;
	FATFS fs;
	FIL fd;
	UINT fnum;
	uint8_t err;

	char WriteBuffer[10] = {0};

	
	void *msg;
	struct Roller* roller = NULL;
	
	pdata = pdata;
	
	f_res = f_mount (&fs, (TCHAR const*)SDPath, 1);		//挂载SD卡
	OSTimeDlyHMSM(0,0,0,100);
	while (1)
	{
			OSSemPend (key_sd_sem,0, &err);			//等待按键释放信号量
			msg = OSMboxAccept (roller_box);		//无阻塞获取滚动编码器发送的邮箱
			if (msg != (void*)0)
				roller = (struct Roller*)msg;
			
			//打开文件
			f_res = f_open(&fd, "distance.txt",   FA_OPEN_ALWAYS |FA_WRITE |FA_READ);
			if ( f_res != FR_OK ){		//打开文件失败，LED2一直亮，需要复位
				f_close(&fd);
				while (1){													//打开文件失败，需要复位
					HAL_GPIO_WritePin (LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
					OSTimeDlyHMSM(0,0,0,100);
				}			
			}
			f_lseek(&fd, f_size(&fd));		//文件尾部追加
			
			sprintf (WriteBuffer, "%0.2fcm   ",roller->distance );	//在OLED上显示要保存的位移距离值
			OLED_ShowStr (0,3,(uint8_t*)WriteBuffer,1);
			
			
			sprintf (WriteBuffer, "%0.2fcm\n",roller->distance );	
			f_res=f_write(&fd,WriteBuffer,strlen(WriteBuffer),&fnum);	//往文件写入位移值
			if(f_res==FR_OK)			//成功，LED1闪烁10下									
			{	
				OSTaskCreate(led2_task,
							(void*)0,
							(OS_STK*)&LED2_TASK_STK[LED2_STK_SIZE-1],
							 LED2_TASK_PRIO);
			}
			else
			{
				f_close(&fd);		//打开文件失败，需要复位	
				while (1){													
					//HAL_GPIO_WritePin (LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
					OSTimeDlyHMSM(0,0,0,100);
				}
			}
			f_sync (&fd);	
			f_close(&fd);
	}
}


//滚动编码器任务
void roller_task(void  *pdata)
{
	roller_box = OSMboxCreate ((void*)0);
	roller_uart_box = OSMboxCreate ((void*)0);
	struct Roller roller;
	
	roller_init (&roller);
	
	pdata = pdata;
	while (1)
	{	
		get_roller_stat (&roller);					//获取滚动编码器状态
		run_roller (&roller);						//判断编码前一次与这次的读取的状态，从而得知道正转与反转
		OSMboxPost (roller_box,(void*)&roller);		//给OLED发送邮箱
		OSMboxPost (roller_uart_box,(void*)&roller);//给串口发送邮箱
		OSTimeDlyHMSM(0,0,0,1);
	}
}

//AD电压采集任务
void AD_task (void *pdata)
{
	uint16_t AD_value;
	ad_box = OSMboxCreate ((void*)0);							//创建邮箱
	pdata = pdata;
	
	while (1)
	{
		HAL_ADC_Stop_DMA(&hadc1);								//暂停AD转换
		HAL_ADC_Start_DMA (&hadc1, (uint32_t*)&AD_value, 1);	//启动AD转换
		OSMboxPost (ad_box, (void*)&AD_value);					//向邮箱发送信息
		OSTimeDlyHMSM(0,0,0,200);
	}
}

//串口任务
void uart_task (void *pdata)
{
	uint8_t err;
	void *msg;
	struct Roller *roller;
	char str[20];
	
	OSTimeDlyHMSM(0,0,0,100);//任务开启，先让出CPU，让其他任务执行，因为uart_sem信号量还没创建。
	while (1)
	{
		OSSemPend (uart_sem, 0, &err);						//等待按键释放信号量
		msg = OSMboxPend (roller_uart_box, 0, &err); 		//等待滚动编码器发送的邮箱
		if (msg != (void*)0)
			roller = (struct Roller*)msg;
		
		sprintf (str,"%.2fcm\n", roller->distance);
		HAL_UART_Transmit_IT(&huart1, (uint8_t *)str, strlen(str));
		//HAL_GPIO_TogglePin (LED2_GPIO_Port,LED2_Pin);
		OSTimeDlyHMSM(0,0,0,100);
	}
	
}

//按键任务
//新建两个信号量，用于信号同步
//按键分别对应SD卡和串口，用于选择打印位移距离
void key_task (void *pdata)
{

	key_sd_sem 	= OSSemCreate (0);		
	uart_sem 	= OSSemCreate (0);		
	
	while (1)
	{	
		if (HAL_GPIO_ReadPin (KEY1_GPIO_Port, KEY1_Pin) == 0)	
		{
			OSTimeDlyHMSM(0,0,0,100);
			if (HAL_GPIO_ReadPin (KEY1_GPIO_Port, KEY1_Pin) == 0)
			{
				OSSemPost (key_sd_sem);		//释放SD信号量
			}
		}
		
		if (HAL_GPIO_ReadPin (KEY2_GPIO_Port, KEY2_Pin) == 0)
		{
			OSTimeDlyHMSM(0,0,0,100);
			if (HAL_GPIO_ReadPin (KEY2_GPIO_Port, KEY2_Pin) == 0)
			{
				
				OSSemPost (uart_sem);		//释放串口信号量
			}
		}
		
		OSTimeDlyHMSM(0,0,0,100);
	}
}


//串口中断任务
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART1)
		OSTaskCreate(led2_task, 					//如果发送成功，LED闪烁10次
					(void*)0,
					(OS_STK*)&LED2_TASK_STK[LED2_STK_SIZE-1],
					 LED2_TASK_PRIO);
}
	
