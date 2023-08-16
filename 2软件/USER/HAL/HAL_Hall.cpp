#include "HAL.h"
#include "exti.h"
#include "timer.h"
#include "Arduino.h"

double Hall_time=0;		//����������ʱ��	��λΪs
double Short_time=0;	//��������ϸ΢ʱ��	��λΪs
extern uint32_t Hall_second;		//������������	��λΪs
double Circle_radius=0.5;				//Բ�뾶	��λΪm
double Circle_cfe=0;					//Բ�ܳ�	��λΪm
double Circle_speed = 0;			//�ٶ� 	��λΪm/s
//���������ⲿ�жϷ�����
static void Hall_EventHandler()
{
	//��ȡʱ��	
		Short_time = TMR_GetCounter(TIM3);
		Short_time = Short_time*0.0001;
		Hall_time=  Hall_second+Short_time;
	
	//�����ܳ��Լ��ٶ�
		Circle_cfe = 2*PI*Circle_radius;
		Circle_speed = Circle_cfe/Hall_time;
	
	//��ӡʱ��
		printf("Short_time: %f s\r\n",Short_time);
		printf("Hall_time: %f s\r\n",Hall_time);
		printf("Circle_speed: %f s\r\n",Circle_speed);
	
		Short_time =0;
		Hall_second=0;
		TIM_Cmd(CONFIG_HALL_SWITCH_TIM, DISABLE);
	
		//������һ�μ���
		TIM_Cmd(CONFIG_HALL_SWITCH_TIM, ENABLE);
}
//�������س�ʼ��
void HAL::Hall_switch_Init()
{
	pinMode(CONFIG_HALL_SWITCH_PIN, INPUT_PULLDOWN);
	
	attachInterrupt(CONFIG_HALL_SWITCH_PIN, Hall_EventHandler , CHANGE);
	
}

//LED0��ʼ��
void HAL::LED_Init()
{
	pinMode(CONFIG_LED_PIN,OUTPUT);
}

//LED0��
void HAL::LED_Open()
{
	digitalWrite(CONFIG_LED_PIN,0);
}

//LED0�ر�
void HAL::LED_Close()
{
	digitalWrite(CONFIG_LED_PIN,1);
}