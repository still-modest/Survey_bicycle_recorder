#include "HAL.h"
#include "ButtonEvent/ButtonEvent.h"

static ButtonEvent EncoderPush(CONFIG_POWER_SHUTDOWM_DELAY);

static bool EncoderEnable = true;
static volatile int32_t EncoderDiff = 0;
static bool EncoderDiffDisable = false;

/*****/
static volatile int32_t EncoderBack = 0;
/*****/

static void Buzz_Handler(int dir)
{
    static const uint16_t freqStart = 2000;
    static uint16_t freq = freqStart;
    static uint32_t lastRotateTime;

    if(millis() - lastRotateTime > 1000)
    {
        freq = freqStart;
    }
    else
    {
        if(dir > 0)
        {
            freq += 100;
        }

        if(dir < 0)
        {
            freq -= 100;
        }

        freq = constrain(freq, 100, 20 * 1000);
    }

    lastRotateTime = millis();
    HAL::Buzz_Tone(freq, 5);
}

static void Encoder_EventHandler_A()
{
    if(!EncoderEnable || EncoderDiffDisable)
    {
        return;
    }

		int dir ;
		if(digitalRead(CONFIG_ENCODER_A_PIN) == LOW)
		{
			dir = -1;
		}
		EncoderDiff += dir;
		Buzz_Handler(dir);
		
}

static void Encoder_EventHandler_B()
{
    if(!EncoderEnable || EncoderDiffDisable)
    {
        return;
    }

		int dir ;
		if(digitalRead(CONFIG_ENCODER_B_PIN) == LOW)
		{
			dir = +1;
		}
		EncoderDiff += dir;
		Buzz_Handler(dir);
		
}

/*********/
static void Encoder_EventHandler_D()
{
	
		//关机
		if(digitalRead(CONFIG_ENCODER_D_PIN) == LOW)
		{	
			delay(1000);
			if(digitalRead(CONFIG_ENCODER_D_PIN) == LOW)
			{
				HAL::Audio_PlayMusic("Shutdown");
				delay(500);
				HAL::Power_Shutdown();
			}
		}
		
		
}
/*********/
static void Encoder_PushHandler(ButtonEvent* btn, int event)  //
{
    if(event == ButtonEvent::EVENT_PRESSED)
    {
        EncoderDiffDisable = true;
    }
    else if(event == ButtonEvent::EVENT_RELEASED)
    {
        EncoderDiffDisable = false;
    }
    else if(event == ButtonEvent::EVENT_LONG_PRESSED)
    {
//        HAL::Audio_PlayMusic("Shutdown");
//        HAL::Power_Shutdown();
    }
}

void HAL::Encoder_Init()
{
    pinMode(CONFIG_ENCODER_A_PIN, INPUT_PULLUP);
    pinMode(CONFIG_ENCODER_B_PIN, INPUT_PULLUP);
    pinMode(CONFIG_ENCODER_PUSH_PIN, INPUT_PULLUP);
		pinMode(CONFIG_ENCODER_D_PIN, INPUT_PULLUP);

    attachInterrupt(CONFIG_ENCODER_A_PIN, Encoder_EventHandler_A, FALLING);
		attachInterrupt(CONFIG_ENCODER_B_PIN, Encoder_EventHandler_B, FALLING);
		attachInterrupt(CONFIG_ENCODER_D_PIN, Encoder_EventHandler_D, FALLING);

    EncoderPush.EventAttach(Encoder_PushHandler);
}

void HAL::Encoder_Update()
{
    EncoderPush.EventMonitor(Encoder_GetIsPush());
}

int32_t HAL::Encoder_GetDiff()
{
    int32_t diff = EncoderDiff;
    EncoderDiff = 0;
    return diff;
}
/******/
int32_t HAL::Encoder_GetIsBack()
{
		int32_t IsBack = EncoderBack;
		EncoderBack = 0;
	  return IsBack;
}
/******/
bool HAL::Encoder_GetIsPush()
{
    if(!EncoderEnable)
    {
        return false;
    }
    
    return (digitalRead(CONFIG_ENCODER_PUSH_PIN) == LOW);
}

void HAL::Encoder_SetEnable(bool en)
{
    EncoderEnable = en;
}
