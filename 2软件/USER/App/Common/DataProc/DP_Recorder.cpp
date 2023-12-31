#include "DataProc.h"
#include "Utils/GPX/GPX.h"
#include <stdio.h>
#include "Config/Config.h"
#include "Version.h"

using namespace DataProc;

#define RECORDER_GPX_TIME_FMT    "%d-%02d-%02dT%02d:%02d:%02dZ"
#define RECORDER_GPX_META_NAME   VERSION_FIRMWARE_NAME " " VERSION_SOFTWARE
#define RECORDER_GPX_META_DESC   VERSION_PROJECT_LINK

typedef struct
{
    GPX gpx;
    Recorder_Info_t recInfo;
    lv_fs_file_t file;
    bool active;
    Account* account;
} Recorder_t;

static lv_fs_res_t Recorder_FileWriteString(lv_fs_file_t* file_p, const char* str)
{
    //LV_LOG_USER(str);

    lv_fs_res_t res = lv_fs_write(
                          file_p,
                          str,
                          strlen(str),
                          nullptr
                      );

    return res;
}

static int Recorder_GetTimeConv(
    Recorder_t* recorder,
    const char* format,
    char* buf,
    uint32_t size)
{
    HAL::Clock_Info_t clock;
    recorder->account->Pull("Clock", &clock, sizeof(clock));

    int ret = snprintf(
                  buf,
                  size,
                  format,
                  clock.year,
                  clock.month,
                  clock.day,
                  clock.hour,
                  clock.minute,
                  clock.second
              );
    return ret;
}

static void Recorder_RecPoint(Recorder_t* recorder, HAL::GPS_Info_t* gpsInfo)
{
    //LV_LOG_USER("Track recording...");

    char timeBuf[64];
    Recorder_GetTimeConv(
        recorder,
        RECORDER_GPX_TIME_FMT,
        timeBuf,
        sizeof(timeBuf)
    );

    recorder->gpx.setEle(String(gpsInfo->altitude, 2));
    recorder->gpx.setTime(timeBuf);

    String gpxStr = recorder->gpx.getPt(
                        GPX_TRKPT,
                        String(gpsInfo->longitude, 6),
                        String(gpsInfo->latitude, 6)
                    );

    Recorder_FileWriteString(&(recorder->file), gpxStr.c_str());
}

static void Recorder_RecStart(Recorder_t* recorder, uint16_t time)
{
    LV_LOG_USER("Track record start");

    char filepath[128];
    Recorder_GetTimeConv(
        recorder,
        RECORDER_GPX_FILE_NAME,
        filepath, sizeof(filepath)
    );

    lv_fs_res_t res = lv_fs_open(&(recorder->file), filepath, LV_FS_MODE_WR | LV_FS_MODE_RD);

    if (res == LV_FS_RES_OK)
    {
        LV_LOG_USER("Track file %s open success", filepath);

        GPX* gpx = &(recorder->gpx);
        lv_fs_file_t* file_p = &(recorder->file);

        gpx->setMetaName(RECORDER_GPX_META_NAME);
        gpx->setMetaDesc(RECORDER_GPX_META_DESC);
        gpx->setName(filepath);
        gpx->setDesc("");

        Recorder_FileWriteString(file_p, gpx->getOpen().c_str());
				Recorder_FileWriteString(file_p, gpx->getInfo_Open().c_str());
				Recorder_FileWriteString(file_p, gpx->getInfo_Close().c_str());
        Recorder_FileWriteString(file_p, gpx->getMetaData().c_str());
        Recorder_FileWriteString(file_p, gpx->getTrakOpen().c_str());
        Recorder_FileWriteString(file_p, gpx->getInfo().c_str());
        Recorder_FileWriteString(file_p, gpx->getTrakSegOpen().c_str());

        recorder->active = true;
    }
    else
    {
        LV_LOG_ERROR("Track file open error!");
    }
}

/*添加暂停储存信息*/
static void Recorder_Pause(Recorder_t* recorder)
{
		recorder->active = false;
		GPX* gpx = &(recorder->gpx);
		lv_fs_file_t* file_p = &(recorder->file);
	
		Recorder_FileWriteString(file_p,gpx->getType_Open().c_str());
		Recorder_FileWriteString(file_p,gpx->getPause().c_str());
		Recorder_FileWriteString(file_p,gpx->getType_Close().c_str());
	
		LV_LOG_USER("Track record pause");
	
}

/*添加继续储存信息*/
static void Recorder_Continue(Recorder_t* recorder)
{
		
		GPX* gpx = &(recorder->gpx);
		lv_fs_file_t* file_p = &(recorder->file);
	
		Recorder_FileWriteString(file_p,gpx->getType_Open().c_str());
		Recorder_FileWriteString(file_p,gpx->getContinue().c_str());
		Recorder_FileWriteString(file_p,gpx->getType_Close().c_str());
		
		LV_LOG_USER("Track record continue");
    recorder->active = true;
}

static void Recorder_RecStop(Recorder_t* recorder)
{
    recorder->active = false;
    GPX* gpx = &(recorder->gpx);
    lv_fs_file_t* file_p = &(recorder->file);

    Recorder_FileWriteString(file_p, gpx->getTrakSegClose().c_str());;
    Recorder_FileWriteString(file_p, gpx->getTrakClose().c_str());
    Recorder_FileWriteString(file_p, gpx->getClose().c_str());
    lv_fs_close(file_p);

    LV_LOG_USER("Track record end");
}

static int onNotify(Recorder_t* recorder, Recorder_Info_t* info)
{
    int retval = 0;

    switch (info->cmd)
    {
    case RECORDER_CMD_START:
        Recorder_RecStart(recorder, info->time);
        break;
    case RECORDER_CMD_PAUSE:
        Recorder_Pause(recorder);
        break;
    case RECORDER_CMD_CONTINUE:
        Recorder_Continue(recorder);
        break;
    case RECORDER_CMD_STOP:
        Recorder_RecStop(recorder);
        break;
    }

    TrackFilter_Info_t tfInfo =
    {
        .cmd = (TrackFilter_Cmd_t)info->cmd
    };
    recorder->account->Notify("TrackFilter", &tfInfo, sizeof(tfInfo));

    return retval;
}

static int onEvent(Account* account, Account::EventParam_t* param)
{
    int retval = Account::ERROR_UNKNOW;
    Recorder_t* recorder = (Recorder_t*)account->UserData;;

    switch (param->event)
    {
    case Account::EVENT_PUB_PUBLISH:
        if (param->size == sizeof(HAL::GPS_Info_t))
        {
            if (recorder->active)
            {
                Recorder_RecPoint(recorder, (HAL::GPS_Info_t*)param->data_p);
            }
            retval = Account::ERROR_NONE;
        }
        else
        {
            retval = Account::ERROR_SIZE_MISMATCH;
        }
        break;

    case Account::EVENT_SUB_PULL:
        if (param->size == sizeof(Recorder_Info_t))
        {
            memcpy(param->data_p, &(recorder->recInfo), param->size);
        }
        else
        {
            retval = Account::ERROR_SIZE_MISMATCH;
        }
        break;

    case Account::EVENT_NOTIFY:
        if (param->size == sizeof(Recorder_Info_t))
        {
            retval = onNotify(recorder, (Recorder_Info_t*)param->data_p);
        }
        else
        {
            retval = Account::ERROR_SIZE_MISMATCH;
        }
        break;

    default:
        break;
    }

    return retval;
}

DATA_PROC_INIT_DEF(Recorder)
{
    static Recorder_t recorder;
    recorder.account = account;
    account->UserData = &recorder;

    account->Subscribe("GPS");
    account->Subscribe("Clock");
    account->Subscribe("TrackFilter");
    account->SetEventCallback(onEvent);
}
