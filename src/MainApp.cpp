﻿#include "StdAfx.h"
#include "MainApp.h"
#include "dialogs\MainDlg.h"

namespace app
{
    CMainApp m_App;

    BEGIN_MESSAGE_MAP(CMainApp, CWinApp)
    END_MESSAGE_MAP()

    CMainApp::CMainApp()
    {
    }

    BOOL CMainApp::InitInstance()
    {
        HANDLE hMutex = ::CreateMutex(nullptr, TRUE, L"BatchEncoder");
        if (hMutex == nullptr || ::GetLastError() == ERROR_ALREADY_EXISTS)
        {
            if (hMutex != nullptr)
                ::ReleaseMutex(hMutex);

            HWND m_Hwnd = ::FindWindow(0, L"BatchEncoder");
            if (m_Hwnd)
                ::SetForegroundWindow(m_Hwnd);
            else
                ::MessageBox(nullptr, L"You can only run one instance of the BatchEncoder.", L"Error", MB_ICONERROR | MB_OK);

            return FALSE;
        }

        dialogs::CMainDlg dlg;

        dlg.m_Config.FileSystem = std::make_unique<worker::Win32FileSystem>();

        dlg.m_Config.m_Settings.Init(dlg.m_Config.FileSystem.get());

        dlg.m_Config.Log = std::make_unique<util::FileLog>(dlg.m_Config.m_Settings.szLogFile);
        dlg.m_Config.Log->Open();

        std::wstring szConfigMode = dlg.m_Config.m_Settings.IsPortable(dlg.m_Config.FileSystem.get()) ? L"Portable" : L"Roaming";
        dlg.m_Config.Log->Log(L"[Info] Program started: " + szConfigMode);

        try
        {
            INITCOMMONCONTROLSEX InitCtrls;
            InitCtrls.dwSize = sizeof(InitCtrls);
            InitCtrls.dwICC = ICC_WIN95_CLASSES;
            InitCommonControlsEx(&InitCtrls);

            CWinApp::InitInstance();
        }
        catch (...)
        {
            dlg.m_Config.Log->Log(L"[Error] Failed to init application.");
        }

        try
        {
            m_pMainWnd = &dlg;
            dlg.DoModal();
        }
        catch (...)
        {
            dlg.m_Config.Log->Log(L"[Error] Main dialog exception.");
        }

        dlg.m_Config.Log->Log(L"[Info] Program exited: " + szConfigMode);
        dlg.m_Config.Log->Close();

        ::ReleaseMutex(hMutex);
        return FALSE;
    }
}
