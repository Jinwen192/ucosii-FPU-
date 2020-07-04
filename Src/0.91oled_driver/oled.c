#include "oled.h"
#include "ascii.h"
#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_i2c.h"
#include "oled.h"
#include "includes.h"
void WriteCmd(unsigned char I2C_Command)//д����
 {

		HAL_I2C_Mem_Write(&hi2c1,OLED0561_ADD,COM,I2C_MEMADD_SIZE_8BIT,&I2C_Command,1,100);
	
 }
		
void WriteDat(unsigned char I2C_Data)//д����
 
 {

		HAL_I2C_Mem_Write(&hi2c1,OLED0561_ADD,DAT,I2C_MEMADD_SIZE_8BIT,&I2C_Data,1,100);
	
  }
 
  /*0.96��
void OLED_Init(void)
{
	HAL_Delay(100); //�������ʱ����Ҫ
	
	WriteCmd(0xAE); //display off
	WriteCmd(0x20);	//Set Memory Addressing Mode	
	WriteCmd(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	WriteCmd(0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
	WriteCmd(0xc8);	//Set COM Output Scan Direction
	WriteCmd(0x00); //---set low column address
	WriteCmd(0x10); //---set high column address
	WriteCmd(0x40); //--set start line address
	WriteCmd(0x81); //--set contrast control register
	WriteCmd(0xff); //���ȵ��� 0x00~0xff
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

//0.91��
void OLED_Init(void)
{
	//OSTimeDlyHMSM(0,0,0,100);//��ʼ��֮ǰ����ʱ important
	//��ʼ��ָ�� �ɳ��̸���
	WriteCmd(0xAE);//display off
	WriteCmd(0xD5);//set memory addressing Mode
	WriteCmd(0X80);//��Ƶ����
	WriteCmd(0xA8);//��������·��
	WriteCmd(0x1F);//Ĭ��0X3f(1/64) 0x1f(1/32)
	WriteCmd(0xD3); //������ʾƫ��
	WriteCmd(0x00);//Ĭ��ֵ00
		
	WriteCmd(0x40);//���ÿ�ʼ�� ��5:0��������
	
	WriteCmd(0x8D);//��ɱ�����
	WriteCmd(0x14);//bit2,����/�ر�
	
	WriteCmd(0x20);//�����ڴ��ַģʽ
	WriteCmd(0x02);//[[1:0],00���е�ַģʽ;01���е�ַģʽ;10,ҳ��ַģʽ;Ĭ��10;
	WriteCmd(0xA1);//���ض�������,bit0:0,0->0;1,0->127;
	WriteCmd(0xC8);//����COMɨ�跽��
	
	WriteCmd(0xDA);//����COMӲ����������
	WriteCmd(0x02);//0.91Ӣ��128*32�ֱ���
	
	WriteCmd(0x81);//�Աȶ�����
	WriteCmd(0x8f);//1~255(��������,Խ��Խ��)
	
	WriteCmd(0xD9);//����Ԥ�������
	WriteCmd(0xf1);//[3:0],PHASE 1;[7:4],PHASE 2;
	WriteCmd(0xDB);//����VCOMH ��ѹ����
	WriteCmd(0x40);//[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

	WriteCmd(0xA4);//ȫ����ʾ����;bit0:1,����;0,�ر�;(����/����)
	WriteCmd(0xA6);//������ʾ��ʽ;bit0:1,������ʾ;0,������ʾ

	WriteCmd(0x2E);//ͣ�ù�����

	WriteCmd(0xAF);//������ʾ
	OSTimeDlyHMSM(0,0,0,100);//��ʱһ��ʱ��
	OLED_CLS();//�����ʾ��Ļ����ֹ��Ļ�д������
}

void OLED_SetPos(unsigned char x, unsigned char y) //������ʼ������
{ 
	WriteCmd(0xb0+y);
	WriteCmd(((x&0xf0)>>4)|0x10);
	WriteCmd((x&0x0f)|0x01);
}

void OLED_Fill(unsigned char fill_Data)//ȫ�����
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


void OLED_CLS(void)//����
{
	OLED_Fill(0x00);
}

void OLED_ON(void)
{
	WriteCmd(0X8D);  //���õ�ɱ�
	WriteCmd(0X14);  //������ɱ�
	WriteCmd(0XAF);  //OLED����
}

void OLED_OFF(void)
{
	WriteCmd(0X8D);  //���õ�ɱ�
	WriteCmd(0X10);  //�رյ�ɱ�
	WriteCmd(0XAE);  //OLED����
}


// Parameters     : x,y -- ��ʼ������(x:0~127, y:0~7); ch[] -- Ҫ��ʾ���ַ���; TextSize -- �ַ���С(1:6*8 ; 2:8*16)
// Description    : ��ʾcodetab.h�е�ASCII�ַ�,��6*8��8*16��ѡ��
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


// Parameters     : x,y -- ��ʼ������(x:0~127, y:0~7); N:������.h�е�����
// Description    : ��ʾASCII_8x16.h�еĺ���,16*16����
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



// Parameters     : x0,y0 -- ��ʼ������(x0:0~127, y0:0~7); x1,y1 -- ���Խ���(������)������(x1:1~128,y1:1~8)
// Description    : ��ʾBMPλͼ
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
		c=chr-' ';//�õ�ƫ�ƺ��ֵ			
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


//��ʾ2������
//x,y :�������	 
//len :���ֵ�λ��
//size:�����С
//mode:ģʽ	0,���ģʽ;1,����ģʽ
//num:��ֵ(0~4294967295);	 		  
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


//������ʵ�ʲ�òο���ѹΪ2.8V , ��Ϊ����﮵��ֻ��4V���ң�����3.3V����ѹоƬ������Ϊ���ṩ5V��ѹ��
//���ţ�ͨ������5.1k���з�ѹ������ʱ�������ϣ�AD�����Ϊ2V (4V/2)
//���ǽ�3.5V����Ϊ����޵�״̬����ʱ��AD����˲��1.75V.(3.5V/2)
//oled��ص���ʾ״̬��Ϊ 13����ÿһ�� (2000-1750)/14 = 17.86mv
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