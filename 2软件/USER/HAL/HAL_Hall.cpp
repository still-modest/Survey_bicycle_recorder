#include "HAL.h"
#include "exti.h"
#include "timer.h"
#include "Arduino.h"

double Hall_time=0;		//霍尔开关总时间	单位为s
double Short_time=0;	//霍尔开关细微时间	单位为s
extern uint32_t Hall_second;		//霍尔开关秒数	单位为s
double Circle_radius=0.5;				//圆半径	单位为m
double Circle_cfe=0;					//圆周长	单位为m
double Circle_speed = 0;			//速度 	单位为m/s
//霍尔开关外部中断服务函数
static void Hall_EventHandler()
{
	//获取时间	
		Short_time = TMR_GetCounter(TIM3);
		Short_time = Short_time*0.0001;
		Hall_time=  Hall_second+Short_time;
	
	//计算周长以及速度
		Circle_cfe = 2*PI*Circle_radius;
		Circle_speed = Circle_cfe/Hall_time;
	
	//打印时间
		printf("Short_time: %f s\r\n",Short_time);
		printf("Hall_time: %f s\r\n",Hall_time);
		printf("Circle_speed: %f s\r\n",Circle_speed);
	
		Short_time =0;
		Hall_second=0;
		TIM_Cmd(CONFIG_HALL_SWITCH_TIM, DISABLE);
	
		//开启另一次计数
		TIM_Cmd(CONFIG_HALL_SWITCH_TIM, ENABLE);
}
//霍尔开关初始化
void HAL::Hall_switch_Init()
{
	pinMode(CONFIG_HALL_SWITCH_PIN, INPUT_PULLDOWN);
	
	attachInterrupt(CONFIG_HALL_SWITCH_PIN, Hall_EventHandler , CHANGE);
	
}

//LED0初始化
void HAL::LED_Init()
{
	pinMode(CONFIG_LED_PIN,OUTPUT);
}

//LED0打开
void HAL::LED_Open()
{
	digitalWrite(CONFIG_LED_PIN,0);
}

//LED0关闭
void HAL::LED_Close()
{
	digitalWrite(CONFIG_LED_PIN,1);
}