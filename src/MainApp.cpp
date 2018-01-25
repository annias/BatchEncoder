﻿// Copyright (c) Wiesław Šoltés. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "StdAfx.h"
#include "MainApp.h"
#include "controls\MyDialogEx.h"
#include "dialogs\MainDlg.h"
#include "utilities\Utilities.h"

namespace app
{
    std::wstring CombinePath(const std::wstring& szPath, const std::wstring& szFile)
    {
        std::wstring szOutputFile = szFile;
        if (szPath.length() >= 1)
        {
            auto cLast = szPath[szPath.length() - 1];
            if ((cLast == '\\') || (cLast == '/'))
                szOutputFile = szPath + szOutputFile;
            else
                szOutputFile = szPath + L"\\") + szOutputFile;
        }
        return szOutputFile;
    }

    bool IsPortable()
    {
        cont std::wstring szPath = util::GetExeFilePath() + L"BatchEncoder.portable";
        return ::PathFileExists(szPath.c_str()) == TRUE;
    }

    CMainApp m_App;

    BEGIN_MESSAGE_MAP(CMainApp, CWinAppEx)
        ON_COMMAND(ID_HELP, CWinAppEx::OnHelp)
    END_MESSAGE_MAP()

    CMainApp::CMainApp()
    {
    }

    BOOL CMainApp::InitInstance()
    {
        INITCOMMONCONTROLSEX InitCtrls;
        InitCtrls.dwSize = sizeof(InitCtrls);
        InitCtrls.dwICC = ICC_WIN95_CLASSES;
        InitCommonControlsEx(&InitCtrls);

        CWinAppEx::InitInstance();
        AfxEnableControlContainer();
        InitShellManager();

        if (app::IsPortable())
        {
            this->szSettingsPath = util::GetExeFilePath();

            this->szFormatsPath = app::CombinePath(this->szSettingsPath, L"formats");
            this->szPresetsPath = app::CombinePath(this->szSettingsPath, L"presets");
            this->szLanguagesPath = app::CombinePath(this->szSettingsPath, L"lang");
            this->szProgressPath = app::CombinePath(this->szSettingsPath, L"progress");
            this->szToolsPath = app::CombinePath(this->szSettingsPath, L"tools");

            try
            {
                ::CreateDirectory(this->szFormatsPath.c_str(), NULL);
                ::CreateDirectory(this->szPresetsPath.c_str(), NULL);
                ::CreateDirectory(this->szLanguagesPath.c_str(), NULL);
                ::CreateDirectory(this->szProgressPath.c_str(), NULL);
                ::CreateDirectory(this->szToolsPath.c_str(), NULL);
            }
            catch (...) {}

            this->szOptionsFile = app::CombinePath(this->szSettingsPath, L"Options.xml");
            this->szFormatsFile = app::CombinePath(this->szSettingsPath, L"formats.xml");
            this->szItemsFile = app::CombinePath(this->szSettingsPath, L"Items.xml");
            this->szToolsFile = app::CombinePath(this->szSettingsPath, L"Tools.xml");
        }
        else
        {
            CString szConfigDir = L"BatchEncoder";

            this->szSettingsPath = util::GetSettingsFilePath(L""), szConfigDir);

            this->szFormatsPath = util::GetSettingsFilePath(L"", szConfigDir + L"\\formats");
            this->szPresetsPath = util::GetSettingsFilePath(L"", szConfigDir + L"\\presets");
            this->szLanguagesPath = util::GetSettingsFilePath(L"", szConfigDir + L"\\lang");
            this->szProgressPath = util::GetSettingsFilePath(L"", szConfigDir + L"\\progress");
            this->szToolsPath = util::GetSettingsFilePath(L"", szConfigDir + L"\\tools");

            try
            {
                ::CreateDirectory(szSettingsPath.c_str(), NULL);
                ::CreateDirectory(szFormatsPath.c_str(), NULL);
                ::CreateDirectory(szPresetsPath.c_str(), NULL);
                ::CreateDirectory(szLanguagesPath.c_str(), NULL);
                ::CreateDirectory(szProgressPath.c_str(), NULL);
                ::CreateDirectory(szToolsPath.c_str(), NULL);
            }
            catch (...) {}

            this->szOptionsFile = util::GetSettingsFilePath(L"Options.xml", szConfigDir);
            this->szFormatsFile = util::GetSettingsFilePath(L"Formats.xml", szConfigDir);
            this->szItemsFile = util::GetSettingsFilePath(L"Items.xml", szConfigDir);
            this->szToolsFile = util::GetSettingsFilePath(L"Tools.xml", szConfigDir);
        }

        CMainDlg dlg;
        m_pMainWnd = &dlg;
        dlg.DoModal();

        return FALSE;
    }
}
