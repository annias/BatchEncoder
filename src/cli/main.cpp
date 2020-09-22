﻿#define WIN32_LEAN_AND_MEAN
#include <SDKDDKVer.h>
#include <Windows.h>
#include "mainapp.h"
#include "utilities\ArgvParser.h"

int wmain(int argc, wchar_t *argv[])
{
    config::CConfig m_Config;

    m_Config.FileSystem = std::make_unique<worker::Win32FileSystem>();

    m_Config.m_Settings.Init(m_Config.FileSystem.get());

    m_Config.Log = std::make_unique<util::ConsoleLog>();
    m_Config.Log->Open();

    std::wstring szConfigMode = m_Config.m_Settings.IsPortable(m_Config.FileSystem.get()) ? L"Portable" : L"Roaming";
    m_Config.Log->Log(L"[Info] Program started: " + szConfigMode);

    try
    {
        m_Config.nLangId = -1;

        m_Config.LoadTools(m_Config.m_Settings.szToolsPath);
        m_Config.LoadFormats(m_Config.m_Settings.szFormatsPath);

        if (m_Config.LoadOutputs(m_Config.m_Settings.szOutputsFile) == false)
            m_Config.m_Outputs = config::m_OutpuPathsPresets;

        if (m_Config.LoadOptions(m_Config.m_Settings.szOptionsFile) == false)
            m_Config.m_Options.Defaults();

        m_Config.LoadItems(m_Config.m_Settings.szItemsFile);
    }
    catch (...)
    {
        m_Config.Log->Log(L"[Error] Failed to load config.");
    }

    m_Config.FileSystem->SetCurrentDirectory_(m_Config.m_Settings.szSettingsPath);

    worker::Win32WorkerFactory m_Factory;
    CConsoleWorkerContext ctx;

    size_t nItems = m_Config.m_Items.size();
    if (nItems <= 0)
    {
        ctx.bDone = true;
        m_Config.Log->Log(L"[Error] No items in the list.");
        m_Config.Log->Close();
        return -1;
    }

    std::wstring szDefaultTime = m_Config.GetString(0x00150001);
    std::wstring szDefaultStatus = m_Config.GetString(0x00210001);
    size_t nChecked = 0;
    for (size_t i = 0; i < nItems; i++)
    {
        config::CItem& item = m_Config.m_Items[i];
        if (item.bChecked == true)
        {
            item.szTime = szDefaultTime;
            item.szStatus = szDefaultStatus;
            nChecked++;
        }
    }

    if (nChecked <= 0)
    {
        ctx.bDone = true;
        m_Config.Log->Log(L"[Error] No checked items in the list.");
        m_Config.Log->Close();
        return -1;
    }

    ctx.Init();

    ctx.nThreadCount = m_Config.m_Options.nThreadCount;
    if (ctx.nThreadCount < 1)
    {
        util::LogicalProcessorInformation info;
        if (util::GetLogicalProcessorInformation(&info) == 0)
            ctx.nThreadCount = info.processorCoreCount;
        else
            ctx.nThreadCount = 1;
    }

    ctx.pConfig = &m_Config;

    auto m_WorkerThread = std::thread([&ctx, &m_Config]()
    {
        auto pWorker = std::make_unique<worker::CWorker>();
        pWorker->ConsoleConverter = std::make_unique<worker::CConsoleConverter>();
        pWorker->PipesConverter = std::make_unique<worker::CPipesConverter>();
        pWorker->PipesTranscoder = std::make_unique<worker::CPipesTranscoder>();
        pWorker->Convert(&ctx, m_Config.m_Items);
    });
    m_WorkerThread.join();

    ctx.bRunning = false;

    m_Config.Log->Log(L"[Info] Program exited: " + szConfigMode);
    m_Config.Log->Close();

    return 0;
}
