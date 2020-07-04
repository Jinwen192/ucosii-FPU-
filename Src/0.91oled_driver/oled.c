#include "oled.h"
#include "ascii.h"
#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_i2c.h"
#include "oled.h"
#include "includes.h"
void WriteCmd(unsigned char I2C_Command)//写命令
 {

		HAL_I2C_Mem_Write(&hi2c1,OLED0561_ADD,COM,I2C_MEMADD_SIZE_8BIT,&I2C_Command,1,100);
	
 }
		
void WriteDat(unsigned char I2C_Data)//写数据
 
 {

		HAL_I2C_Mem_Write(&hi2c1,OLED0561_ADD,DAT,I2C_MEMADD_SIZE_8BIT,&I2C_Data,1,100);
	
  }
 
  /*0.96寸
void OLED_Init(void)
{
	HAL_Delay(100); //这里的延时很重要
	
	WriteCmd(0xAE); //display off
	WriteCmd(0x20);	//Set Memory Addressing Mode	
	WriteCmd(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCmd(0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
	WriteCmd(0xc8);	//Set COM Output Scan Direction
	WriteCmd(0x00); //---set low column address
	WriteCmd(0x10); //---set high column address
	WriteCmd(0x40); //--set start line address
	WriteCmd(0x81); //--set contrast control register
	WriteCmd(0xff); //亮度调节 0x00~0xff
	WriteCmd(0xa1); //--set segment re-map 0 to 127
	WriteCmd(0xa6); //--set normal display
	WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
	WriteCmd(0x3F); //
	WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	WriteCmd(0xd3); //-set display offset
	WriteCmd(0x00); //-not offset
	WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
	WriteCmd(0xf0); //--set divide ratio
	WriteCmd(0xd9); //--set pre-charge period
	WriteCmd(0x22); //
	WriteCmd(0xda); //--set com pins hardware configuration
	WriteCmd(0x12);
	WriteCmd(0xdb); //--set vcomh
	WriteCmd(0x20); //0x20,0.77xVcc
	WriteCmd(0x8d); //--set DC-DC enable
	WriteCmd(0x14); //
	WriteCmd(0xaf); //--turn on oled panel
}
*/

//0.91寸
void OLED_Init(void)
{
	//OSTimeDlyHMSM(0,0,0,100);//初始化之前的延时 important
	//初始化指令 由厂商给定
	WriteCmd(0xAE);//display off
	WriteCmd(0xD5);//set memory addressing Mode
	WriteCmd(0X80);//分频因子
	WriteCmd(0xA8);//设置驱动路数
	WriteCmd(0x1F);//默认0X3f(1/64) 0x1f(1/32)
	WriteCmd(0xD3); //设置显示偏移
	WriteCmd(0x00);//默认值00
		
	WriteCmd(0x40);//设置开始行 【5:0】，行数
	
	WriteCmd(0x8D);//电荷泵设置
	WriteCmd(0x14);//bit2,开启/关闭
	
	WriteCmd(0x20);//设置内存地址模式
	WriteCmd(0x02);//[[1:0],00，列地址模式;01，行地址模式;10,页地址模式;默认10;
	WriteCmd(0xA1);//段重定义设置,bit0:0,0->0;1,0->127;
	WriteCmd(0xC8);//设置COM扫描方向
	
	WriteCmd(0xDA);//设置COM硬件引脚配置
	WriteCmd(0x02);//0.91英寸128*32分辨率
	
	WriteCmd(0x81);//对比度设置
	WriteCmd(0x8f);//1~255(亮度设置,越大越亮)
	
	WriteCmd(0xD9);//设置预充电周期
	WriteCmd(0xf1);//[3:0],PHASE 1;[7:4],PHASE 2;
	WriteCmd(0xDB);//设置VCOMH 电压倍率
	WriteCmd(0x40);//[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

	WriteCmd(0xA4);//全局显示开启;bit0:1,开启;0,关闭;(白屏/黑屏)
	WriteCmd(0xA6);//设置显示方式;bit0:1,反相显示;0,正常显示

	WriteCmd(0x2E);//停用滚动条

	WriteCmd(0xAF);//开启显示
	OSTimeDlyHMSM(0,0,0,100);//延时一段时间
	OLED_CLS();//清除显示屏幕，防止屏幕中存在噪点
}

void OLED_SetPos(unsigned char x, unsigned char y) //设置起始点坐标
{ 
	WriteCmd(0xb0+y);
	WriteCmd(((x&0xf0)>>4)|0x10);
	WriteCmd((x&0x0f)|0x01);
}

void OLED_Fill(unsigned char fill_Data)//全屏填充
{
	unsigned char m,n;
	for(m=0;m<8;m++)
	{
		WriteCmd(0xb0+m);		//page0-page1
		WriteCmd(0x00);		//low column start address
		WriteCmd(0x10);		//high column start address
		for(n=0;n<128;n++)
			{
				WriteDat(fill_Data);
			}
	}
}


void OLED_CLS(void)//清屏
{
	OLED_Fill(0x00);
}

void OLED_ON(void)
{
	WriteCmd(0X8D);  //设置电荷泵
	WriteCmd(0X14);  //开启电荷泵
	WriteCmd(0XAF);  //OLED唤醒
}

void OLED_OFF(void)
{
	WriteCmd(0X8D);  //设置电荷泵
	WriteCmd(0X10);  //关闭电荷泵
	WriteCmd(0XAE);  //OLED休眠
}


// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); ch[] -- 要显示的字符串; TextSize -- 字符大小(1:6*8 ; 2:8*16)
// Description    : 显示codetab.h中的ASCII字符,有6*8和8*16可选择
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
{
	unsigned char c = 0,i = 0,j = 0;
	switch(TextSize)
	{
		case 1:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 126)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<6;i++)
					WriteDat(F6x8[c][i]);
				x += 6;
				j++;
			}
		}break;
		case 2:
		{
			while(ch[j] != '\0')
			{
				c = ch[j] - 32;
				if(x > 120)
				{
					x = 0;
					y++;
				}
				OLED_SetPos(x,y);
				for(i=0;i<8;i++)
					WriteDat(F8X16[c*16+i]);
				OLED_SetPos(x,y+1);
				for(i=0;i<8;i++)
					WriteDat(F8X16[c*16+i+8]);
				x += 8;
				j++;
			}
		}break;
	}
}


// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); N:汉字在.h中的索引
// Description    : 显示ASCII_8x16.h中的汉字,16*16点阵
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
{
	unsigned char wm=0;
	unsigned int  adder=32*N;
	OLED_SetPos(x , y);
	for(wm = 0;wm < 16;wm++)
	{
		WriteDat(F16x16[adder]);
		adder += 1;
	}
	OLED_SetPos(x,y + 1);
	for(wm = 0;wm < 16;wm++)
	{
		WriteDat(F16x16[adder]);
		adder += 1;
	}
}



// Parameters     : x0,y0 -- 起始点坐标(x0:0~127, y0:0~7); x1,y1 -- 起点对角线(结束点)的坐标(x1:1~128,y1:1~8)
// Description    : 显示BMP位图
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[])
{
	unsigned int j=0;
	unsigned char x,y;

  if(y1%8==0)
		y = y1/8;
  else
		y = y1/8 + 1;
	for(y=y0;y<y1;y++)
	{
		OLED_SetPos(x0,y);
    for(x=x0;x<x1;x++)
		{
			WriteDat(BMP[j++]);
		}
	}
}






void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t Char_Size)
{      	
	unsigned char c=0,i=0;	
		c=chr-' ';//得到偏移后的值			
		if(x>128-1){x=0;y=y+2;}
		if(Char_Size ==16)
			{
			OLED_SetPos(x,y);	
			for(i=0;i<8;i++)
			WriteDat(F8X16[c*16+i]);
			OLED_SetPos(x,y+1);
			for(i=0;i<8;i++)
			WriteDat(F8X16[c*16+i+8]);
			}
			else {	
				OLED_SetPos(x,y);
				for(i=0;i<6;i++)
				WriteDat(F6x8[c][i]);
				
			}
}

uint32_t oled_pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;	 
	while(n--)result*=m;    
	return result;
}	


//显示2个数字
//x,y :起点坐标	 
//len :数字的位数
//size:字体大小
//mode:模式	0,填充模式;1,叠加模式
//num:数值(0~4294967295);	 		  
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size2)
{         	
	uint8_t t,temp;
	uint8_t enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/oled_pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				OLED_ShowChar(x+(size2/2)*t,y,' ',size2);
				continue;
			}else enshow=1; 
		 	 
		}
	 	OLED_ShowChar(x+(size2/2)*t,y,temp+'0',size2); 
	}
} 


//开发板实际测得参考电压为2.8V , 因为单颗锂电池只有4V左右，对于3.3V的稳压芯片，不能为它提供5V电压。
//接着，通过两个5.1k进行分压。满电时候，理论上，AD输入端为2V (4V/2)
//我们将3.5V定义为电池无电状态，此时，AD输入端测得1.75V.(3.5V/2)
//oled电池的显示状态分为 13级，每一级 (2000-1750)/14 = 17.86mv
void OLED_ShowBat (unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1, uint16_t volt) 
{
	static uint16_t sum_volt=0;
	static uint8_t cnt;
	uint16_t low_limt = 1750 ; 
	unsigned char level;
	
	if (cnt++ < 16){
		sum_volt += volt;
	}else{
		sum_volt = (sum_volt>>4);
		level = (sum_volt - low_limt) / 17.86;
		if (level > 13) level = 13;
		if (level <  0) level = 0;
		OLED_DrawBMP(x0,y0,x1,y1, bat[level]);
		cnt = 0;
		sum_volt = 0;
	}
}