/*
Copyright (C) 2016 - 2019, Beijing 7invensun Technology Co.Ltd.All rights reserved.
File Name: SDKLog.h
description: 通用日志文件接口定义头文件
Change History:
Date         Version        Changed By          Changes
2018/4/23    1.0.0.1        oscar           Create
*/

#ifndef _SDK_SYS_LOG_H_
#define _SDK_SYS_LOG_H_

#include "LogBase.h"
#include <mutex>
#include <vector>

#define MXT_NUM 10

class SdkSysLog : public SDKLogBase
{
public:
    virtual int AddLogFile(const char* file, int& fileIdx);
    virtual char* GetTime();
    virtual int Print(int fileIdx, int logLevel, const char* format, ...);

public:

    int Init(const char* path);
    int Close();

    int SetLogLevel(int logLevel);

    std::mutex m_mutex[MXT_NUM];
    std::vector<FILE*> m_pFileStreams;

    std::string m_path;
    int m_curLogLevel;
    char sbuf[4096];

#ifdef _WIN32
    static __declspec(thread) char g_timeStr[1024];
#else
    static thread_local char g_timeStr[1024];
#endif

};


#endif //_LOG_H_
