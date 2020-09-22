﻿#pragma once

#define TOOL_COLUMN_NAME      0
#define TOOL_COLUMN_PRIORITY  1
#define TOOL_COLUMN_URL       2
#define TOOL_COLUMN_STATUS    3

namespace dialogs
{
    class CToolsDlg;

    class CToolsDlg : public controls::CMyDialogEx
    {
        DECLARE_DYNAMIC(CToolsDlg)
    public:
        CToolsDlg(CWnd* pParent = nullptr);
        virtual ~CToolsDlg();
        enum { IDD = IDD_DIALOG_TOOLS };
    protected:
        virtual void DoDataExchange(CDataExchange* pDX);
        DECLARE_MESSAGE_MAP()
    protected:
        HICON m_hIcon;
        afx_msg void OnPaint();
        afx_msg HCURSOR OnQueryDragIcon();
    public:
        virtual BOOL OnInitDialog();
    public:
        config::CConfig *pConfig;
        std::wstring szToolsDialogResize;
        std::wstring szToolsListColumns;
        CString szLastToolsBrowse;
        volatile bool bUpdate;
        int nSelectedTool;
        std::vector<config::CTool> m_Tools;
        std::vector<config::CFormat> m_Formats;
        std::thread m_Thread;
        volatile bool bAbort = false;
        worker::CToolDownloader m_Downloader;
    public:
        controls::CMyStatic m_StcName;
        controls::CMyStatic m_StcPlatform;
        controls::CMyStatic m_StcPriority;
        controls::CMyStatic m_StcFormats;
        controls::CMyStatic m_StcUrl;
        controls::CMyStatic m_StcFile;
        controls::CMyStatic m_StcExtract;
        controls::CMyStatic m_StcPath;
        controls::CMyListCtrl m_LstTools;
        controls::CMyEdit m_EdtName;
        controls::CMyEdit m_EdtPlatform;
        controls::CMyEdit m_EdtPriority;
        controls::CMySpinButtonCtrl m_SpinPriority;
        controls::CMyEdit m_EdtFormats;
        controls::CMyEdit m_EdtUrl;
        controls::CMyEdit m_EdtFile;
        controls::CMyEdit m_EdtExtract;
        controls::CMyEdit m_EdtPath;
        controls::CMyButton m_BtnOK;
        controls::CMyButton m_BtnCancel;
        controls::CMyButton m_BtnImport;
        controls::CMyButton m_BtnExport;
        controls::CMyButton m_BtnDuplicate;
        controls::CMyButton m_BtnRemoveAll;
        controls::CMyButton m_BtnRemove;
        controls::CMyButton m_BtnAdd;
        controls::CMyButton m_BtnUpdate;
        controls::CMyButton m_BtnDownload;
        controls::CMyButton m_BtnSetFormat;
        controls::CMyButton m_BtnSetFormatX86;
        controls::CMyButton m_BtnSetFormatX64;
    public:
        afx_msg void OnDropFiles(HDROP hDropInfo);
        afx_msg void OnBnClickedOk();
        afx_msg void OnBnClickedCancel();
        afx_msg void OnLvnGetdispinfoListTools(NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg void OnOdfindListTools(NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg void OnLvnItemchangedListTools(NMHDR *pNMHDR, LRESULT *pResult);
        afx_msg LRESULT OnNotifyFormat(WPARAM wParam, LPARAM lParam);
        afx_msg void OnBnClickedButtonImport();
        afx_msg void OnBnClickedButtonExport();
        afx_msg void OnBnClickedButtonDuplicate();
        afx_msg void OnBnClickedButtonRemoveAllTools();
        afx_msg void OnBnClickedButtonRemoveTool();
        afx_msg void OnBnClickedButtonAddTool();
        afx_msg void OnBnClickedButtonUpdateTool();
        afx_msg void OnEnChangeEditToolName();
        afx_msg void OnEnChangeEditToolPlatform();
        afx_msg void OnEnChangeEditToolPriority();
        afx_msg void OnEnChangeEditToolFormats();
        afx_msg void OnEnChangeEditToolUrl();
        afx_msg void OnEnChangeEditToolFile();
        afx_msg void OnEnChangeEditToolExtract();
        afx_msg void OnEnChangeEditToolPath();
        afx_msg void OnBnClickedButtonDownloadSelected();
        afx_msg void OnBnClickedButtonToolSetFormat();
        afx_msg void OnBnClickedButtonToolSetFormatX86();
        afx_msg void OnBnClickedButtonToolSetFormatX64();
        afx_msg void OnClose();
        afx_msg void OnDestroy();
    public:
        void LoadWindowSettings();
        void SaveWindowSettings();
        void SetLanguage();
        void RedrawTool(int nId);
        void RedrawTools();
        void UpdateFields(config::CTool &format);
        void ListSelectionChange();
        void EnableUserInterface(BOOL bEnable = TRUE);
        void DownloadTools();
    public:
        bool LoadTool(const std::wstring& szFileXml);
        bool LoadTool(config::xml::XmlDocumnent &doc);
        bool SaveTool(const std::wstring& szFileXml, config::CTool &tool);
        bool SaveTools(const std::wstring& szPath);
    };
}
