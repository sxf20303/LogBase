/*
Copyright (C) 2016 - 2020, Beijing 7invensun Technology Co.Ltd.All rights reserved.
File Name: LogBase.h
description: 统一SDK日志头文件
Change History:
Date         Version        Changed By          Changes
2019/11/14    1.0.0.1        oscar             Create
*/

#ifndef _SDK_LOG_BASE_H_
#define _SDK_LOG_BASE_H_

/*
SDK日志使用说明：
使用步骤：
1.拷贝LogBase.h头文件到自己的代码目录并在工程中#include，注意不得擅自修改LogBase.h里的内容，请把需求提交给相关维护人员，统一进行修改。
2.在接口的init函数实现的cpp文件中需要做以下步骤，举例如下：

#include "LogBase.h"

INIT_SDK_LOG("Interface");//Interface是模块名

int Interface::Init(void* logHandle)//定义好的初始化函数，参数可能有所不同，但其中必须有void* logHandle参数
{
    INIT_SYS_LOG_HANDLE(logHandle);//这个宏意思是使用默认SDK的日志文件，如果需要定义自己的日志文件，需要使用INIT_DEF_LOG_HANDLE(logHandle, "1.log"); 其中1.log就是自己的日志文件名

    //自己的初始化实现
    return 0;
}

第一步是调用宏 INIT_SDK_LOG("自己所封装的模块名称"); 声明自己的模块名称
第二部是在init函数里拿到logHandle调用INIT_SYS_LOG_HANDLE(logHandle)初始化日志系统，也可以指定自己的日志文件名使用宏INIT_DEF_LOG_HANDLE(logHandle, "1.log");其中1.log是自己的日志文件名

3.在需要打印日志的地方直接调用宏SDK_LOG开始写日志了，举例如下：

int ret = -1;
SDK_LOG(SDK_ERRROR, "DealImage ret = %d", ret);

其中日志必须指定写出日志级别，现在分别包括：SDK_DEBUG，SDK_INFO，SDK_ERRROR，不可以掠过，之后的打印信息可以参照printf的格式。

*/

/**
* @brief 日志类型枚举，在写日志时必须指定枚举类型
*/
enum LOG_LEVEL
{
    SDK_DEBUG     = 0,
    SDK_INFO      = 1,
    SDK_ERROR    = 2,
    SDK_LOG_NUM
};
/**
* @brief 日志枚举类型对应的字符信息，一个枚举对应一个信息说明
*/
const static char* logType[SDK_LOG_NUM] = {
    "Debug", 
    "Info",
    "Error"
};

/**
* @brief 日志系统统一的接口类声明
*/
class SDKLogBase
{
public:
    /**
    * @brief 添加日志文件名到日志系统，如果使用默认的日志文件，参数file为空。
    *
    * @param file 输入数据，日志文件名，如果使用系统默认日志文件，请传入NULL，否则为文件名
    *
    * @param fileIdx 输出数据，返回文件名的索引id，对应file参数的文件名的索引id 
    *
    * @return  0成功，1成功（但文件已经存在请知晓），-1失败
    */
    virtual int AddLogFile(const char* file, int& fileIdx) = 0;
    /**
    * @brief 获取系统时间的接口，返回日期和时间字符串。
    *
    * @param 无参数
    *
    * @return  0成功，-1失败
    */
    virtual char* GetTime() = 0;
    /**
    * @brief 写具体的日志信息接口
    *
    * @param fileIdx 输入数据，AddLogFile接口返回的fileIdx索引id
    *
    * @param logLevel 输入数据，日志级别请参照LOG_LEVEL枚举
    *
    * @param format 输入数据，格式化字符串，请参照printf的输入
    *
    * @return  0成功，-1失败
    */
    virtual int Print(int fileIdx, int logLevel, const char* format, ...) = 0;
};

const int IDX_NUM = 10;//默认预留10个日志文件位置

extern SDKLogBase* g_logHandle;
extern int g_logFileIdx[IDX_NUM];
extern char* g_moduleName;

/**
* @brief 初始化日志系统宏，需要传入模块名，这个宏只能调用一次
*/
#define INIT_SDK_LOG(NAME) \
    SDKLogBase* g_logHandle = nullptr; \
    int g_logFileIdx[IDX_NUM] = {}; \
    char* g_moduleName = NAME;

/**
* @brief 在init函数里，拿到void* logHandle之后调用的宏，没有指定日志文件名，使用默认的日志文件，这个宏只能调用一次
*/
#define INIT_SYS_LOG_HANDLE(handle) \
if (handle) \
{\
    g_logHandle = (SDKLogBase*)handle; \
    g_logHandle->AddLogFile(nullptr, g_logFileIdx[0]); \
    g_logHandle->Print(g_logFileIdx[0], SDK_INFO, "[COMPILE][%s %s][%s]", __DATE__, __TIME__, g_moduleName); \
}

/**
* @brief 在init函数里，拿到void* logHandle之后调用的宏，指定日志文件名，这个宏与INIT_SYS_LOG_HANDLE宏选择其中一个并且只能调用一次
*/
#define INIT_DEF_LOG_HANDLE(handle,name) \
if (handle) \
{\
    g_logHandle = (SDKLogBase*)handle; \
    g_logHandle->AddLogFile(name, g_logFileIdx[0]); \
    g_logHandle->Print(g_logFileIdx[0], SDK_INFO, "[COMPILE][%s %s][%s]", __DATE__, __TIME__, g_moduleName); \
}

#define ADD_DEF_LOG(idx,name) \
if (g_logHandle) \
{\
    if ( idx > 0 && idx < IDX_NUM)\
    {\
        g_logHandle->AddLogFile(name, g_logFileIdx[idx]); \
    }\
}

#ifndef __linux__
#include "windows.h"
#define THREADID GetCurrentThreadId()
#else
#if defined (__aarch64__)
#include <unistd.h>
#include <sys/syscall.h>
#define THREADID syscall(SYS_gettid)
#else
#include "unistd.h"
#include <sys/syscall.h>
//#define THREADID gettid()
#define THREADID syscall(__NR_gettid)
#endif
#endif


/**
* @brief 写日志的宏，其中需要传入日志级别和格式化字符串，具体参照printf的参数
*/
#define SDK_LOG(logLevel,FMT, ...) { if(g_logHandle) g_logHandle->Print(g_logFileIdx[0],logLevel,"[%s][%d][%s][%s][%s:%d]: " FMT,g_logHandle->GetTime(),THREADID,logType[logLevel],g_moduleName, __FILE__, __LINE__,  ##__VA_ARGS__);}

#define SDK_IDX_LOG(idx,logLevel,FMT, ...) { if(g_logHandle && idx > 0 && idx < IDX_NUM) g_logHandle->Print(g_logFileIdx[idx],logLevel,FMT, ##__VA_ARGS__);}


#endif