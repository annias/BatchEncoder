﻿#include "StdAfx.h"
#include "MainApp.h"
#include "MainDlg.h"
#include "PathsDlg.h"
#include "PresetsDlg.h"
#include "AboutDlg.h"
#include "FormatsDlg.h"
#include "ToolsDlg.h"

#define IDC_STATUSBAR           1500
#define IDC_FOLDERTREE          0x3741
#define IDC_TITLE               0x3742
#define IDC_STATUSTEXT          0x3743
#define IDC_CHECK_RECURSE       0x3744
#define IDC_BROWSE_NEW_FOLDER   0x3746
#define ID_LANGUAGE_MIN         2000
#define ID_LANGUAGE_MAX         2999

namespace dialogs
{
    static CString szLastOutputBrowse;
    static CString szLastDirectoryBrowse;
    static WNDPROC lpOldWindowProc;
    static bool bRecurseChecked = true;
    static HWND hWndBtnRecurse = nullptr;
    static HWND hWndStaticText = nullptr;

    LRESULT CALLBACK BrowseDlgWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if (uMsg == WM_COMMAND)
        {
            if ((HIWORD(wParam) == BN_CLICKED) && ((HWND)lParam == hWndBtnRecurse))
                bRecurseChecked = ::SendMessage(hWndBtnRecurse, BM_GETCHECK, (WPARAM)0, (LPARAM)0) == BST_CHECKED;
        }
        return ::CallWindowProc(lpOldWindowProc, hWnd, uMsg, wParam, lParam);
    }

    int CALLBACK BrowseCallbackAddDir(HWND hWnd, UINT uMsg, LPARAM lp, LPARAM pData)
    {
        if (uMsg == BFFM_INITIALIZED)
        {
            CMainDlg* pDlg = (CMainDlg*)pData;

            HWND hWndTitle = nullptr;
            HFONT hFont;
            RECT rc, rcTitle, rcTree, rcWnd;

            hWndTitle = ::GetDlgItem(hWnd, IDC_TITLE);

            ::GetWindowRect(hWndTitle, &rcTitle);
            ::GetWindowRect(::GetDlgItem(hWnd, IDC_FOLDERTREE), &rcTree);
            ::GetWindowRect(hWnd, &rcWnd);

            rc.top = 8;
            rc.left = rcTree.left - rcWnd.left;
            rc.right = rcTree.right - rcTree.left;
            rc.bottom = (rcTitle.bottom - rcTitle.top) + 8;

            hWndBtnRecurse = ::CreateWindowEx(0,
                _T("BUTTON"),
                pDlg->m_Config.GetString(0x00210005).c_str(),
                BS_CHECKBOX | BS_AUTOCHECKBOX | WS_CHILD | WS_TABSTOP | WS_VISIBLE,
                rc.left, rc.top,
                rc.right - rc.left, rc.bottom - rc.top,
                hWnd,
                nullptr, nullptr, nullptr);
            if (hWndBtnRecurse != nullptr)
            {
                ::ShowWindow(hWndTitle, SW_HIDE);
                ::ShowWindow(::GetDlgItem(hWnd, IDC_STATUSTEXT), SW_HIDE);

                if (bRecurseChecked == true)
                    ::SendMessage(hWndBtnRecurse, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
                else
                    ::SendMessage(hWndBtnRecurse, BM_SETCHECK, (WPARAM)BST_UNCHECKED, (LPARAM)0);

                lpOldWindowProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)BrowseDlgWindowProc);

                ::ShowWindow(hWndBtnRecurse, SW_SHOW);
                hFont = (HFONT) ::SendMessage(hWnd, WM_GETFONT, 0, 0);
                ::SendMessage(hWndBtnRecurse, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            }

            std::wstring szPath = szLastDirectoryBrowse;
            ::SendMessage(hWnd, BFFM_SETSELECTION, TRUE, (LPARAM)szPath.c_str());
        }
        return(0);
    }

    int CALLBACK BrowseCallbackOutPath(HWND hWnd, UINT uMsg, LPARAM lp, LPARAM pData)
    {
        if (uMsg == BFFM_INITIALIZED)
        {
            CMainDlg* pDlg = (CMainDlg*)pData;

            TCHAR szText[256] = _T("");
            HWND hWndTitle = nullptr;
            HFONT hFont;
            RECT rc, rcTitle, rcTree, rcWnd;

            hWndTitle = ::GetDlgItem(hWnd, IDC_TITLE);

            ::GetWindowText(hWndTitle, szText, 256);

            ::GetWindowRect(hWndTitle, &rcTitle);
            ::GetWindowRect(::GetDlgItem(hWnd, IDC_FOLDERTREE), &rcTree);
            ::GetWindowRect(hWnd, &rcWnd);

            rc.top = 8;
            rc.left = rcTree.left - rcWnd.left;
            rc.right = rcTree.right - rcTree.left;
            rc.bottom = (rcTitle.bottom - rcTitle.top) + 8;

            hWndStaticText = ::CreateWindowEx(0,
                _T("STATIC"),
                szText,
                SS_CENTERIMAGE | WS_CHILD | WS_TABSTOP | WS_VISIBLE,
                rc.left, rc.top,
                rc.right - rc.left, rc.bottom - rc.top,
                hWnd,
                nullptr, nullptr, nullptr);
            if (hWndStaticText != nullptr)
            {
                ::ShowWindow(hWndTitle, SW_HIDE);
                ::ShowWindow(::GetDlgItem(hWnd, IDC_STATUSTEXT), SW_HIDE);

                ::ShowWindow(hWndStaticText, SW_SHOW);
                hFont = (HFONT) ::SendMessage(hWnd, WM_GETFONT, 0, 0);
                ::SendMessage(hWndStaticText, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
            }

            std::wstring szPath = szLastOutputBrowse;
            ::SendMessage(hWnd, BFFM_SETSELECTION, TRUE, (LPARAM)szPath.c_str());
        }
        return(0);
    }

    class CMainDlgWorkerContext : public worker::IWorkerContext
    {
    private:
        volatile bool bSafeCheck;
        CMainDlg *pDlg;
        util::CTimeCount timer;
    public:
        CMainDlgWorkerContext(CMainDlg* pDlg)
        {
            this->bDone = true;
            this->bRunning = false;
            this->pConfig = nullptr;
            this->pFactory = std::make_shared<worker::Win32WorkerFactory>();
            this->bSafeCheck = false;
            this->pDlg = pDlg;
        }
        virtual ~CMainDlgWorkerContext() { }
    public:
        std::wstring GetString(int nKey)
        {
            return pDlg->m_Config.GetString(nKey);
        }
        void Init()
        {
            this->bRunning = true;
            this->bDone = false;
            this->nTotalFiles = 0;
            this->nProcessedFiles = 0;
            this->nErrors = 0;
            this->nLastItemId = -1;
        }
        void Start()
        {
            this->timer.Start();
            pDlg->m_Progress.SetPos(0);
        }
        void Stop()
        {
            this->timer.Stop();
            CString szFormat = pDlg->m_Config.GetString(0x00190004).c_str();
            CString szText;
            szText.Format(szFormat,
                this->nProcessedFiles,
                this->nTotalFiles,
                this->nProcessedFiles - this->nErrors,
                this->nErrors,
                ((this->nErrors == 0) || (this->nErrors > 1)) ?
                pDlg->m_Config.GetString(0x00190002).c_str() : pDlg->m_Config.GetString(0x00190001).c_str(),
                util::CTimeCount::Format(this->timer.ElapsedTime()).c_str());
            pDlg->m_StatusBar.SetText(szText, 1, 0);
            int nPos = (int)(100.0 * ((double)this->nProcessedFiles / (double)this->nTotalFiles));
            pDlg->m_Progress.SetPos(nPos);
            pDlg->FinishConvert();
            this->pConfig = nullptr;
            this->bRunning = false;
        }
        bool ItemProgress(int nItemId, int nProgress, bool bFinished, bool bError = false)
        {
            if (bError == true)
            {
                config::CItem &item = pDlg->m_Config.m_Items[nItemId];
                item.bFinished = true;
                if (pDlg->m_Config.m_Options.bStopOnErrors == true)
                {
                    pDlg->m_Progress.SetPos(0);
                    this->bRunning = false;
                }
                return this->bRunning;
            }

            if (bFinished == true)
            {
                config::CItem &item = pDlg->m_Config.m_Items[nItemId];
                item.bFinished = true;
            }

            if ((bFinished == false) && (this->bRunning == true))
            {
                config::CItem &item = pDlg->m_Config.m_Items[nItemId];
                item.nProgress = nProgress;
                if (item.nPreviousProgress > nProgress)
                    item.nPreviousProgress = nProgress;

                if (item.bChecked == true)
                {
                    int nItemProgress = item.nProgress;
                    int nItemPreviousProgress = item.nPreviousProgress;
                    if (item.bFinished == false)
                    {
                        if (nItemProgress > 0 && nItemProgress < 100 && nItemProgress > nItemPreviousProgress)
                        {
                            item.szStatus = std::to_wstring(nItemProgress) + L"%";
                            item.nPreviousProgress = nItemProgress;
                            pDlg->RedrawItem(nItemId);
                        }
                        else if (nItemProgress == 100 && nItemProgress > nItemPreviousProgress)
                        {
                            item.nPreviousProgress = nItemProgress;
                        }
                    }
                }

                if (this->bSafeCheck == false)
                {
                    this->bSafeCheck = true;
                    if (nItemId > this->nLastItemId)
                    {
                        this->nLastItemId = nItemId;
                        if (pDlg->m_Config.m_Options.bEnsureItemIsVisible == true)
                            pDlg->MakeItemVisible(nItemId);
                    }
                    this->bSafeCheck = false;
                }
            }
            return this->bRunning;
        }
        void ItemStatus(int nItemId, const std::wstring& szTime, const std::wstring& szStatus)
        {
            config::CItem &item = pDlg->m_Config.m_Items[nItemId];
            item.szTime = szTime;
            item.szStatus = szStatus;
            if (this->bSafeCheck == false)
            {
                this->bSafeCheck = true;
                if (nItemId > this->nLastItemId)
                {
                    this->nLastItemId = nItemId;
                    if (pDlg->m_Config.m_Options.bEnsureItemIsVisible == true)
                        pDlg->MakeItemVisible(nItemId);
                }
                this->bSafeCheck = false;
            }
            pDlg->RedrawItem(nItemId);
        }
        void TotalProgress(int nItemId)
        {
            CString szFormat = pDlg->m_Config.GetString(0x00190003).c_str();
            CString szText;
            szText.Format(szFormat,
                this->nProcessedFiles,
                this->nTotalFiles,
                this->nProcessedFiles - this->nErrors,
                this->nErrors,
                ((this->nErrors == 0) || (this->nErrors > 1)) ?
                pDlg->m_Config.GetString(0x00190002).c_str() : pDlg->m_Config.GetString(0x00190001).c_str());
            pDlg->m_StatusBar.SetText(szText, 1, 0);

            if (nItemId > this->nLastItemId)
            {
                this->nLastItemId = nItemId;
                if (pDlg->m_Config.m_Options.bEnsureItemIsVisible == true)
                    pDlg->MakeItemVisible(nItemId);
            }

            int nPos = (int)(100.0f * ((double)this->nProcessedFiles / (double)this->nTotalFiles));
            pDlg->m_Progress.SetPos(nPos);
        }
    };

    IMPLEMENT_DYNAMIC(CMainDlg, CDialog)
    CMainDlg::CMainDlg(CWnd* pParent /*=nullptr*/)
        : CMyDialogEx(CMainDlg::IDD, pParent)
    {
        this->m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON_MAIN);
        this->m_Config.m_Options.Defaults();
        this->ctx = std::make_shared<CMainDlgWorkerContext>(this);
    }

    CMainDlg::~CMainDlg()
    {
    }

    void CMainDlg::DoDataExchange(CDataExchange* pDX)
    {
        CMyDialogEx::DoDataExchange(pDX);
        DDX_Control(pDX, IDC_PROGRESS_CONVERT, m_Progress);
        DDX_Control(pDX, IDC_STATIC_GROUP_OUTPUT, m_GrpOutput);
        DDX_Control(pDX, IDC_COMBO_PRESETS, m_CmbPresets);
        DDX_Control(pDX, IDC_COMBO_FORMAT, m_CmbFormat);
        DDX_Control(pDX, IDC_LIST_ITEMS, m_LstInputItems);
        DDX_Control(pDX, IDC_STATIC_TEXT_OUTPUT, m_StcOutPath);
        DDX_Control(pDX, IDC_COMBO_OUTPUT, m_CmbOutPath);
        DDX_Control(pDX, IDC_EDIT_THREADCOUNT, m_EdtThreads);
        DDX_Control(pDX, IDC_SPIN_THREADCOUNT, m_SpinThreads);
        DDX_Control(pDX, IDC_STATIC_TEXT_PRESET, m_StcPreset);
        DDX_Control(pDX, IDC_STATIC_TEXT_FORMAT, m_StcFormat);
        DDX_Control(pDX, IDC_STATIC_THREAD_COUNT, m_StcThreads);
        DDX_Control(pDX, IDC_BUTTON_CONVERT, m_BtnConvert);
        DDX_Control(pDX, IDC_BUTTON_BROWSE_OUTPUT, m_BtnBrowse);
    }

    BEGIN_MESSAGE_MAP(CMainDlg, CMyDialogEx)
        ON_WM_PAINT()
        ON_WM_QUERYDRAGICON()
        ON_WM_CLOSE()
        ON_WM_DESTROY()
        ON_WM_DROPFILES()
        ON_WM_HELPINFO()
        ON_MESSAGE(WM_ITEMCHANGED, OnListItemChaged)
        ON_MESSAGE(WM_NOTIFYFORMAT, OnNotifyFormat)
        ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_ITEMS, OnLvnGetdispinfoListItems)
        ON_NOTIFY(LVN_ODFINDITEM, IDC_LIST_ITEMS, OnOdfindListItems)
        ON_NOTIFY(NM_CLICK, IDC_LIST_ITEMS, OnNMClickListItems)
        ON_EN_KILLFOCUS(IDC_EDIT_ITEM, OnEnKillfocusEditItem)
        ON_NOTIFY(LVN_KEYDOWN, IDC_LIST_ITEMS, OnLvnKeydownListInputItems)
        ON_NOTIFY(NM_RCLICK, IDC_LIST_ITEMS, OnNMRclickListInputItems)
        ON_NOTIFY(NM_DBLCLK, IDC_LIST_ITEMS, OnNMDblclkListInputItems)
        ON_NOTIFY(LVN_ITEMCHANGING, IDC_LIST_ITEMS, OnLvnItemchangingListInputItems)
        ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_ITEMS, OnLvnItemchangedListInputItems)
        ON_CBN_SELCHANGE(IDC_COMBO_PRESETS, OnCbnSelchangeComboPresets)
        ON_CBN_SELCHANGE(IDC_COMBO_FORMAT, OnCbnSelchangeComboFormat)
        ON_BN_CLICKED(IDC_BUTTON_BROWSE_OUTPUT, OnBnClickedButtonBrowsePath)
        ON_BN_CLICKED(IDC_BUTTON_CONVERT, OnBnClickedButtonConvert)
        ON_COMMAND(ID_FILE_LOADLIST, OnFileLoadList)
        ON_COMMAND(ID_FILE_SAVELIST, OnFileSaveList)
        ON_COMMAND(ID_FILE_CLEARLIST, OnFileClearList)
        ON_COMMAND(ID_FILE_EXIT, OnFileExit)
        ON_COMMAND(ID_EDIT_ADDFILES, OnEditAddFiles)
        ON_COMMAND(ID_EDIT_ADDDIR, OnEditAddDir)
        ON_COMMAND(ID_EDIT_RENAME, OnEditRename)
        ON_COMMAND(ID_EDIT_RESETTIME, OnEditResetTime)
        ON_COMMAND(ID_EDIT_RESETOUTPUT, OnEditResetOutput)
        ON_COMMAND(ID_EDIT_REMOVE, OnEditRemove)
        ON_COMMAND(ID_EDIT_CROP, OnEditCrop)
        ON_COMMAND(ID_EDIT_REMOVECHECKED, OnEditRemoveChecked)
        ON_COMMAND(ID_EDIT_REMOVEUNCHECKED, OnEditRemoveUnchecked)
        ON_COMMAND(ID_EDIT_CHECKSELECTED, OnEditCheckSelected)
        ON_COMMAND(ID_EDIT_UNCHECKSELECTED, OnEditUncheckSelected)
        ON_COMMAND(ID_EDIT_SELECTALL, OnEditSelectAll)
        ON_COMMAND(ID_EDIT_SELECTNONE, OnEditSelectNone)
        ON_COMMAND(ID_EDIT_INVERTSELECTION, OnEditInvertSelection)
        ON_COMMAND(ID_EDIT_OPEN, OnEditOpen)
        ON_COMMAND(ID_EDIT_EXPLORE, OnEditExplore)
        ON_COMMAND(ID_ACTION_CONVERT, OnActionConvert)
        ON_COMMAND(ID_OPTIONS_CONFIGUREPRESETS, OnOptionsConfigurePresets)
        ON_COMMAND(ID_OPTIONS_CONFIGUREFORMAT, OnOptionsConfigureFormat)
        ON_COMMAND(ID_OPTIONS_CONFIGURETOOLS, OnOptionsConfigureTools)
        ON_COMMAND(ID_OPTIONS_DELETE_SOURCE, OnOptionsDeleteSource)
        ON_COMMAND(ID_OPTIONS_SHUTDOWN_WINDOWS, OnOptionsShutdownWindows)
        ON_COMMAND(ID_OPTIONS_DO_NOT_SAVE, OnOptionsDoNotSave)
        ON_COMMAND(ID_OPTIONS_DELETE_ON_ERRORS, OnOptionsDeleteOnErrors)
        ON_COMMAND(ID_OPTIONS_STOP_ON_ERRORS, OnOptionsStopOnErrors)
        ON_COMMAND(ID_OPTIONS_HIDE_CONSOLE, OnOptionsHideConsole)
        ON_COMMAND(ID_OPTIONS_ENSURE_VISIBLE, OnOptionsEnsureVisible)
        ON_COMMAND(ID_OPTIONS_FIND_DECODER, OnOptionsFindDecoder)
        ON_COMMAND(ID_OPTIONS_VALIDATE_FILES, OnOptionsValidateFiles)
        ON_COMMAND(ID_OPTIONS_OVERWRITE_FILES, OnOptionsOverwriteFiles)
        ON_COMMAND(ID_OPTIONS_RENAME_FILES, OnOptionsRenameFiles)
        ON_COMMAND(ID_OPTIONS_DOWNLOAD_TOOLS, OnOptionsDownloadTools)
        ON_COMMAND(ID_LANGUAGE_DEFAULT, OnLanguageDefault)
        ON_COMMAND_RANGE(ID_LANGUAGE_MIN, ID_LANGUAGE_MAX, OnLanguageChange)
        ON_COMMAND(ID_HELP_WEBSITE, OnHelpWebsite)
        ON_COMMAND(ID_HELP_ABOUT, OnHelpAbout)
        ON_COMMAND(ID_ACCELERATOR_F5, OnEditAddFiles)
        ON_COMMAND(ID_ACCELERATOR_F6, OnEditAddDir)
        ON_COMMAND(ID_ACCELERATOR_CTRL_L, OnFileLoadList)
        ON_COMMAND(ID_ACCELERATOR_CTRL_S, OnFileSaveList)
        ON_COMMAND(ID_ACCELERATOR_CTRL_E, OnFileClearList)
        ON_COMMAND(ID_ACCELERATOR_ALT_F4, OnFileExit)
        ON_COMMAND(ID_ACCELERATOR_F2, OnEditRename)
        ON_COMMAND(ID_ACCELERATOR_F3, OnEditResetTime)
        ON_COMMAND(ID_ACCELERATOR_F4, OnEditResetOutput)
        ON_COMMAND(ID_ACCELERATOR_CTRL_A, OnEditSelectAll)
        ON_COMMAND(ID_ACCELERATOR_CTRL_N, OnEditSelectNone)
        ON_COMMAND(ID_ACCELERATOR_CTRL_I, OnEditInvertSelection)
        ON_COMMAND(ID_ACCELERATOR_SHIFT_PLUS, OnEditCheckSelected)
        ON_COMMAND(ID_ACCELERATOR_SHIFT_MINUS, OnEditUncheckSelected)
        ON_COMMAND(ID_ACCELERATOR_CTRL_PLUS, OnEditRemoveChecked)
        ON_COMMAND(ID_ACCELERATOR_CTRL_MINUS, OnEditRemoveUnchecked)
        ON_COMMAND(ID_ACCELERATOR_F9, OnBnClickedButtonConvert)
        ON_COMMAND(ID_ACCELERATOR_F7, OnOptionsConfigurePresets)
        ON_COMMAND(ID_ACCELERATOR_F8, OnOptionsConfigureFormat)
        ON_COMMAND(ID_ACCELERATOR_F10, OnOptionsConfigureTools)
    END_MESSAGE_MAP()

    BOOL CMainDlg::OnInitDialog()
    {
        CMyDialogEx::OnInitDialog();

        InitCommonControls();

        SetIcon(m_hIcon, TRUE);
        SetIcon(m_hIcon, FALSE);

        m_StatusBar.Create(WS_CHILD | WS_VISIBLE | CCS_BOTTOM | SBARS_SIZEGRIP, CRect(0, 0, 0, 0), this, IDC_STATUSBAR);

        int nStatusBarParts[2] = { 100, -1 };
        m_StatusBar.SetParts(2, nStatusBarParts);

        CMFCDynamicLayout* layout = this->GetDynamicLayout();
        layout->AddItem(IDC_STATUSBAR, CMFCDynamicLayout::MoveVertical(100), CMFCDynamicLayout::SizeHorizontal(100));

        m_hAccel = ::LoadAccelerators(::GetModuleHandle(nullptr), MAKEINTRESOURCE(IDR_ACCELERATOR_MAIN));

#ifdef _UNICODE
        m_LstInputItems.SendMessage(CCM_SETUNICODEFORMAT, (WPARAM)(BOOL)TRUE, 0);
#endif

        this->SetWindowText(_T(VER_PRODUCTNAME_STR));

        m_SpinThreads.SetRange32(0, INT_MAX);

        m_Progress.SetRange(0, 100);
        m_Progress.SetPos(0);
        m_Progress.ShowWindow(SW_HIDE);

        DWORD dwExStyle = m_LstInputItems.GetExtendedStyle();
        dwExStyle |= LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | LVS_EX_DOUBLEBUFFER | LVS_EX_GRIDLINES;
        m_LstInputItems.SetExtendedStyle(dwExStyle);

        m_LstInputItems.InsertColumn(ITEM_COLUMN_NAME, _T("Name"), LVCFMT_LEFT, 200);
        m_LstInputItems.InsertColumn(ITEM_COLUMN_INPUT, _T("Input"), LVCFMT_LEFT, 50);
        m_LstInputItems.InsertColumn(ITEM_COLUMN_SIZE, _T("Size (bytes)"), LVCFMT_LEFT, 80);
        m_LstInputItems.InsertColumn(ITEM_COLUMN_OUTPUT, _T("Output"), LVCFMT_LEFT, 70);
        m_LstInputItems.InsertColumn(ITEM_COLUMN_PRESET, _T("Preset#"), LVCFMT_LEFT, 55);
        m_LstInputItems.InsertColumn(ITEM_COLUMN_OPTIONS, _T("Options"), LVCFMT_LEFT, 65);
        m_LstInputItems.InsertColumn(ITEM_COLUMN_TIME, _T("Time"), LVCFMT_LEFT, 90);
        m_LstInputItems.InsertColumn(ITEM_COLUMN_STATUS, _T("Status"), LVCFMT_LEFT, 80);

        m_EdtItem.Create(ES_AUTOHSCROLL | WS_CHILD, CRect(0, 0, 1, 1), &m_LstInputItems, IDC_EDIT_ITEM);
        m_EdtItem.SetFont(this->GetFont());

        m_StcFormat.SetBold(true);
        m_StcPreset.SetBold(true);
        m_BtnConvert.SetBold(true);

        this->DragAcceptFiles(TRUE);
        this->LoadConfig();
        this->UpdateFormatComboBox();
        this->UpdatePresetComboBox();
        this->UpdateOutputsComboBox();
        this->RedrawItems();
        this->UpdateStatusBar();
        this->SetOptions();
        this->InitLanguageMenu();
        this->SetLanguage();

        return TRUE;
    }

    void CMainDlg::OnOK()
    {
    }

    void CMainDlg::OnCancel()
    {
    }

    BOOL CMainDlg::PreTranslateMessage(MSG* pMsg)
    {
        if (pMsg->message == WM_KEYDOWN && pMsg->hwnd == m_EdtItem.m_hWnd)
        {
            if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
            {
                UpdateEdtItem(pMsg->wParam == VK_RETURN);
                return TRUE;
            }
        }

        if (pMsg->message == WM_MOUSEWHEEL && pMsg->hwnd == m_LstInputItems.m_hWnd)
        {
            UpdateEdtItem(FALSE);
        }

        if (m_hAccel != nullptr)
        {
            if (::TranslateAccelerator(this->GetSafeHwnd(), m_hAccel, pMsg))
                return TRUE;
        }

        return CMyDialogEx::PreTranslateMessage(pMsg);
    }

    void CMainDlg::OnPaint()
    {
        if (IsIconic())
        {
            CPaintDC dc(this);
            SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
            int cxIcon = GetSystemMetrics(SM_CXICON);
            int cyIcon = GetSystemMetrics(SM_CYICON);
            CRect rect;
            GetClientRect(&rect);
            int x = (rect.Width() - cxIcon + 1) / 2;
            int y = (rect.Height() - cyIcon + 1) / 2;
            dc.DrawIcon(x, y, m_hIcon);
        }
        else
        {
            CMyDialogEx::OnPaint();
        }
    }

    HCURSOR CMainDlg::OnQueryDragIcon()
    {
        return static_cast<HCURSOR>(m_hIcon);
    }

    void CMainDlg::OnClose()
    {
        CMyDialogEx::OnClose();

        this->GetOptions();

        if (this->m_Config.m_Options.bDoNotSaveConfiguration == false)
        {
            this->SaveConfig();
        }

        CMyDialogEx::OnOK();
    }

    void CMainDlg::OnDestroy()
    {
        CMyDialogEx::OnDestroy();
    }

    void CMainDlg::OnDropFiles(HDROP hDropInfo)
    {
        if (this->ctx->bRunning == true)
            return;

        static volatile bool bSafeCheck = false;
        static HDROP hDropInfoLocal;
        static int nFormat;
        static int nPreset;

        if (bSafeCheck == true)
            return;

        hDropInfoLocal = hDropInfo;
        nFormat = this->m_CmbFormat.GetCurSel();
        nPreset = this->m_CmbPresets.GetCurSel();

        static auto HandleFile = [&](const std::wstring& file)
        {
            std::wstring szExt = util::GetFileExtension(file);
            if (util::string::CompareNoCase(szExt, L"xml"))
            {
                config::xml::XmlDocumnent doc;
                std::string szName = config::xml::XmlConfig::GetRootName(file, doc);
                if (!szName.empty())
                {
                    if (util::string::CompareNoCase(szName, "Items"))
                        this->LoadItems(doc);
                    else if (util::string::CompareNoCase(szName, "Format"))
                        this->LoadFormat(doc);
                    else if (util::string::CompareNoCase(szName, "Presets"))
                        this->LoadPresets(doc);
                    else if (util::string::CompareNoCase(szName, "Outputs"))
                        this->LoadOutputs(doc);
                    else if (util::string::CompareNoCase(szName, "Options"))
                        this->LoadOptions(doc);
                    else if (util::string::CompareNoCase(szName, "Tool"))
                        this->LoadTool(doc);
                    else if (util::string::CompareNoCase(szName, "Language"))
                        this->LoadLanguage(doc);
                }
            }
            else if (util::string::CompareNoCase(szExt, L"exe"))
            {
                if (nFormat != -1)
                {
                    config::CFormat& format = m_Config.m_Formats[nFormat];
                    format.szPath = file;
                }
            }
            else if (util::string::CompareNoCase(szExt, L"lua"))
            {
                if (nFormat != -1)
                {
                    config::CFormat& format = m_Config.m_Formats[nFormat];
                    format.szFunction = file;
                }
            }
            else
            {
                this->AddToList(file, nFormat, nPreset);
            }
        };

        static auto HandleFinish = [&]()
        {
            this->RedrawItems();
            this->UpdateStatusBar();
        };

        auto m_DropThread = std::thread([&]()
        {
            if (bSafeCheck == true)
                return;

            bSafeCheck = true;

            int nCount = ::DragQueryFile(hDropInfoLocal, (UINT)0xFFFFFFFF, nullptr, 0);
            if (nCount > 0)
            {
                for (int i = 0; i < nCount; i++)
                {
                    int nReqChars = ::DragQueryFile(hDropInfoLocal, i, nullptr, 0);
                    CString szFile;
                    ::DragQueryFile(hDropInfoLocal, i, szFile.GetBuffer(nReqChars * 2 + 8), nReqChars * 2 + 8);
                    if (::GetFileAttributes(szFile) & FILE_ATTRIBUTE_DIRECTORY)
                    {
                        std::wstring szPath = szFile;
                        std::vector<std::wstring> files;
                        if (util::FindFiles(szPath, files, true) == true)
                        {
                            for (auto& file : files)
                                HandleFile(file);
                        }
                    }
                    else
                    {
                        std::wstring file = szFile;
                        HandleFile(file);
                    }
                    szFile.ReleaseBuffer();
                }
                HandleFinish();
            }
            ::DragFinish(hDropInfoLocal);

            bSafeCheck = false;
        });
        m_DropThread.detach();

        CMyDialogEx::OnDropFiles(hDropInfoLocal);
    }

    BOOL CMainDlg::OnHelpInfo(HELPINFO* pHelpInfo)
    {
        return FALSE;
    }

    LRESULT CMainDlg::OnListItemChaged(WPARAM wParam, LPARAM lParam)
    {
        if (this->ctx->bRunning == false)
        {
            INT nIndex = (INT)wParam;
            LPTSTR szText = (LPTSTR)lParam;
            if ((nIndex >= 0) && szText != nullptr)
            {
                config::CItem& item = m_Config.m_Items[nIndex];
                item.szName = szText;
            }
        }
        return(0);
    }

    LRESULT CMainDlg::OnNotifyFormat(WPARAM wParam, LPARAM lParam)
    {
#ifdef _UNICODE
        return NFR_UNICODE;
#else
        return NFR_ANSI;
#endif
    }

    void CMainDlg::OnLvnGetdispinfoListItems(NMHDR* pNMHDR, LRESULT* pResult)
    {
        NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
        LV_ITEM* pItem = &(pDispInfo)->item;
        int nItem = pItem->iItem;

        if (pItem->mask & LVIF_TEXT)
        {
            config::CItem& item = m_Config.m_Items[nItem];
            std::wstring szText;

            switch (pItem->iSubItem)
            {
            case ITEM_COLUMN_NAME:
                szText = item.szName;
                break;
            case ITEM_COLUMN_INPUT:
                szText = item.szExtension;
                break;
            case ITEM_COLUMN_SIZE:
                szText = std::to_wstring(item.nSize);
                break;
            case ITEM_COLUMN_OUTPUT:
                szText = item.szFormatId;
                break;
            case ITEM_COLUMN_PRESET:
                szText = std::to_wstring(item.nPreset);
                break;
            case ITEM_COLUMN_OPTIONS:
                szText = item.szOptions;
                break;
            case ITEM_COLUMN_TIME:
                szText = item.szTime.empty() ? m_Config.GetString(0x00150001) : item.szTime;
                break;
            case ITEM_COLUMN_STATUS:
                szText = item.szStatus.empty() ? m_Config.GetString(0x00210001) : item.szStatus;
                break;
            }

            _tcscpy_s(pItem->pszText, pItem->cchTextMax, szText.c_str());
        }

        if (pItem->mask & LVIF_IMAGE)
        {
            config::CItem& item = m_Config.m_Items[nItem];

            pItem->mask |= LVIF_STATE;
            pItem->stateMask = LVIS_STATEIMAGEMASK;
            pItem->state = item.bChecked ? INDEXTOSTATEIMAGEMASK(2) : INDEXTOSTATEIMAGEMASK(1);
        }

        *pResult = 0;
    }

    void CMainDlg::OnOdfindListItems(NMHDR* pNMHDR, LRESULT* pResult)
    {
        if (this->ctx->bRunning == false)
        {
            NMLVFINDITEM* pFindInfo = (NMLVFINDITEM*)pNMHDR;
            *pResult = -1;
            if ((pFindInfo->lvfi.flags & LVFI_STRING) == 0)
            {
                return;
            }

            CString szSearchStr = pFindInfo->lvfi.psz;
            int startPos = pFindInfo->iStart;
            if (startPos >= m_LstInputItems.GetItemCount())
                startPos = 0;

            int currentPos = startPos;
            do
            {
                config::CItem& item = m_Config.m_Items[currentPos];
                if (_tcsnicmp(item.szName.c_str(), szSearchStr, szSearchStr.GetLength()) == 0)
                {
                    *pResult = currentPos;
                    break;
                }
                currentPos++;
                if (currentPos >= m_LstInputItems.GetItemCount())
                    currentPos = 0;
            } while (currentPos != startPos);
        }
    }

    void CMainDlg::OnNMClickListItems(NMHDR *pNMHDR, LRESULT *pResult)
    {
        if (this->ctx->bRunning == false)
        {
            NMLISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
            LVHITTESTINFO hitInfo;
            hitInfo.pt = pNMListView->ptAction;
            int nItem = m_LstInputItems.HitTest(&hitInfo);
            if (nItem != -1)
            {
                if (hitInfo.flags == LVHT_ONITEMSTATEICON)
                {
                    ToggleItem(nItem);
                }
            }

            LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
            if (pNMItemActivate->iSubItem == -1 || pNMItemActivate->iItem == -1)
            {
                UpdateEdtItem(TRUE);
                *pResult = 0;
                return;
            }

            if (pNMItemActivate->iSubItem == 0 || pNMItemActivate->iSubItem != ITEM_COLUMN_OPTIONS)
            {
                UpdateEdtItem(TRUE);
                *pResult = 0;
                return;
            }

            UpdateEdtItem(TRUE);

            nEdtItem = pNMItemActivate->iItem;
            nEdtSubItem = pNMItemActivate->iSubItem;

            config::CItem& item = m_Config.m_Items[nEdtItem];
            szEdtText = item.szOptions.c_str();

            ShowEdtItem();
        }

        *pResult = 0;
    }

    void CMainDlg::OnEnKillfocusEditItem()
    {
        if (this->ctx->bRunning == false)
        {
            UpdateEdtItem(TRUE);
        }
    }

    void CMainDlg::OnLvnKeydownListInputItems(NMHDR *pNMHDR, LRESULT *pResult)
    {
        if (this->ctx->bRunning == false)
        {
            LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast<LPNMLVKEYDOWN>(pNMHDR);
            switch (pLVKeyDow->wVKey)
            {
            case VK_SPACE:
                this->OnEditToggleSelected();
                break;
            case VK_INSERT:
                this->OnEditCrop();
                break;
            case VK_DELETE:
                this->OnEditRemove();
                break;
            default: break;
            };
        }

        *pResult = 0;
    }

    void CMainDlg::OnNMRclickListInputItems(NMHDR *pNMHDR, LRESULT *pResult)
    {
        if (this->ctx->bRunning == false)
        {
            POINT point;
            GetCursorPos(&point);

            CMenu *m_hMenu = this->GetMenu();
            CMenu *m_hEditMenu = m_hMenu->GetSubMenu(1);
            m_hEditMenu->TrackPopupMenu(0, point.x, point.y, this, nullptr);
            m_hEditMenu = nullptr;
            m_hMenu = nullptr;
        }
        *pResult = 0;
    }

    void CMainDlg::OnNMDblclkListInputItems(NMHDR *pNMHDR, LRESULT *pResult)
    {
        if (this->ctx->bRunning == false)
        {
            POSITION pos = m_LstInputItems.GetFirstSelectedItemPosition();
            if (pos != nullptr)
            {
                size_t nItem = this->m_LstInputItems.GetNextSelectedItem(pos);
                if (nItem < m_Config.m_Items.size())
                {
                    config::CItem& item = m_Config.m_Items[nItem];

                    CPathsDlg dlg;
                    dlg.pConfig = &m_Config;
                    dlg.m_Paths = item.m_Paths;
                    dlg.szPathsDialogResize = m_Config.m_Options.szPathsDialogResize;
                    dlg.szPathsListColumns = m_Config.m_Options.szPathsListColumns;

                    INT_PTR nRet = dlg.DoModal();
                    if (nRet == IDOK)
                    {
                        item.m_Paths = std::move(dlg.m_Paths);

                        item.nSize = 0;
                        for (auto& path : item.m_Paths)
                            item.nSize += path.nSize;

                        this->RedrawItems();
                        this->UpdateStatusBar();
                    }

                    m_Config.m_Options.szPathsDialogResize = dlg.szPathsDialogResize;
                    m_Config.m_Options.szPathsListColumns = dlg.szPathsListColumns;
                }
            }
        }
        *pResult = 0;
    }

    void CMainDlg::OnLvnItemchangingListInputItems(NMHDR* pNMHDR, LRESULT* pResult)
    {
        if (this->ctx->bRunning == true)
        {
            *pResult = true;
            return;
        }
        *pResult = false;
    }

    void CMainDlg::OnLvnItemchangedListInputItems(NMHDR *pNMHDR, LRESULT *pResult)
    {
        if (this->ctx->bRunning == false)
        {
            LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

            int nSelCount = this->m_LstInputItems.GetSelectedCount();
            if (nSelCount == 1)
            {
                POSITION pos = m_LstInputItems.GetFirstSelectedItemPosition();
                if (pos != nullptr)
                {
                    size_t nItem = this->m_LstInputItems.GetNextSelectedItem(pos);
                    if (nItem < m_Config.m_Items.size())
                    {
                        config::CItem& item = m_Config.m_Items[nItem];
                        if (m_Config.m_Formats.size() > 0)
                        {
                            config::CFormat& format = m_Config.m_Formats[this->m_CmbFormat.GetCurSel()];
                            if (util::string::CompareNoCase(item.szFormatId, format.szId))
                            {
                                format.nDefaultPreset = item.nPreset;
                                this->m_CmbPresets.SetCurSel(item.nPreset);
                            }
                            else
                            {
                                int nFormat = config::CFormat::GetFormatById(m_Config.m_Formats, item.szFormatId);
                                if (nFormat >= 0)
                                {
                                    config::CFormat& format = m_Config.m_Formats[nFormat];

                                    m_Config.m_Options.nSelectedFormat = nFormat;
                                    format.nDefaultPreset = item.nPreset;

                                    this->UpdateFormatComboBox();
                                    this->UpdatePresetComboBox();
                                }
                            }
                        }
                    }
                }
            }
        }
        *pResult = 0;
    }

    void CMainDlg::OnCbnSelchangeComboPresets()
    {
        if (this->ctx->bRunning == false)
        {
            int nFormat = this->m_CmbFormat.GetCurSel();
            int nPreset = this->m_CmbPresets.GetCurSel();

            config::CFormat& format = m_Config.m_Formats[nFormat];
            format.nDefaultPreset = nPreset;

            this->UpdateFormatAndPreset();
        }
    }

    void CMainDlg::OnCbnSelchangeComboFormat()
    {
        if (this->ctx->bRunning == false)
        {
            int nFormat = this->m_CmbFormat.GetCurSel();
            if (nFormat != -1)
            {
                m_Config.m_Options.nSelectedFormat = nFormat;
                this->UpdatePresetComboBox();
            }

            this->UpdateFormatAndPreset();
        }
    }

    void CMainDlg::OnBnClickedButtonBrowsePath()
    {
        if (this->ctx->bRunning == false)
        {
            LPMALLOC pMalloc;
            BROWSEINFO bi;
            LPITEMIDLIST pidlDesktop;
            LPITEMIDLIST pidlBrowse;
            TCHAR *lpBuffer;

            if (SHGetMalloc(&pMalloc) == E_FAIL)
                return;

            if ((lpBuffer = (TCHAR *)pMalloc->Alloc(MAX_PATH * 2)) == nullptr)
            {
                pMalloc->Release();
                return;
            }

            if (!SUCCEEDED(::SHGetSpecialFolderLocation(this->GetSafeHwnd(), CSIDL_DESKTOP, &pidlDesktop)))
            {
                pMalloc->Free(lpBuffer);
                pMalloc->Release();
                return;
            }

            static std::wstring szTitle;
            szTitle = m_Config.GetString(0x00210006);

            bi.hwndOwner = this->GetSafeHwnd();
            bi.pidlRoot = pidlDesktop;
            bi.pszDisplayName = lpBuffer;
            bi.lpszTitle = szTitle.c_str();
            bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
            bi.iImage = 0;
            bi.lpfn = BrowseCallbackOutPath;
            bi.lParam = reinterpret_cast<LPARAM>(this);

            pidlBrowse = ::SHBrowseForFolder(&bi);
            if (pidlBrowse != nullptr)
            {
                if (::SHGetPathFromIDList(pidlBrowse, lpBuffer))
                {
                    szLastOutputBrowse.Format(_T("%s\0"), lpBuffer);

                    CString szOutPath;
                    szOutPath.Format(_T("%s\\%s.%s\0"), lpBuffer, VAR_OUTPUT_NAME, VAR_OUTPUT_EXT);
                    this->m_CmbOutPath.SetWindowText(szOutPath);
                }
                pMalloc->Free(pidlBrowse);
            }

            pMalloc->Free(pidlDesktop);
            pMalloc->Free(lpBuffer);
            pMalloc->Release();
        }
    }

    void CMainDlg::OnBnClickedButtonConvert()
    {
        this->StartConvert();
    }

    void CMainDlg::OnFileLoadList()
    {
        if (this->ctx->bRunning == false)
        {
            CString szFilter;
            szFilter.Format(_T("%s (*.xml)|*.xml|%s (*.*)|*.*||"),
                m_Config.GetString(0x00310003).c_str(),
                m_Config.GetString(0x00310001).c_str());

            CFileDialog fd(TRUE, _T("xml"), _T(""),
                OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_EXPLORER,
                szFilter, this);

            if (fd.DoModal() == IDOK)
            {
                std::wstring szFileXml = fd.GetPathName();
                if (this->LoadItems(szFileXml) == false)
                    m_StatusBar.SetText(m_Config.GetString(0x00210007).c_str(), 1, 0);
                else
                    this->UpdateStatusBar();
            }
        }
    }

    void CMainDlg::OnFileSaveList()
    {
        if (this->ctx->bRunning == false)
        {
            CString szFilter;
            szFilter.Format(_T("%s (*.xml)|*.xml|%s (*.*)|*.*||"),
                m_Config.GetString(0x00310003).c_str(),
                m_Config.GetString(0x00310001).c_str());

            CFileDialog fd(FALSE, _T("xml"), _T("Items"),
                OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_EXPLORER | OFN_OVERWRITEPROMPT,
                szFilter, this);

            if (fd.DoModal() == IDOK)
            {
                std::wstring szFileXml = fd.GetPathName();
                if (this->SaveItems(szFileXml) == false)
                    m_StatusBar.SetText(m_Config.GetString(0x00210008).c_str(), 1, 0);
            }
        }
    }

    void CMainDlg::OnFileClearList()
    {
        if (this->ctx->bRunning == false)
        {
            m_Config.m_Items = std::vector<config::CItem>();
            this->RedrawItems();
            this->UpdateStatusBar();
        }
    }

    void CMainDlg::OnFileExit()
    {
        if (this->ctx->bRunning == false)
        {
            this->OnClose();
        }
    }

    void CMainDlg::OnEditAddFiles()
    {
        if (this->ctx->bRunning == false)
        {
            try
            {
                int nSize = (768 * (MAX_PATH + 1)) + 1;
                auto buffer = std::make_unique<TCHAR[]>(nSize);

                CString szFilter;
                szFilter.Format(_T("%s (*.*)|*.*||"),
                    m_Config.GetString(0x00310001).c_str());

                CFileDialog fd(TRUE, _T(""), _T(""),
                    OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_ENABLESIZING,
                    szFilter,
                    this);

                fd.m_ofn.lpstrFile = buffer.get();
                fd.m_ofn.nMaxFile = nSize;

                if (fd.DoModal() != IDCANCEL)
                {
                    int nFormat = this->m_CmbFormat.GetCurSel();
                    int nPreset = this->m_CmbPresets.GetCurSel();
                    POSITION pos = fd.GetStartPosition();
                    do
                    {
                        std::wstring szFilePath = fd.GetNextPathName(pos);
                        if (!szFilePath.empty())
                        {
                            this->AddToList(szFilePath, nFormat, nPreset);
                        }
                    } while (pos != nullptr);

                    this->RedrawItems();
                    this->UpdateStatusBar();
                }
            }
            catch (...) { }
        }
    }

    void CMainDlg::OnEditAddDir()
    {
        if (this->ctx->bRunning == false)
        {
            LPMALLOC pMalloc;
            BROWSEINFO bi;
            LPITEMIDLIST pidlDesktop;
            LPITEMIDLIST pidlBrowse;
            TCHAR *lpBuffer;

            if (SHGetMalloc(&pMalloc) == E_FAIL)
                return;

            if ((lpBuffer = (TCHAR *)pMalloc->Alloc(MAX_PATH * 2)) == nullptr)
            {
                pMalloc->Release();
                return;
            }

            if (!SUCCEEDED(::SHGetSpecialFolderLocation(this->GetSafeHwnd(), CSIDL_DESKTOP, &pidlDesktop)))
            {
                pMalloc->Free(lpBuffer);
                pMalloc->Release();
                return;
            }

            static std::wstring szTitle;
            szTitle = m_Config.GetString(0x0021000A);

            bi.hwndOwner = this->GetSafeHwnd();
            bi.pidlRoot = pidlDesktop;
            bi.pszDisplayName = lpBuffer;
            bi.lpszTitle = szTitle.c_str();
            bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
            bi.iImage = 0;
            bi.lpfn = BrowseCallbackAddDir;
            bi.lParam = reinterpret_cast<LPARAM>(this);

            pidlBrowse = ::SHBrowseForFolder(&bi);
            if (pidlBrowse != nullptr)
            {
                if (::SHGetPathFromIDList(pidlBrowse, lpBuffer))
                {
                    szLastDirectoryBrowse.Format(_T("%s\0"), lpBuffer);

                    std::wstring szPath = std::wstring(lpBuffer);
                    std::vector<std::wstring> files;
                    bool bResult = util::FindFiles(szPath, files, bRecurseChecked);
                    if (bResult == true)
                    {
                        int nFormat = this->m_CmbFormat.GetCurSel();
                        int nPreset = this->m_CmbPresets.GetCurSel();
                        for (auto& file : files)
                        {
                            this->AddToList(file, nFormat, nPreset);
                        }
                        this->RedrawItems();
                        this->UpdateStatusBar();
                    }
                    else
                    {
                        m_StatusBar.SetText(m_Config.GetString(0x0021000C).c_str(), 1, 0);
                    }
                }
                pMalloc->Free(pidlBrowse);
            }

            pMalloc->Free(pidlDesktop);
            pMalloc->Free(lpBuffer);
            pMalloc->Release();
        }
    }

    void CMainDlg::OnEditRename()
    {
        if (this->ctx->bRunning == false)
        {
            if (m_LstInputItems.GetFocus()->GetSafeHwnd() != m_LstInputItems.GetSafeHwnd())
                return;

            POSITION pos = m_LstInputItems.GetFirstSelectedItemPosition();
            if (pos != nullptr)
            {
                this->m_LstInputItems.SetFocus();
                int nItem = m_LstInputItems.GetNextSelectedItem(pos);
                this->m_LstInputItems.EditLabel(nItem);
            }
        }
    }

    void CMainDlg::OnEditResetTime()
    {
        if (this->ctx->bRunning == false)
        {
            this->m_StatusBar.SetText(L"", 1, 0);
            this->ResetConvertionTime();
            this->ResetConvertionStatus();
        }
    }

    void CMainDlg::OnEditResetOutput()
    {
        if (this->ctx->bRunning == false)
        {
            this->ResetOutput();
        }
    }

    void CMainDlg::OnEditRemove()
    {
        if (this->ctx->bRunning == false)
        {
            size_t nItems = m_Config.m_Items.size();
            if (nItems <= 0)
                return;

            size_t nItemLastRemoved = -1;
            std::vector<int> keep;
            for (size_t i = 0; i < nItems; i++)
            {
                if (!this->IsItemSelected(i))
                    keep.emplace_back(i);
                else
                    nItemLastRemoved = i;
            }

            m_Config.RemoveItems(keep);

            this->RedrawItems();

            nItems = m_Config.m_Items.size();

            if (nItemLastRemoved != -1)
            {
                if (nItemLastRemoved < nItems && nItems >= 0)
                {
                    this->SelectItem(nItemLastRemoved);
                    this->MakeItemVisible(nItemLastRemoved);
                }
                else if (nItemLastRemoved >= nItems && nItems >= 0)
                {
                    this->SelectItem(nItemLastRemoved - 1);
                    this->MakeItemVisible(nItemLastRemoved - 1);
                }
            }

            if (nItems == 0)
            {
                m_Config.m_Items = std::vector<config::CItem>();
                this->RedrawItems();
            }

            this->UpdateStatusBar();
        }
    }

    void CMainDlg::OnEditCrop()
    {
        if (this->ctx->bRunning == false)
        {
            size_t nItems = m_Config.m_Items.size();
            for (size_t i = 0; i < nItems; i++)
            {
                if (this->IsItemSelected(i))
                    this->DeselectItem(i);
                else
                    this->SelectItem(i);
            }
            OnEditRemove();
        }
    }

    void CMainDlg::OnEditRemoveChecked()
    {
        if (this->ctx->bRunning == false)
        {
            size_t nItems = m_Config.m_Items.size();
            if (nItems <= 0)
                return;

            std::vector<int> keep;
            for (size_t i = 0; i < nItems; i++)
            {
                config::CItem& item = m_Config.m_Items[i];
                if (item.bChecked == false)
                    keep.emplace_back(i);
            }

            m_Config.RemoveItems(keep);

            this->RedrawItems();

            nItems = m_Config.m_Items.size();
            if (nItems == 0)
            {
                m_Config.m_Items = std::vector<config::CItem>();
                this->RedrawItems();
            }
            this->UpdateStatusBar();
        }
    }

    void CMainDlg::OnEditRemoveUnchecked()
    {
        if (this->ctx->bRunning == false)
        {
            size_t nItems = m_Config.m_Items.size();
            if (nItems <= 0)
                return;

            std::vector<int> keep;
            for (size_t i = 0; i < nItems; i++)
            {
                config::CItem& item = m_Config.m_Items[i];
                if (item.bChecked == true)
                    keep.emplace_back(i);
            }

            m_Config.RemoveItems(keep);

            this->RedrawItems();

            nItems = m_Config.m_Items.size();
            if (nItems == 0)
            {
                m_Config.m_Items = std::vector<config::CItem>();
                this->RedrawItems();
            }
            this->UpdateStatusBar();
        }
    }

    void CMainDlg::OnEditCheckSelected()
    {
        if (this->ctx->bRunning == false)
        {
            size_t nItems = m_Config.m_Items.size();
            if (nItems > 0)
            {
                for (size_t i = 0; i < nItems; i++)
                {
                    if (this->IsItemSelected(i))
                    {
                        config::CItem& item = m_Config.m_Items[i];
                        if (item.bChecked == false)
                            item.bChecked = true;
                    }
                }
                this->RedrawItems();
                this->UpdateStatusBar();
            }
        }
    }

    void CMainDlg::OnEditUncheckSelected()
    {
        if (this->ctx->bRunning == false)
        {
            size_t nItems = m_Config.m_Items.size();
            if (nItems > 0)
            {
                for (size_t i = 0; i < nItems; i++)
                {
                    if (this->IsItemSelected(i))
                    {
                        config::CItem& item = m_Config.m_Items[i];
                        if (item.bChecked == true)
                        {
                            item.bChecked = false;
                        }
                    }
                }
                this->RedrawItems();
                this->UpdateStatusBar();
            }
        }
    }

    void CMainDlg::OnEditToggleSelected()
    {
        if (this->ctx->bRunning == false)
        {
            size_t nItems = m_Config.m_Items.size();
            if (nItems > 0)
            {
                for (size_t i = 0; i < nItems; i++)
                {
                    if (this->IsItemSelected(i))
                    {
                        config::CItem& item = m_Config.m_Items[i];
                        item.bChecked = !item.bChecked;
                    }
                }
                this->RedrawItems();
                this->UpdateStatusBar();
            }
        }
    }

    void CMainDlg::OnEditSelectAll()
    {
        if (this->ctx->bRunning == false)
        {
            m_LstInputItems.SetItemState(-1, LVIS_SELECTED, LVIS_SELECTED);
        }
    }

    void CMainDlg::OnEditSelectNone()
    {
        if (this->ctx->bRunning == false)
        {
            m_LstInputItems.SetItemState(-1, 0, LVIS_SELECTED);
        }
    }

    void CMainDlg::OnEditInvertSelection()
    {
        if (this->ctx->bRunning == false)
        {
            size_t nItems = m_Config.m_Items.size();
            if (nItems > 0)
            {
                for (size_t i = 0; i < nItems; i++)
                {
                    if (this->IsItemSelected(i))
                        this->DeselectItem(i);
                    else
                        this->SelectItem(i);
                }
            }
        }
    }

    void CMainDlg::OnEditOpen()
    {
        if (this->ctx->bRunning == false)
        {
            POSITION pos = m_LstInputItems.GetFirstSelectedItemPosition();
            if (pos != nullptr)
            {
                int nItem = m_LstInputItems.GetNextSelectedItem(pos);
                config::CItem& item = m_Config.m_Items[nItem];
                if (item.m_Paths.size() > 0)
                {
                    config::CPath& path = item.m_Paths[0];
                    util::LaunchAndWait(path.szPath, L"", FALSE);
                }
            }
        }
    }

    void CMainDlg::OnEditExplore()
    {
        if (this->ctx->bRunning == false)
        {
            POSITION pos = m_LstInputItems.GetFirstSelectedItemPosition();
            if (pos != nullptr)
            {
                int nItem = m_LstInputItems.GetNextSelectedItem(pos);
                config::CItem& item = m_Config.m_Items[nItem];
                if (item.m_Paths.size() > 0)
                {
                    config::CPath& path = item.m_Paths[0];
                    util::LaunchAndWait(util::GetFilePath(path.szPath), L"", FALSE);
                }
            }
        }
    }

    void CMainDlg::OnActionConvert()
    {
        this->StartConvert();
    }

    void CMainDlg::OnOptionsConfigurePresets()
    {
        if (this->ctx->bRunning == false)
        {
            CPresetsDlg dlg;
            dlg.pConfig = &m_Config;
            dlg.nSelectedFormat = this->m_CmbFormat.GetCurSel();
            dlg.m_Formats = m_Config.m_Formats;
            dlg.szPresetsDialogResize = m_Config.m_Options.szPresetsDialogResize;
            dlg.szPresetsListColumns = m_Config.m_Options.szPresetsListColumns;

            INT_PTR nRet = dlg.DoModal();
            if (nRet == IDOK)
            {
                m_Config.m_Formats = std::move(dlg.m_Formats);

                this->UpdateFormatComboBox();
                this->UpdatePresetComboBox();
            }

            m_Config.m_Options.szPresetsDialogResize = dlg.szPresetsDialogResize;
            m_Config.m_Options.szPresetsListColumns = dlg.szPresetsListColumns;
        }
    }

    void CMainDlg::OnOptionsConfigureFormat()
    {
        if (this->ctx->bRunning == false)
        {
            CFormatsDlg dlg;
            dlg.pConfig = &m_Config;
            dlg.nSelectedFormat = this->m_CmbFormat.GetCurSel();
            dlg.m_Formats = m_Config.m_Formats;
            dlg.szFormatsDialogResize = m_Config.m_Options.szFormatsDialogResize;
            dlg.szFormatsListColumns = m_Config.m_Options.szFormatsListColumns;

            INT_PTR nRet = dlg.DoModal();
            if (nRet == IDOK)
            {
                m_Config.m_Formats = std::move(dlg.m_Formats);

                if (dlg.nSelectedFormat >= 0)
                    m_Config.m_Options.nSelectedFormat = dlg.nSelectedFormat;

                this->UpdateFormatComboBox();
                this->UpdatePresetComboBox();
            }

            m_Config.m_Options.szFormatsDialogResize = dlg.szFormatsDialogResize;
            m_Config.m_Options.szFormatsListColumns = dlg.szFormatsListColumns;
        }
    }

    void CMainDlg::OnOptionsConfigureTools()
    {
        if (this->ctx->bRunning == false)
        {
            int nSelectedTool = 0;
            int nFormat = this->m_CmbFormat.GetCurSel();
            if (nFormat >= 0)
            {
#if defined(_WIN32) & !defined(_WIN64)
                const std::wstring szPlatform = L"x86";
#else
                const std::wstring szPlatform = L"x64";
#endif
                config::CFormat& format = m_Config.m_Formats[nFormat];
                int nTool = config::CTool::GetToolByFormatAndPlatform(m_Config.m_Tools, format.szId, szPlatform);
                if (nTool >= 0)
                    nSelectedTool = nTool;
            }

            CToolsDlg dlg;
            dlg.pConfig = &m_Config;
            dlg.nSelectedTool = nSelectedTool;
            dlg.m_Tools = m_Config.m_Tools;
            dlg.m_Formats = m_Config.m_Formats;
            dlg.szToolsDialogResize = m_Config.m_Options.szToolsDialogResize;
            dlg.szToolsListColumns = m_Config.m_Options.szToolsListColumns;

            INT_PTR nRet = dlg.DoModal();
            if (nRet == IDOK)
            {
                m_Config.m_Tools = std::move(dlg.m_Tools);
                m_Config.m_Formats = std::move(dlg.m_Formats);

                this->UpdateFormatComboBox();
                this->UpdatePresetComboBox();
            }

            m_Config.m_Options.szToolsDialogResize = dlg.szToolsDialogResize;
            m_Config.m_Options.szToolsListColumns = dlg.szToolsListColumns;
        }
    }

    void CMainDlg::OnOptionsDeleteSource()
    {
        if (this->ctx->bRunning == false)
        {
            CMenu *m_hMenu = this->GetMenu();
            bool bChecked = m_hMenu->GetMenuState(ID_OPTIONS_DELETE_SOURCE, MF_BYCOMMAND) == MF_CHECKED;
            m_hMenu->CheckMenuItem(ID_OPTIONS_DELETE_SOURCE, (bChecked == true) ? MF_UNCHECKED : MF_CHECKED);
            m_hMenu = nullptr;

            this->m_Config.m_Options.bDeleteSourceFiles = !bChecked;
        }
    }

    void CMainDlg::OnOptionsShutdownWindows()
    {
        if (this->ctx->bRunning == false)
        {
            CMenu *m_hMenu = this->GetMenu();
            bool bChecked = m_hMenu->GetMenuState(ID_OPTIONS_SHUTDOWN_WINDOWS, MF_BYCOMMAND) == MF_CHECKED;
            m_hMenu->CheckMenuItem(ID_OPTIONS_SHUTDOWN_WINDOWS, (bChecked == true) ? MF_UNCHECKED : MF_CHECKED);
            m_hMenu = nullptr;

            this->m_Config.m_Options.bShutdownWhenFinished = !bChecked;
        }
    }

    void CMainDlg::OnOptionsDoNotSave()
    {
        if (this->ctx->bRunning == false)
        {
            CMenu *m_hMenu = this->GetMenu();
            bool bChecked = m_hMenu->GetMenuState(ID_OPTIONS_DO_NOT_SAVE, MF_BYCOMMAND) == MF_CHECKED;
            m_hMenu->CheckMenuItem(ID_OPTIONS_DO_NOT_SAVE, (bChecked == true) ? MF_UNCHECKED : MF_CHECKED);
            m_hMenu = nullptr;

            this->m_Config.m_Options.bDoNotSaveConfiguration = !bChecked;
        }
    }

    void CMainDlg::OnOptionsDeleteOnErrors()
    {
        if (this->ctx->bRunning == false)
        {
            CMenu *m_hMenu = this->GetMenu();
            bool bChecked = m_hMenu->GetMenuState(ID_OPTIONS_DELETE_ON_ERRORS, MF_BYCOMMAND) == MF_CHECKED;
            m_hMenu->CheckMenuItem(ID_OPTIONS_DELETE_ON_ERRORS, (bChecked == true) ? MF_UNCHECKED : MF_CHECKED);
            m_hMenu = nullptr;

            this->m_Config.m_Options.bDeleteOnErrors = !bChecked;
        }
    }

    void CMainDlg::OnOptionsStopOnErrors()
    {
        if (this->ctx->bRunning == false)
        {
            CMenu *m_hMenu = this->GetMenu();
            bool bChecked = m_hMenu->GetMenuState(ID_OPTIONS_STOP_ON_ERRORS, MF_BYCOMMAND) == MF_CHECKED;
            m_hMenu->CheckMenuItem(ID_OPTIONS_STOP_ON_ERRORS, (bChecked == true) ? MF_UNCHECKED : MF_CHECKED);
            m_hMenu = nullptr;

            this->m_Config.m_Options.bStopOnErrors = !bChecked;
        }
    }

    void CMainDlg::OnOptionsHideConsole()
    {
        if (this->ctx->bRunning == false)
        {
            CMenu *m_hMenu = this->GetMenu();
            bool bChecked = m_hMenu->GetMenuState(ID_OPTIONS_HIDE_CONSOLE, MF_BYCOMMAND) == MF_CHECKED;
            m_hMenu->CheckMenuItem(ID_OPTIONS_HIDE_CONSOLE, (bChecked == true) ? MF_UNCHECKED : MF_CHECKED);
            m_hMenu = nullptr;

            this->m_Config.m_Options.bHideConsoleWindow = !bChecked;
        }
    }

    void CMainDlg::OnOptionsEnsureVisible()
    {
        if (this->ctx->bRunning == false)
        {
            CMenu *m_hMenu = this->GetMenu();
            bool bChecked = m_hMenu->GetMenuState(ID_OPTIONS_ENSURE_VISIBLE, MF_BYCOMMAND) == MF_CHECKED;
            m_hMenu->CheckMenuItem(ID_OPTIONS_ENSURE_VISIBLE, (bChecked == true) ? MF_UNCHECKED : MF_CHECKED);
            m_hMenu = nullptr;

            this->m_Config.m_Options.bEnsureItemIsVisible = !bChecked;
        }
    }

    void CMainDlg::OnOptionsFindDecoder()
    {
        if (this->ctx->bRunning == false)
        {
            CMenu *m_hMenu = this->GetMenu();
            bool bChecked = m_hMenu->GetMenuState(ID_OPTIONS_FIND_DECODER, MF_BYCOMMAND) == MF_CHECKED;
            m_hMenu->CheckMenuItem(ID_OPTIONS_FIND_DECODER, (bChecked == true) ? MF_UNCHECKED : MF_CHECKED);
            m_hMenu = nullptr;

            this->m_Config.m_Options.bTryToFindDecoder = !bChecked;
        }
    }

    void CMainDlg::OnOptionsValidateFiles()
    {
        if (this->ctx->bRunning == false)
        {
            CMenu *m_hMenu = this->GetMenu();
            bool bChecked = m_hMenu->GetMenuState(ID_OPTIONS_VALIDATE_FILES, MF_BYCOMMAND) == MF_CHECKED;
            m_hMenu->CheckMenuItem(ID_OPTIONS_VALIDATE_FILES, (bChecked == true) ? MF_UNCHECKED : MF_CHECKED);
            m_hMenu = nullptr;

            this->m_Config.m_Options.bValidateInputFiles = !bChecked;
        }
    }

    void CMainDlg::OnOptionsOverwriteFiles()
    {
        if (this->ctx->bRunning == false)
        {
            CMenu *m_hMenu = this->GetMenu();
            bool bChecked = m_hMenu->GetMenuState(ID_OPTIONS_OVERWRITE_FILES, MF_BYCOMMAND) == MF_CHECKED;
            m_hMenu->CheckMenuItem(ID_OPTIONS_OVERWRITE_FILES, (bChecked == true) ? MF_UNCHECKED : MF_CHECKED);
            m_hMenu = nullptr;

            this->m_Config.m_Options.bOverwriteExistingFiles = !bChecked;
        }
    }

    void CMainDlg::OnOptionsRenameFiles()
    {
        if (this->ctx->bRunning == false)
        {
            CMenu *m_hMenu = this->GetMenu();
            bool bChecked = m_hMenu->GetMenuState(ID_OPTIONS_RENAME_FILES, MF_BYCOMMAND) == MF_CHECKED;
            m_hMenu->CheckMenuItem(ID_OPTIONS_RENAME_FILES, (bChecked == true) ? MF_UNCHECKED : MF_CHECKED);
            m_hMenu = nullptr;

            this->m_Config.m_Options.bRenameExistingFiles = !bChecked;
        }
    }

    void CMainDlg::OnOptionsDownloadTools()
    {
        if (this->ctx->bRunning == false)
        {
            CMenu *m_hMenu = this->GetMenu();
            bool bChecked = m_hMenu->GetMenuState(ID_OPTIONS_DOWNLOAD_TOOLS, MF_BYCOMMAND) == MF_CHECKED;
            m_hMenu->CheckMenuItem(ID_OPTIONS_DOWNLOAD_TOOLS, (bChecked == true) ? MF_UNCHECKED : MF_CHECKED);
            m_hMenu = nullptr;

            this->m_Config.m_Options.bTryToDownloadTools = !bChecked;
        }
    }

    void CMainDlg::OnLanguageDefault()
    {
    }

    void CMainDlg::OnLanguageChange(UINT nID)
    {
        int nSelectedLanguage = nID - ID_LANGUAGE_MIN;
        config::CLanguage& language = m_Config.m_Languages[nSelectedLanguage];
        m_Config.m_Options.szSelectedLanguage = language.szId;
        m_Config.nLangId = nSelectedLanguage;

        CMenu *m_hLangMenu = this->GetMenu()->GetSubMenu(4);
        size_t nLanguages = m_Config.m_Languages.size();
        for (size_t i = 0; i < nLanguages; i++)
        {
            UINT nLangID = ID_LANGUAGE_MIN + i;
            if (nLangID != nID)
                m_hLangMenu->CheckMenuItem(nLangID, MF_UNCHECKED);
        }

        m_hLangMenu->CheckMenuItem(nID, MF_CHECKED);
        m_hLangMenu = nullptr;

        this->SetLanguage();
        this->UpdateStatusBar();
    }

    void CMainDlg::OnHelpWebsite()
    {
        if (this->ctx->bRunning == false)
        {
            util::LaunchAndWait(L"https://github.com/wieslawsoltes/BatchEncoder", L"", FALSE);
        }
    }

    void CMainDlg::OnHelpAbout()
    {
        if (this->ctx->bRunning == false)
        {
            CAboutDlg dlg;
            dlg.pConfig = &m_Config;
            dlg.DoModal();
        }
    }

    void CMainDlg::InitLanguageMenu()
    {
        CMenu *m_hLangMenu = this->GetMenu()->GetSubMenu(4);
        size_t nLanguages = m_Config.m_Languages.size();
        if (nLanguages > 0 && nLanguages <= (ID_LANGUAGE_MAX - ID_LANGUAGE_MIN))
        {
            m_hLangMenu->DeleteMenu(ID_LANGUAGE_DEFAULT, 0);

            for (size_t i = 0; i < nLanguages; i++)
            {
                config::CLanguage& language = m_Config.m_Languages[i];
                std::wstring szText = language.szOriginalName + L" (" + language.szTranslatedName + L")";

                UINT nLangID = ID_LANGUAGE_MIN + i;
                m_hLangMenu->AppendMenu(MF_STRING, nLangID, szText.c_str());
                m_hLangMenu->CheckMenuItem(nLangID, MF_UNCHECKED);
            }

            int nSelectedLanguage = config::CLanguage::GetLanguageById(m_Config.m_Languages, m_Config.m_Options.szSelectedLanguage);
            if (nSelectedLanguage >= 0)
            {
                config::CLanguage& language = m_Config.m_Languages[nSelectedLanguage];
                m_Config.nLangId = nSelectedLanguage;

                m_hLangMenu->CheckMenuItem(ID_LANGUAGE_MIN + nSelectedLanguage, MF_CHECKED);
            }
            else
            {
                config::CLanguage& language = m_Config.m_Languages[0];
                m_Config.m_Options.szSelectedLanguage = language.szId;
                m_Config.nLangId = 0;

                m_hLangMenu->CheckMenuItem(ID_LANGUAGE_MIN, MF_CHECKED);
            }
        }
        else
        {
            m_hLangMenu->CheckMenuItem(ID_LANGUAGE_DEFAULT, MF_CHECKED);
            m_hLangMenu->EnableMenuItem(ID_LANGUAGE_DEFAULT, MF_DISABLED);
        }
        m_hLangMenu = nullptr;
    }

    void CMainDlg::SetLanguage()
    {
        app::CLanguageHelper helper(&m_Config);
        CMenu *m_hMenu = this->GetMenu();

        // File Menu
        helper.SetMenuPopupText(m_hMenu, 0, 0x00010001);
        helper.SetMenuItemText(m_hMenu, ID_FILE_LOADLIST, 0x00010002);
        helper.SetMenuItemText(m_hMenu, ID_FILE_SAVELIST, 0x00010003);
        helper.SetMenuItemText(m_hMenu, ID_FILE_CLEARLIST, 0x00010004);
        helper.SetMenuItemText(m_hMenu, ID_FILE_EXIT, 0x00010005);

        // Edit Menu
        helper.SetMenuPopupText(m_hMenu, 1, 0x00020001);
        helper.SetMenuItemText(m_hMenu, ID_EDIT_ADDFILES, 0x00020002);
        helper.SetMenuItemText(m_hMenu, ID_EDIT_ADDDIR, 0x00020003);
        helper.SetMenuItemText(m_hMenu, ID_EDIT_RENAME, 0x00020004);
        helper.SetMenuItemText(m_hMenu, ID_EDIT_RESETTIME, 0x00020005);
        helper.SetMenuItemText(m_hMenu, ID_EDIT_RESETOUTPUT, 0x00020006);
        helper.SetMenuItemText(m_hMenu, ID_EDIT_REMOVE, 0x00020007);
        helper.SetMenuItemText(m_hMenu, ID_EDIT_CROP, 0x00020008);
        helper.SetMenuItemText(m_hMenu, ID_EDIT_REMOVECHECKED, 0x00020009);
        helper.SetMenuItemText(m_hMenu, ID_EDIT_REMOVEUNCHECKED, 0x0002000A);
        helper.SetMenuItemText(m_hMenu, ID_EDIT_CHECKSELECTED, 0x0002000B);
        helper.SetMenuItemText(m_hMenu, ID_EDIT_UNCHECKSELECTED, 0x0002000C);
        helper.SetMenuItemText(m_hMenu, ID_EDIT_SELECTALL, 0x0002000D);
        helper.SetMenuItemText(m_hMenu, ID_EDIT_SELECTNONE, 0x0002000E);
        helper.SetMenuItemText(m_hMenu, ID_EDIT_INVERTSELECTION, 0x0002000F);
        helper.SetMenuItemText(m_hMenu, ID_EDIT_OPEN, 0x00020010);
        helper.SetMenuItemText(m_hMenu, ID_EDIT_EXPLORE, 0x00020011);

        // Action Menu
        helper.SetMenuPopupText(m_hMenu, 2, 0x00030001);
        helper.SetMenuItemText(m_hMenu, ID_ACTION_CONVERT, 0x00030002);

        // Options Menu
        helper.SetMenuPopupText(m_hMenu, 3, 0x00040001);
        helper.SetMenuItemText(m_hMenu, ID_OPTIONS_CONFIGUREPRESETS, 0x00040002);
        helper.SetMenuItemText(m_hMenu, ID_OPTIONS_CONFIGUREFORMAT, 0x00040003);
        helper.SetMenuItemText(m_hMenu, ID_OPTIONS_DELETE_SOURCE, 0x00040004);
        helper.SetMenuItemText(m_hMenu, ID_OPTIONS_SHUTDOWN_WINDOWS, 0x00040005);
        helper.SetMenuItemText(m_hMenu, ID_OPTIONS_DO_NOT_SAVE, 0x00040006);
        helper.SetMenuItemText(m_hMenu, ID_OPTIONS_DELETE_ON_ERRORS, 0x00040007);
        helper.SetMenuItemText(m_hMenu, ID_OPTIONS_STOP_ON_ERRORS, 0x00040008);
        helper.SetMenuItemText(m_hMenu, ID_OPTIONS_HIDE_CONSOLE, 0x00040009);
        helper.SetMenuItemText(m_hMenu, ID_OPTIONS_ENSURE_VISIBLE, 0x0004000A);
        helper.SetMenuItemText(m_hMenu, ID_OPTIONS_FIND_DECODER, 0x0004000B);
        helper.SetMenuItemText(m_hMenu, ID_OPTIONS_VALIDATE_FILES, 0x0004000C);
        helper.SetMenuItemText(m_hMenu, ID_OPTIONS_OVERWRITE_FILES, 0x0004000D);
        helper.SetMenuItemText(m_hMenu, ID_OPTIONS_RENAME_FILES, 0x00040010);
        helper.SetMenuItemText(m_hMenu, ID_OPTIONS_CONFIGURETOOLS, 0x0004000E);
        helper.SetMenuItemText(m_hMenu, ID_OPTIONS_DOWNLOAD_TOOLS, 0x0004000F);

        // Language Menu
        helper.SetMenuPopupText(m_hMenu, 4, 0x00050001);
        helper.SetMenuItemText(m_hMenu, ID_LANGUAGE_DEFAULT, 0x00050002);

        // Help Menu
        helper.SetMenuPopupText(m_hMenu, 5, 0x00060001);
        helper.SetMenuItemText(m_hMenu, ID_HELP_WEBSITE, 0x00060002);
        helper.SetMenuItemText(m_hMenu, ID_HELP_ABOUT, 0x00060003);

        m_hMenu = nullptr;

        this->DrawMenuBar();

        // Main Dialog
        helper.SetColumnText(m_LstInputItems, ITEM_COLUMN_NAME, 0x000A0001);
        helper.SetColumnText(m_LstInputItems, ITEM_COLUMN_INPUT, 0x000A0002);
        helper.SetColumnText(m_LstInputItems, ITEM_COLUMN_SIZE, 0x000A0003);
        helper.SetColumnText(m_LstInputItems, ITEM_COLUMN_OUTPUT, 0x000A0004);
        helper.SetColumnText(m_LstInputItems, ITEM_COLUMN_PRESET, 0x000A0005);
        helper.SetColumnText(m_LstInputItems, ITEM_COLUMN_OPTIONS, 0x000B0002);
        helper.SetColumnText(m_LstInputItems, ITEM_COLUMN_TIME, 0x000A0006);
        helper.SetColumnText(m_LstInputItems, ITEM_COLUMN_STATUS, 0x000A0007);

        helper.SetWndText(this, 0x000A0010);
        helper.SetWndText(&m_GrpOutput, 0x000A0011);
        helper.SetWndText(&m_StcFormat, 0x000A0012);
        helper.SetWndText(&m_StcPreset, 0x000A0013);
        helper.SetWndText(&m_StcOutPath, 0x000A0014);
        helper.SetWndText(&m_BtnBrowse, 0x000A0015);
        helper.SetWndText(&m_StcThreads, 0x000A0016);
        helper.SetWndText(&m_BtnConvert, 0x000A0017);
    }

    void CMainDlg::GetOptions()
    {
        m_Config.m_Options.nSelectedFormat = this->m_CmbFormat.GetCurSel();

        CString szOutputPath;
        m_CmbOutPath.GetWindowText(szOutputPath);
        m_Config.m_Options.szOutputPath = szOutputPath;

        CMenu *m_hMenu = this->GetMenu();

        m_Config.m_Options.bDeleteSourceFiles = m_hMenu->GetMenuState(ID_OPTIONS_DELETE_SOURCE, MF_BYCOMMAND) == MF_CHECKED;
        m_Config.m_Options.bRecurseChecked = bRecurseChecked;
        m_Config.m_Options.bShutdownWhenFinished = m_hMenu->GetMenuState(ID_OPTIONS_SHUTDOWN_WINDOWS, MF_BYCOMMAND) == MF_CHECKED;
        m_Config.m_Options.bDoNotSaveConfiguration = m_hMenu->GetMenuState(ID_OPTIONS_DO_NOT_SAVE, MF_BYCOMMAND) == MF_CHECKED;
        m_Config.m_Options.bDeleteOnErrors = m_hMenu->GetMenuState(ID_OPTIONS_DELETE_ON_ERRORS, MF_BYCOMMAND) == MF_CHECKED;
        m_Config.m_Options.bStopOnErrors = m_hMenu->GetMenuState(ID_OPTIONS_STOP_ON_ERRORS, MF_BYCOMMAND) == MF_CHECKED;
        m_Config.m_Options.bHideConsoleWindow = m_hMenu->GetMenuState(ID_OPTIONS_HIDE_CONSOLE, MF_BYCOMMAND) == MF_CHECKED;
        m_Config.m_Options.bEnsureItemIsVisible = m_hMenu->GetMenuState(ID_OPTIONS_ENSURE_VISIBLE, MF_BYCOMMAND) == MF_CHECKED;
        m_Config.m_Options.bTryToFindDecoder = m_hMenu->GetMenuState(ID_OPTIONS_FIND_DECODER, MF_BYCOMMAND) == MF_CHECKED;
        m_Config.m_Options.bValidateInputFiles = m_hMenu->GetMenuState(ID_OPTIONS_VALIDATE_FILES, MF_BYCOMMAND) == MF_CHECKED;
        m_Config.m_Options.bOverwriteExistingFiles = m_hMenu->GetMenuState(ID_OPTIONS_OVERWRITE_FILES, MF_BYCOMMAND) == MF_CHECKED;
        m_Config.m_Options.bRenameExistingFiles = m_hMenu->GetMenuState(ID_OPTIONS_RENAME_FILES, MF_BYCOMMAND) == MF_CHECKED;
        m_Config.m_Options.bTryToDownloadTools = m_hMenu->GetMenuState(ID_OPTIONS_DOWNLOAD_TOOLS, MF_BYCOMMAND) == MF_CHECKED;

        m_hMenu = nullptr;

        CString szThreadCount;
        m_EdtThreads.GetWindowText(szThreadCount);
        m_Config.m_Options.nThreadCount = _tstoi(szThreadCount);

        m_Config.m_Options.szOutputBrowse = szLastOutputBrowse;
        m_Config.m_Options.szDirectoryBrowse = szLastDirectoryBrowse;
        m_Config.m_Options.szMainWindowResize = this->GetWindowRectStr();

        int nColWidth[8];
        for (int i = 0; i < 8; i++)
            nColWidth[i] = m_LstInputItems.GetColumnWidth(i);

        m_Config.m_Options.szFileListColumns =
            std::to_wstring(nColWidth[0]) + L" " +
            std::to_wstring(nColWidth[1]) + L" " +
            std::to_wstring(nColWidth[2]) + L" " +
            std::to_wstring(nColWidth[3]) + L" " +
            std::to_wstring(nColWidth[4]) + L" " +
            std::to_wstring(nColWidth[5]) + L" " +
            std::to_wstring(nColWidth[6]) + L" " +
            std::to_wstring(nColWidth[7]);
    }

    void CMainDlg::SetOptions()
    {
        if (!m_Config.m_Options.szOutputPath.empty())
        {
            this->m_CmbOutPath.SetWindowText(m_Config.m_Options.szOutputPath.c_str());
        }
        else
        {
            m_Config.m_Options.szOutputPath = this->m_Config.m_Settings.szSettingsPath;
            this->m_CmbOutPath.SetWindowText(m_Config.m_Options.szOutputPath.c_str());
        }

        CMenu *m_hMenu = this->GetMenu();

        m_hMenu->CheckMenuItem(ID_OPTIONS_DELETE_SOURCE, m_Config.m_Options.bDeleteSourceFiles  ? MF_CHECKED : MF_UNCHECKED);

        bRecurseChecked = m_Config.m_Options.bRecurseChecked;

        m_hMenu->CheckMenuItem(ID_OPTIONS_SHUTDOWN_WINDOWS, m_Config.m_Options.bShutdownWhenFinished ? MF_CHECKED : MF_UNCHECKED);
        m_hMenu->CheckMenuItem(ID_OPTIONS_DO_NOT_SAVE, m_Config.m_Options.bDoNotSaveConfiguration ? MF_CHECKED : MF_UNCHECKED);
        m_hMenu->CheckMenuItem(ID_OPTIONS_DELETE_ON_ERRORS, m_Config.m_Options.bDeleteOnErrors ? MF_CHECKED : MF_UNCHECKED);
        m_hMenu->CheckMenuItem(ID_OPTIONS_STOP_ON_ERRORS, m_Config.m_Options.bStopOnErrors ? MF_CHECKED : MF_UNCHECKED);
        m_hMenu->CheckMenuItem(ID_OPTIONS_HIDE_CONSOLE, m_Config.m_Options.bHideConsoleWindow ? MF_CHECKED : MF_UNCHECKED);
        m_hMenu->CheckMenuItem(ID_OPTIONS_ENSURE_VISIBLE, m_Config.m_Options.bEnsureItemIsVisible ? MF_CHECKED : MF_UNCHECKED);
        m_hMenu->CheckMenuItem(ID_OPTIONS_FIND_DECODER, m_Config.m_Options.bTryToFindDecoder ? MF_CHECKED : MF_UNCHECKED);
        m_hMenu->CheckMenuItem(ID_OPTIONS_VALIDATE_FILES, m_Config.m_Options.bValidateInputFiles ? MF_CHECKED : MF_UNCHECKED);
        m_hMenu->CheckMenuItem(ID_OPTIONS_OVERWRITE_FILES, m_Config.m_Options.bOverwriteExistingFiles ? MF_CHECKED : MF_UNCHECKED);
        m_hMenu->CheckMenuItem(ID_OPTIONS_RENAME_FILES, m_Config.m_Options.bRenameExistingFiles ? MF_CHECKED : MF_UNCHECKED);
        m_hMenu->CheckMenuItem(ID_OPTIONS_DOWNLOAD_TOOLS, m_Config.m_Options.bTryToDownloadTools ? MF_CHECKED : MF_UNCHECKED);

        m_hMenu = nullptr;

        CString szThreadCount;
        szThreadCount.Format(_T("%d\0"), m_Config.m_Options.nThreadCount);
        m_EdtThreads.SetWindowText(szThreadCount);

        szLastOutputBrowse = m_Config.m_Options.szOutputBrowse.c_str();
        szLastDirectoryBrowse = m_Config.m_Options.szDirectoryBrowse.c_str();

        if (!m_Config.m_Options.szMainWindowResize.empty())
        {
            this->SetWindowRectStr(m_Config.m_Options.szMainWindowResize.c_str());
        }

        if (!m_Config.m_Options.szFileListColumns.empty())
        {
            auto widths = util::string::Split(m_Config.m_Options.szFileListColumns.c_str(), ' ');
            if (widths.size() == 8)
            {
                for (int i = 0; i < 8; i++)
                {
                    std::wstring szWidth = widths[i];
                    int nWidth = util::string::ToInt(szWidth);
                    m_LstInputItems.SetColumnWidth(i, nWidth);
                }
            }
        }
    }

    bool CMainDlg::IsItemSelected(int nItem)
    {
        return m_LstInputItems.GetItemState(nItem, LVIS_SELECTED) == LVIS_SELECTED;
    }

    void CMainDlg::SelectItem(int nItem)
    {
        m_LstInputItems.SetItemState(nItem, LVIS_SELECTED, LVIS_SELECTED);
    }

    void CMainDlg::DeselectItem(int nItem)
    {
        m_LstInputItems.SetItemState(nItem, 0, LVIS_SELECTED);
    }

    void CMainDlg::MakeItemVisible(int nItem)
    {
        m_LstInputItems.EnsureVisible(nItem, FALSE);
    }

    void CMainDlg::ToggleItem(int nItem)
    {
        config::CItem& item = m_Config.m_Items[nItem];
        item.bChecked = !item.bChecked;
        this->RedrawItems();
    }

    void CMainDlg::RedrawItem(int nId)
    {
        this->m_LstInputItems.RedrawItems(nId, nId);
    }

    void CMainDlg::RedrawItems()
    {
        this->m_LstInputItems.RedrawItems(0, m_Config.m_Items.size() - 1);
        this->m_LstInputItems.SetItemCount(m_Config.m_Items.size());
    }

    bool CMainDlg::AddToList(const std::wstring& szPath, int nFormat, int nPreset)
    {
        if (m_Config.m_Options.bValidateInputFiles == true)
        {
            std::wstring szExt = util::GetFileExtension(szPath);
            if (config::CFormat::IsValidInputExtension(m_Config.m_Formats, szExt) == false)
                return false;
        }

        int nItem = m_Config.AddItem(szPath, nFormat, nPreset);
        if (nItem == -1)
            return false;

        return true;
    }

    void CMainDlg::ShowEdtItem()
    {
        CRect rect;
        m_LstInputItems.GetSubItemRect(nEdtItem, nEdtSubItem, LVIR_BOUNDS, rect);
        rect.InflateRect(-6, -2, -2, -2);

        m_EdtItem.SetWindowText(szEdtText);
        m_EdtItem.MoveWindow(rect, TRUE);
        m_EdtItem.SetSel(0, szEdtText.GetLength(), TRUE);
        m_EdtItem.ShowWindow(SW_SHOW);
        m_EdtItem.ShowCaret();
        m_EdtItem.SetFocus();
    }

    void CMainDlg::UpdateEdtItem(BOOL bUpdateText)
    {
        if (m_EdtItem.IsWindowVisible())
        {
            if (bUpdateText == TRUE)
            {
                m_EdtItem.GetWindowText(szEdtText);

                if (nEdtSubItem == ITEM_COLUMN_OPTIONS)
                {
                    config::CItem& item = m_Config.m_Items[nEdtItem];
                    item.szOptions = szEdtText;
                    this->RedrawItems();
                }
            }

            m_EdtItem.HideCaret();
            m_EdtItem.ShowWindow(SW_HIDE);
            m_LstInputItems.SetFocus();
        }
    }

    void CMainDlg::UpdateFormatComboBox()
    {
        if (this->ctx->bRunning == false)
        {
            this->m_CmbFormat.ResetContent();

            size_t nFormats = m_Config.m_Formats.size();
            for (size_t i = 0; i < nFormats; i++)
            {
                config::CFormat& format = m_Config.m_Formats[i];
                this->m_CmbFormat.InsertString(i, format.szName.c_str());
            }

            static bool bResizeFormatsComboBox = false;
            if (bResizeFormatsComboBox == false)
            {
                util::SetComboBoxHeight(this->GetSafeHwnd(), IDC_COMBO_FORMAT, 15);
                bResizeFormatsComboBox = true;
            }

            this->m_CmbFormat.SetCurSel(m_Config.m_Options.nSelectedFormat);
        }
    }

    void CMainDlg::UpdateOutputsComboBox()
    {
        if (this->ctx->bRunning == false)
        {
            this->m_CmbOutPath.ResetContent();

            size_t nOutputs = m_Config.m_Outputs.size();
            for (size_t i = 0; i < nOutputs; i++)
                this->m_CmbOutPath.InsertString(i, m_Config.m_Outputs[i].c_str());

            static bool bResizeOutputComboBox = false;
            if (bResizeOutputComboBox == false)
            {
                util::SetComboBoxHeight(this->GetSafeHwnd(), IDC_COMBO_OUTPUT, 15);
                bResizeOutputComboBox = true;
            }

            this->m_CmbOutPath.SetCurSel(0);
            this->m_CmbOutPath.SetFocus();
        }
    }

    void CMainDlg::UpdatePresetComboBox()
    {
        if (this->ctx->bRunning == false)
        {
            this->m_CmbPresets.ResetContent();

            int nPreset = -1;

            if (m_Config.m_Options.nSelectedFormat > m_Config.m_Formats.size() - 1)
                m_Config.m_Options.nSelectedFormat = 0;

            if (m_Config.m_Options.nSelectedFormat >= 0 && m_Config.m_Formats.size() > 0)
            {
                config::CFormat& format = m_Config.m_Formats[m_Config.m_Options.nSelectedFormat];
                size_t nPresets = format.m_Presets.size();
                for (size_t i = 0; i < nPresets; i++)
                {
                    config::CPreset& preset = format.m_Presets[i];
                    this->m_CmbPresets.InsertString(i, preset.szName.c_str());
                }

                nPreset = format.nDefaultPreset;
            }

            static bool bResizePresetsComboBox = false;
            if (bResizePresetsComboBox == false)
            {
                util::SetComboBoxHeight(this->GetSafeHwnd(), IDC_COMBO_PRESETS, 15);
                bResizePresetsComboBox = true;
            }

            this->m_CmbPresets.SetCurSel(nPreset);
        }
    }

    void CMainDlg::UpdateFormatAndPreset()
    {
        int nFormat = this->m_CmbFormat.GetCurSel();
        int nPreset = this->m_CmbPresets.GetCurSel();
        if ((nFormat >= 0) && (nPreset >= 0))
        {
            config::CFormat& format = m_Config.m_Formats[nFormat];
            config::CPreset& preset = format.m_Presets[nPreset];
            size_t nItems = m_Config.m_Items.size();
            if (nItems > 0)
            {
                int nSelected = 0;
                for (size_t i = 0; i < nItems; i++)
                {
                    if (this->IsItemSelected(i))
                    {
                        config::CItem& item = m_Config.m_Items[i];
                        item.szFormatId = format.szId;
                        item.nPreset = nPreset;
                        nSelected++;
                    }
                }

                this->RedrawItems();

                if (nSelected == 0)
                {
                    for (size_t i = 0; i < nItems; i++)
                    {
                        config::CItem& item = m_Config.m_Items[i];
                        item.szFormatId = format.szId;
                        item.nPreset = nPreset;
                    }
                    this->RedrawItems();
                }
            }
        }
    }

    void CMainDlg::ResetOutput()
    {
        this->UpdateFormatAndPreset();
    }

    void CMainDlg::ResetConvertionTime()
    {
        size_t nItems = m_Config.m_Items.size();
        if (nItems > 0)
        {
            std::wstring szDefaultTime = m_Config.GetString(0x00150001);
            int nSelected = 0;

            for (size_t i = 0; i < nItems; i++)
            {
                if (this->IsItemSelected(i))
                {
                    config::CItem& item = m_Config.m_Items[i];
                    item.szTime = szDefaultTime;
                    
                    nSelected++;
                }
            }

            this->RedrawItems();

            if (nSelected == 0)
            {
                for (size_t i = 0; i < nItems; i++)
                {
                    config::CItem& item = m_Config.m_Items[i];
                    item.szTime = szDefaultTime;
                }
                this->RedrawItems();
            }
        }
    }

    void CMainDlg::ResetConvertionStatus()
    {
        size_t nItems = m_Config.m_Items.size();
        if (nItems > 0)
        {
            std::wstring szDefaultStatus = m_Config.GetString(0x00210001);
            int nSelected = 0;

            for (size_t i = 0; i < nItems; i++)
            {
                if (this->IsItemSelected(i))
                {
                    config::CItem& item = m_Config.m_Items[i];
                    item.szStatus = szDefaultStatus;
                    nSelected++;
                }
            }
            this->RedrawItems();

            if (nSelected == 0)
            {
                for (size_t i = 0; i < nItems; i++)
                {
                    config::CItem& item = m_Config.m_Items[i];
                    item.szStatus = szDefaultStatus;
                }
                this->RedrawItems();
            }
        }
    }

    void CMainDlg::UpdateStatusBar()
    {
        size_t nItems = m_Config.m_Items.size();
        if (nItems > 0)
        {
            std::wstring szText = std::to_wstring(nItems) + L" " + ((nItems > 1) ? m_Config.GetString(0x00210003) : m_Config.GetString(0x00210002));
            m_StatusBar.SetText(szText.c_str(), 0, 0);
        }
        else
        {
            m_StatusBar.SetText(m_Config.GetString(0x00210004).c_str(), 0, 0);
            m_StatusBar.SetText(L"", 1, 0);
        }
    }

    void CMainDlg::EnableUserInterface(BOOL bEnable)
    {
        this->m_StcOutPath.ShowWindow(bEnable == FALSE ? SW_HIDE : SW_SHOW);
        this->m_CmbOutPath.ShowWindow(bEnable == FALSE ? SW_HIDE : SW_SHOW);
        this->m_BtnBrowse.ShowWindow(bEnable == FALSE ? SW_HIDE : SW_SHOW);
        this->m_StcThreads.ShowWindow(bEnable == FALSE ? SW_HIDE : SW_SHOW);
        this->m_EdtThreads.ShowWindow(bEnable == FALSE ? SW_HIDE : SW_SHOW);
        this->m_SpinThreads.ShowWindow(bEnable == FALSE ? SW_HIDE : SW_SHOW);
        this->m_Progress.ShowWindow(bEnable == FALSE ? SW_SHOW : SW_HIDE);

        CMenu* pSysMenu = GetSystemMenu(FALSE);
        pSysMenu->EnableMenuItem(SC_CLOSE, bEnable == FALSE ? MF_GRAYED : MF_ENABLED);
        pSysMenu = nullptr;

        UINT nEnable = (bEnable == TRUE) ? MF_ENABLED : MF_GRAYED;
        CMenu *pMainMenu = this->GetMenu();
        UINT nItems = pMainMenu->GetMenuItemCount();
        for (UINT i = 0; i < nItems; i++)
        {
            CMenu *pSubMenu = pMainMenu->GetSubMenu(i);
            UINT nSubItems = pSubMenu->GetMenuItemCount();
            for (UINT j = 0; j < nSubItems; j++)
            {
                UINT nID = pSubMenu->GetMenuItemID(j);
                pSubMenu->EnableMenuItem(nID, nEnable);
            }
            pSubMenu = nullptr;
        }
        pMainMenu = nullptr;

        this->m_CmbPresets.EnableWindow(bEnable);
        this->m_CmbFormat.EnableWindow(bEnable);
        this->m_CmbOutPath.EnableWindow(bEnable);
        this->m_BtnBrowse.EnableWindow(bEnable);
    }

    void CMainDlg::LoadingConfig(BOOL bEnable)
    {
        this->m_StcOutPath.EnableWindow(bEnable);
        this->m_CmbOutPath.EnableWindow(bEnable);
        this->m_BtnBrowse.EnableWindow(bEnable);
        this->m_StcThreads.EnableWindow(bEnable);
        this->m_EdtThreads.EnableWindow(bEnable);
        this->m_SpinThreads.EnableWindow(bEnable);
        this->m_Progress.EnableWindow(bEnable);
        this->m_LstInputItems.EnableWindow(bEnable);
        this->m_BtnConvert.EnableWindow(bEnable);

        CMenu* pSysMenu = GetSystemMenu(FALSE);
        pSysMenu->EnableMenuItem(SC_CLOSE, bEnable == FALSE ? MF_GRAYED : MF_ENABLED);
        pSysMenu = nullptr;

        UINT nEnable = (bEnable == TRUE) ? MF_ENABLED : MF_GRAYED;
        CMenu *pMainMenu = this->GetMenu();
        UINT nItems = pMainMenu->GetMenuItemCount();
        for (UINT i = 0; i < nItems; i++)
        {
            CMenu *pSubMenu = pMainMenu->GetSubMenu(i);
            UINT nSubItems = pSubMenu->GetMenuItemCount();
            for (UINT j = 0; j < nSubItems; j++)
            {
                UINT nID = pSubMenu->GetMenuItemID(j);
                pSubMenu->EnableMenuItem(nID, nEnable);
            }
            pSubMenu = nullptr;
        }

        pMainMenu = nullptr;

        this->m_CmbPresets.EnableWindow(bEnable);
        this->m_CmbFormat.EnableWindow(bEnable);
        this->m_CmbOutPath.EnableWindow(bEnable);
        this->m_BtnBrowse.EnableWindow(bEnable);
    }

#if 0
    class CDebugConsoleConverter : public worker::IConverter
    {
    public:
        bool Run(worker::IWorkerContext* ctx, worker::CCommandLine& cl, std::mutex& m_down)
        {
            ::OutputDebugStringW((cl.szCommandLine + L"\n").c_str());
            ctx->ItemStatus(cl.nItemId, ctx->GetString(0x00150001), ctx->GetString(0x00120006));
            ctx->ItemProgress(cl.nItemId, 100, true, false);
            return true;

        }
    };

    class CDebugPipesConverter : public worker::IConverter
    {
    public:
        bool Run(worker::IWorkerContext* ctx, worker::CCommandLine& cl, std::mutex& m_down)
        {
            ::OutputDebugStringW((cl.szCommandLine + L"\n").c_str());
            ctx->ItemStatus(cl.nItemId, ctx->GetString(0x00150001), ctx->GetString(0x0013000B));
            ctx->ItemProgress(cl.nItemId, 100, true, false);
            return true;

        }
    };

    class CDebugPipesTranscoder : public worker::ITranscoder
    {
    public:
        bool Run(worker::IWorkerContext* ctx, worker::CCommandLine &dcl, worker::CCommandLine& ecl, std::mutex& m_down)
        {
            ::OutputDebugStringW((dcl.szCommandLine + L"\n").c_str());
            ::OutputDebugStringW((ecl.szCommandLine + L"\n").c_str());
            ctx->ItemStatus(dcl.nItemId, ctx->GetString(0x00150001), ctx->GetString(0x0013000B));
            ctx->ItemProgress(dcl.nItemId, 100, true, false);
            return true;
        }
    };
#endif

    void CMainDlg::StartConvert()
    {
        static volatile bool bSafeCheck = false;
        if (bSafeCheck == true)
            return;

        if (this->ctx->bRunning == false && this->ctx->bDone == true)
        {
            bSafeCheck = true;

            m_StatusBar.SetText(L"", 1, 0);

            util::SetCurrentDirectory_(this->m_Config.m_Settings.szSettingsPath);

            this->GetOptions();

            size_t nItems = this->m_Config.m_Items.size();
            if (nItems <= 0)
            {
                bSafeCheck = false;
                this->ctx->bDone = true;
                return;
            }

            config::CItem::SetIds(this->m_Config.m_Items);

            std::wstring szDefaultTime = m_Config.GetString(0x00150001);
            std::wstring szDefaultStatus = m_Config.GetString(0x00210001);
            size_t nChecked = 0;
            for (size_t i = 0; i < nItems; i++)
            {
                config::CItem& item = this->m_Config.m_Items[i];
                if (item.bChecked == true)
                {
                    item.szTime = szDefaultTime;
                    item.szStatus = szDefaultStatus;
                    nChecked++;
                }
            }
            this->RedrawItems();

            if (nChecked <= 0)
            {
                bSafeCheck = false;
                this->ctx->bDone = true;
                return;
            }

            this->EnableUserInterface(FALSE);

            m_StatusBar.SetText(L"", 1, 0);
            m_BtnConvert.SetWindowText(m_Config.GetString(0x000A0018).c_str());

            CMenu *m_hMenu = this->GetMenu();
            m_hMenu->ModifyMenu(ID_ACTION_CONVERT, MF_BYCOMMAND, ID_ACTION_CONVERT, m_Config.GetString(0x00030003).c_str());
            m_hMenu = nullptr;

            this->ctx->Init();

            this->ctx->nThreadCount = this->m_Config.m_Options.nThreadCount;
            if (this->ctx->nThreadCount < 1)
            {
                util::LogicalProcessorInformation info;
                if (util::GetLogicalProcessorInformation(&info) == 0)
                    this->ctx->nThreadCount = info.processorCoreCount;
                else
                    this->ctx->nThreadCount = 1;
            }

            this->ctx->pConfig = &this->m_Config;

            auto m_WorkerThread = std::thread([this]()
            {
                auto pWorker = std::make_unique<worker::CWorker>();
            #if 0
                pWorker->ConsoleConverter = std::make_unique<CDebugConsoleConverter>();
                pWorker->PipesConverter = std::make_unique<CDebugPipesConverter>();
                pWorker->PipesTranscoder = std::make_unique<CDebugPipesTranscoder>();
            #else
                pWorker->ConsoleConverter = std::make_unique<worker::CConsoleConverter>();
                pWorker->PipesConverter = std::make_unique<worker::CPipesConverter>();
                pWorker->PipesTranscoder = std::make_unique<worker::CPipesTranscoder>();
            #endif
                pWorker->Convert(this->ctx.get(), this->m_Config.m_Items);
            });
            m_WorkerThread.detach();

            bSafeCheck = false;
        }
        else
        {
            bSafeCheck = true;

            m_BtnConvert.SetWindowText(m_Config.GetString(0x000A0017).c_str());

            CMenu *m_hMenu = this->GetMenu();
            m_hMenu->ModifyMenu(ID_ACTION_CONVERT, MF_BYCOMMAND, ID_ACTION_CONVERT, m_Config.GetString(0x00030002).c_str());
            m_hMenu = nullptr;

            this->EnableUserInterface(TRUE);

            this->ctx->bRunning = false;
            bSafeCheck = false;
        }
    }

    void CMainDlg::FinishConvert()
    {
        this->m_BtnConvert.SetWindowText(m_Config.GetString(0x000A0017).c_str());

        CMenu *m_hMenu = this->GetMenu();
        m_hMenu->ModifyMenu(ID_ACTION_CONVERT, MF_BYCOMMAND, ID_ACTION_CONVERT, m_Config.GetString(0x00030002).c_str());
        m_hMenu = nullptr;

        this->EnableUserInterface(TRUE);

        this->m_Progress.SetPos(0);

        if (this->m_Config.m_Options.bShutdownWhenFinished == true)
        {
            this->GetOptions();

            if (this->m_Config.m_Options.bDoNotSaveConfiguration == false)
            {
                this->SaveConfig();
            }

            this->ctx->bRunning = false;
            util::ShutdownWindows();
        }
    }

    bool CMainDlg::LoadOptions(config::xml::XmlDocumnent &doc)
    {
        if (this->m_Config.LoadOptions(doc))
        {
            this->SetOptions();
            this->UpdateFormatComboBox();
            this->UpdatePresetComboBox();
            return true;
        }
        return false;
    }

    bool CMainDlg::LoadFormat(config::xml::XmlDocumnent &doc)
    {
        if (this->m_Config.LoadFormat(doc))
        {
            this->UpdateFormatComboBox();
            this->UpdatePresetComboBox();
            return true;
        }
        return false;
    }

    bool CMainDlg::LoadPresets(config::xml::XmlDocumnent &doc)
    {
        int nFormat = this->m_CmbFormat.GetCurSel();
        if (nFormat != -1)
        {
            if (this->m_Config.LoadPresets(doc, nFormat))
            {
                this->UpdatePresetComboBox();
                return true;
            }
        }
        return false;
    }

    bool CMainDlg::LoadTool(config::xml::XmlDocumnent &doc)
    {
        return this->m_Config.LoadTool(doc);
    }

    bool CMainDlg::LoadItems(const std::wstring& szFileXml)
    {
        if (this->m_Config.LoadItems(szFileXml))
        {
            this->RedrawItems();
            this->UpdateStatusBar();
            return true;
        }
        return false;
    }

    bool CMainDlg::LoadItems(config::xml::XmlDocumnent &doc)
    {
        if (this->m_Config.LoadItems(doc))
        {
            this->RedrawItems();
            this->UpdateStatusBar();
            return true;
        }
        return false;
    }

    bool CMainDlg::SaveItems(const std::wstring& szFileXml)
    {
        config::CItem::SetIds(this->m_Config.m_Items);
        return this->m_Config.SaveItems(szFileXml);
    }

    bool CMainDlg::LoadOutputs(config::xml::XmlDocumnent &doc)
    {
        if (this->m_Config.LoadOutputs(doc))
        {
            this->UpdateOutputsComboBox();
            return true;
        }
        return false;
    }

    bool CMainDlg::LoadLanguage(config::xml::XmlDocumnent &doc)
    {
        if (this->m_Config.LoadLanguage(doc))
        {
            config::CLanguage &language = m_Config.m_Languages.back();
            int nIndex = m_Config.m_Languages.size() - 1;
            std::wstring szText = language.szOriginalName + L" (" + language.szTranslatedName + L")";

            UINT nLangID = ID_LANGUAGE_MIN + nIndex;
            CMenu *m_hMenu = this->GetMenu();
            CMenu *m_hLangMenu = m_hMenu->GetSubMenu(4);
            if (nLangID == ID_LANGUAGE_MIN)
                m_hLangMenu->DeleteMenu(ID_LANGUAGE_DEFAULT, 0);

            m_hLangMenu->AppendMenu(MF_STRING, nLangID, szText.c_str());
            m_hLangMenu->CheckMenuItem(nLangID, MF_UNCHECKED);
            m_hLangMenu = nullptr;
            m_hMenu = nullptr;

            return true;
        }
        return false;
    }

    void CMainDlg::LoadConfig()
    {
        try
        {
            this->m_Config.nLangId = -1;

            this->m_Config.LoadTools(this->m_Config.m_Settings.szToolsPath);
            this->m_Config.LoadFormats(this->m_Config.m_Settings.szFormatsPath);

            if (this->m_Config.LoadOutputs(this->m_Config.m_Settings.szOutputsFile) == false)
                this->m_Config.m_Outputs = config::m_OutpuPathsPresets;

            if (this->m_Config.LoadOptions(this->m_Config.m_Settings.szOptionsFile) == false)
                this->m_Config.m_Options.Defaults();

            this->m_Config.LoadItems(this->m_Config.m_Settings.szItemsFile);

            this->m_Config.LoadLanguages(this->m_Config.m_Settings.szSettingsPath);
            this->m_Config.LoadLanguages(this->m_Config.m_Settings.szLanguagesPath);
        }
        catch (...)
        {
            this->m_Config.Log->Log(L"[Error] Failed to load config.");
        }
    }

    void CMainDlg::SaveConfig()
    {
        try
        {
            this->m_Config.SaveTools(this->m_Config.m_Settings.szToolsPath);
            this->m_Config.SaveFormats(this->m_Config.m_Settings.szFormatsPath);
            this->m_Config.SaveOutputs(this->m_Config.m_Settings.szOutputsFile);

            this->m_Config.SaveOptions(this->m_Config.m_Settings.szOptionsFile);

            config::CItem::SetIds(this->m_Config.m_Items);
            this->m_Config.SaveItems(this->m_Config.m_Settings.szItemsFile);
        }
        catch (...)
        {
            this->m_Config.Log->Log(L"[Error] Failed to save config.");
        }
    }
}
