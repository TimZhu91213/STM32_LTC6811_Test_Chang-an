#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"	  
#include "spi.h"
#include "LTC68041.h"
 
/************************************************
 ALIENTEK战舰STM32开发板实验24
 SPI 实验   
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/

 				 	
uint16_t    cell_codes[1][12];
unsigned char TOTAL_IC=1;

 int main(void)
 {	 
	u8 i=0;
	delay_init();	    	 //延时函数初始化	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);	 	//串口初始化为115200
	LED_Init();		  		//初始化与LED连接的硬件接口	 	 	
  SPI1_Init();		   	//初始化SPI
	 LTC6804_initialize();
	while(1)
	{
		wakeup_sleep();
  	//LTC6804_adcvax();
		delay_ms(50);
		LTC6804_rdcv(0, TOTAL_IC, cell_codes);
  	printf("V1:%fv \r\n",(float)cell_codes[0][0]/10000);
		printf("V2:%fv \r\n",(float)cell_codes[0][1]/10000);
		printf("V3:%fv \r\n",(float)cell_codes[0][2]/10000);
		printf("V4:%fv \r\n",(float)cell_codes[0][6]/10000);
		printf("V5:%fv \r\n",(float)cell_codes[0][7]/10000);
		printf("\r\n");

//		
		LED0=!LED0;//DS0闪烁
	}

//	{
//		key=KEY_Scan(0);
//		if(key==KEY1_PRES)	//KEY1按下,写入W25QXX
//		{
//			LCD_Fill(0,170,239,319,WHITE);//清除半屏    
// 			LCD_ShowString(30,170,200,16,16,"Start Write W25Q128...."); 
//			W25QXX_Write((u8*)TEXT_Buffer,FLASH_SIZE-100,SIZE);			//从倒数第100个地址处开始,写入SIZE长度的数据
//			LCD_ShowString(30,170,200,16,16,"W25Q128 Write Finished!");	//提示传送完成
//		}
//		if(key==KEY0_PRES)	//KEY0按下,读取字符串并显示
//		{
// 			LCD_ShowString(30,170,200,16,16,"Start Read W25Q128.... ");
//			W25QXX_Read(datatemp,FLASH_SIZE-100,SIZE);					//从倒数第100个地址处开始,读出SIZE个字节
//			LCD_ShowString(30,170,200,16,16,"The Data Readed Is:  ");	//提示传送完成
//			LCD_ShowString(30,190,200,16,16,datatemp);//显示读到的字符串
//		}
//		i++;
//		delay_ms(10);
//		if(i==20)
//		{
//			LED0=!LED0;//提示系统正在运行	
//			i=0;
//		}		   
//	}
}
 

