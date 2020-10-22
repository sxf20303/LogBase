/*
Copyright (C) 2016 - 2019, Beijing 7invensun Technology Co.Ltd.All rights reserved.
File Name: SdkLog.cpp
description: 通用日志记录类实现文件
Created by oscar on 2018/4/23
*/

#include "SdkSysLog.h"
#include "SDK_VERSION_CONTROL.h"
#include<stdlib.h>

#ifdef _SDK_ENENT_MGR_H_
#include "SdkEventMgr.h"
#endif

#ifdef _WIN32
#include <windows.h>
#include "sys/timeb.h"
#elif __ANDROID__
#include <unistd.h>
#elif __linux__
#include<sys/time.h>
#include<string.h>
#endif

#ifdef __linux__
#include <stdarg.h>
#include <unistd.h>
#endif

#ifdef __ANDROID__

#include <libgen.h>
#include <android/log.h>

// log锟斤拷签
#define TAG "SDK.C++"

// 锟斤拷锟斤拷info锟斤拷息
# define LOGI(FMT, ...) __android_log_print(ANDROID_LOG_INFO , TAG, FMT, ## __VA_ARGS__)
// 锟斤拷锟斤拷debug锟斤拷息
# define LOGD(FMT, ...) __android_log_print(ANDROID_LOG_DEBUG, TAG, FMT, ## __VA_ARGS__)
// 锟斤拷锟斤拷error锟斤拷息
# define LOGE(FMT, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, FMT, ## __VA_ARGS__)

#elif defined(__linux__)

# define LOGI(FMT, ...) printf(FMT, ## __VA_ARGS__)

#endif

#ifdef _WIN32
__declspec(thread) char SdkSysLog::g_timeStr[1024] = {};
#else
thread_local char SdkSysLog::g_timeStr[1024] = {};
#endif

int SdkSysLog::Init(const char* path)
{
    m_path = std::string(path);
    m_curLogLevel = 0;
    return 0;
}
int SdkSysLog::SetLogLevel(int logLevel)
{
    m_curLogLevel = logLevel;
    return 0;
}

int SdkSysLog::AddLogFile(const char* file, int& fileIdx)
{
    if (file == nullptr)
    {
        fileIdx = -1;
        if (!m_pFileStreams.empty())
        {
            fileIdx = 0;
        }
        return 0;
    }
    FILE* fp;
    std::string name = file;
    if(m_path.empty())//没有设置路径
        return -1;
    name = m_path + std::string("/") + std::string(file);
#ifdef _WIN32
    if (fopen_s(&fp, name.c_str(), "wt") != 0)
#else
    if (nullptr == (fp = fopen(name.c_str(), "wt")))
#endif
    {
        return -2;
    }
    m_pFileStreams.push_back(fp);
    fileIdx = m_pFileStreams.size() -1;
    return 0;
}
int SdkSysLog::Close()
{
    for (int i = 0; i < m_pFileStreams.size(); i++)
    {
        int mtxId = i;
        if(mtxId >= MXT_NUM )
            mtxId = MXT_NUM -1;
        std::unique_lock<std::mutex> lock(m_mutex[mtxId]);
        if (m_pFileStreams[i])
        {
            fclose(m_pFileStreams[i]);
            m_pFileStreams[i] = nullptr;
        }
    }
    m_pFileStreams.clear();
    return 0;
}

char* SdkSysLog::GetTime()
{
#if  defined(__ANDROID__) || defined(__linux__)
    time_t tt = time(NULL);
    struct tm* ptr;
    ptr = localtime(&tt);
    // printf("time: %d \n", tt);
    char str[80];
    strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S", ptr);
    //2018-09-19 16:01:37.517
    struct timeval tmv;
    gettimeofday(&tmv, NULL);
//    char buf[128] = {0};
    sprintf(g_timeStr,"%s.%03d", str, (int)(tmv.tv_usec / 1000));

    return (char*)g_timeStr;
#elif _WIN32
    char date[64] = { 0 };

        struct timeb tb;
        ftime(&tb);
        std::tm now = { 0 };

        //gmtime_s(&now, &tb.time);
        localtime_s(&now, &tb.time);

        sprintf_s(g_timeStr, sizeof(g_timeStr), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
            now.tm_year + 1900, now.tm_mon + 1, now.tm_mday,
            now.tm_hour, now.tm_min, now.tm_sec,
            tb.millitm);

        return g_timeStr;
#endif

}

int SdkSysLog::Print(int fileIdx, int logLevel, const char* format, ...)
{
    char sbuf[4096];
    memset(sbuf, 0, sizeof(sbuf));
    va_list argList;
    va_start(argList, format);
#ifdef _WIN32
    vsprintf_s(sbuf, format, argList);
#else
    int ret = vsprintf(sbuf, format, argList);
    if (ret < 0)
    {
        exit(0);
    }
#endif
    va_end(argList);

    if (logLevel >= m_curLogLevel)
    {
        if (!m_pFileStreams.empty() && fileIdx >= 0)
        {
            if (m_pFileStreams.size() > fileIdx)
            {
                int mtxId = fileIdx;
                if(mtxId >= MXT_NUM )
                    mtxId = MXT_NUM -1;
                std::unique_lock<std::mutex> lock(m_mutex[mtxId]);
                fprintf(m_pFileStreams[fileIdx], "%s", sbuf);
                fprintf(m_pFileStreams[fileIdx], "\n");
                fflush(m_pFileStreams[fileIdx]);
            }
        }
    }
#ifdef _WIN32
    OutputDebugStringA(sbuf);
    OutputDebugStringA("\n");
#endif

#ifdef __ANDROID__
    LOGI("%s", sbuf);
#elif defined(__linux__)
    LOGI("%s\n",sbuf);
#endif

#ifdef _SDK_ENENT_MGR_H_
    SdkEventMgr::GetInstance()->SendLogInfo(sbuf, strlen(sbuf) + 1);
#endif

    return 0;
}
