// -------------------------------------------------------------------------
//    @FileName         :    NFPluginLoader.cpp
//    @Author           :    LvSheng.Huang
//    @Date             :
//    @Module           :    NFPluginLoader
//
// -------------------------------------------------------------------------

#include <crtdbg.h>
#include <time.h>
#include <stdio.h>
#include <iostream>
#include <utility>
#include <thread>
#include <chrono>
#include <functional>
#include <atomic>
#include "NFCActorManager.h"
#include "NFComm/Config/NFConfig.h"
#include "NFComm/NFCore/NFPlatform.h"
#include "boost/thread.hpp"

#pragma comment( lib, "DbgHelp" )

#if NF_PLATFORM == NF_PLATFORM_WIN \
    || NF_PLATFORM == NF_PLATFORM_APPLE \
    || NF_PLATFORM == NF_PLATFORM_LINUX

#ifdef NF_DEBUG_MODE
#define new   new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

// ����Dump�ļ�
void CreateDumpFile(const std::string& strDumpFilePathName, EXCEPTION_POINTERS* pException)
{
    // ����Dump�ļ�
    HANDLE hDumpFile = CreateFile(strDumpFilePathName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    // Dump��Ϣ
    MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
    dumpInfo.ExceptionPointers = pException;
    dumpInfo.ThreadId = GetCurrentThreadId();
    dumpInfo.ClientPointers = TRUE;

    // д��Dump�ļ�����
    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);

    CloseHandle(hDumpFile);
}

// ����Unhandled Exception�Ļص�����
long ApplicationCrashHandler(EXCEPTION_POINTERS* pException)
{
    time_t t = time(0);
    char szDmupName[MAX_PATH];
    tm* ptm = localtime(&t);

    sprintf(szDmupName, "%d_%d_%d_%d_%d_%d.dmp",  ptm->tm_year + 1900, ptm->tm_mon, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    CreateDumpFile(szDmupName, pException);

    FatalAppExit(-1,  szDmupName);

    return EXCEPTION_EXECUTE_HANDLER;
}

void CloseXButton()
{
#if NF_PLATFORM == NF_PLATFORM_WIN

    HWND hWnd = GetConsoleWindow();  
    if(hWnd)  
    {  
        HMENU hMenu = GetSystemMenu(hWnd, FALSE);  
        EnableMenuItem(hMenu, SC_CLOSE, MF_DISABLED | MF_BYCOMMAND);  
    }

#endif
}

bool bExitApp = false;
boost::thread gThread;

void ThreadFunc()
{
    while ( !bExitApp )
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        std::string s;
        std::cin >> s;
        if ( 0 == stricmp( s.c_str(), "exit" ) )
        {
            bExitApp = true;
        }
    }
}

void CreateBackThread()
{
    gThread = boost::thread(boost::bind(&ThreadFunc));
    //std::cout << "CreateBackThread, thread ID = " << gThread.get_id() << std::endl;
}

void PrintfLogo()
{
    std::cout << "\n" << std::endl;

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    std::cout << "�������������������������" << std::endl;
    std::cout << "��                                            ��" << std::endl;
    std::cout << "��                 NoahFrame                  ��" << std::endl;
    std::cout << "��     Copyright (c) 2011  kytoo              ��" << std::endl;
    std::cout << "��             All rights reserved.           ��" << std::endl;
    std::cout << "��                                            ��" << std::endl;
    std::cout << "��               www.yowoyo.com               ��" << std::endl;
    std::cout << "��                                            ��" << std::endl;
    std::cout << "�������������������������\n" << std::endl;

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    std::cout << "\n" << std::endl;
}

#if NF_PLATFORM == NF_PLATFORM_WIN || NF_PLATFORM == NF_PLATFORM_LINUX || NF_PLATFORM == NF_PLATFORM_APPLE
int main()
#elif  NF_PLATFORM == NF_PLATFORM_ANDROID || NF_PLATFORM == NF_PLATFORM_APPLE_IOS

#endif
{
#ifndef NF_DEBUG_MODE
    SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ApplicationCrashHandler);
#endif

#ifdef NF_DEBUG_MODE
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

	NFCActorManager::GetSingletonPtr()->Init();
	NFCActorManager::GetSingletonPtr()->AfterInit();

#if defined( __CLIENT__ )
    RunCheckPoint();
#endif

    PrintfLogo();
    CloseXButton();
    CreateBackThread();

#ifndef NF_DEBUG_MODE
    while (!bExitApp)    //DEBUG�汾������RELEASE����
#endif
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        while (true)
        {
            if (bExitApp)
            {
                break;
            }
			
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));

#ifndef NF_DEBUG_MODE
            __try
#endif
            {
                NFCActorManager::GetSingletonPtr()->Execute(0.1f, 0.1f);
            }
#ifndef NF_DEBUG_MODE
            __except (ApplicationCrashHandler(GetExceptionInformation()))
            {
            }
#endif
        }
    }

	NFCActorManager::GetSingletonPtr()->BeforeShut();
	NFCActorManager::GetSingletonPtr()->Shut();

    NFCActorManager::GetSingletonPtr()->ReleaseInstance();

    return 0;
}

#else

#endif