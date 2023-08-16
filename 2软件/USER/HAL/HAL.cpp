#include "HAL.h"
#include "timer.h"
#include "App/Version.h"

uint32_t Hall_second=0;

static void HAL_InterrputUpdate()
{
    HAL::Power_Update();
    HAL::Encoder_Update();
    HAL::Audio_Update();
}
//霍尔开关
static void HALL_SWITCH_InterrputUpdate()
{
	//时间秒数加一
	Hall_second= Hall_second+1;
	uint8_t temp=digitalRead(CONFIG_LED_PIN);
	if(temp==1)
	{
		HAL::LED_Open();
	}
	else if(temp==0)
	{
		HAL::LED_Close();
	}
	
	//清除LINE3上的中断标志位 
	TMR_ClearFlag(CONFIG_HALL_SWITCH_TIM, TMR_FLAG_Update);
}

#if CONFIG_SENSOR_ENABLE
static void HAL_Sensor_Init()
{
    HAL::I2C_Scan(true);

#if CONFIG_SENSOR_IMU_ENABLE
    HAL::IMU_Init();
#endif

#if CONFIG_SENSOR_MAG_ENABLE
    HAL::MAG_Init();
#endif
}
#endif

void HAL::HAL_Init()
{
    Serial.begin(115200);
    Serial.println(VERSION_FIRMWARE_NAME);
    Serial.println("Version: " VERSION_SOFTWARE);
    Serial.println("Author: " VERSION_AUTHOR_NAME);
    Serial.println("Project: " VERSION_PROJECT_LINK);
    
    FaultHandle_Init();
    
    Memory_DumpInfo();

    Power_Init();
    Backlight_Init();
    Encoder_Init();
    Clock_Init();
    Buzz_init();
    GPS_Init();
		/*霍尔开关初始化*/
		Hall_switch_Init();
		/*LED0初始化*/
		LED_Init();
	
#if CONFIG_SENSOR_ENABLE
    HAL_Sensor_Init();
#endif
    Audio_Init();
    SD_Init();

    Display_Init();

    Timer_SetInterrupt(CONFIG_HAL_UPDATE_TIM, 10 * 1000, HAL_InterrputUpdate);
		
    TIM_Cmd(CONFIG_HAL_UPDATE_TIM, ENABLE);
		
		//霍尔开关定时器配置
		HALL_Timer_SetInterrupt(CONFIG_HALL_SWITCH_TIM, 1000 * 1000, HALL_SWITCH_InterrputUpdate);
		
		TIM_Cmd(CONFIG_HALL_SWITCH_TIM, ENABLE);
}

#if CONFIG_SENSOR_ENABLE
static void HAL_SensorUpdate()
{
#if CONFIG_SENSOR_IMU_ENABLE
    HAL::IMU_Update();
#endif

#if CONFIG_SENSOR_MAG_ENABLE
    HAL::MAG_Update();
#endif
}
#endif

void HAL::HAL_Update()
{
    __IntervalExecute(SD_Update(), 500);
#if CONFIG_SENSOR_ENABLE
    __IntervalExecute(HAL_SensorUpdate(), 1000);
#endif
    __IntervalExecute(GPS_Update(), 200);
    __IntervalExecute(Memory_DumpInfo(), 1000);
    Power_EventMonitor();
}
