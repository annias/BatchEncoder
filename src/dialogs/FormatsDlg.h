﻿#pragma once

#define FORMAT_COLUMN_NAME      0
#define FORMAT_COLUMN_ID        1
#define FORMAT_COLUMN_PRIORITY  2
#define FORMAT_COLUMN_TEMPLATE  3

namespace dialogs
{
    class CFormatsDlg;

    class CFormatsDlg : public controls::CMyDialogEx
    {
        DECLARE_DYNAMIC(CFormatsDlg)
    public:
        CFormatsDlg(CWnd* pParent = nullptr);
        virtual ~CFormatsDlg();
        enum { IDD = IDD_DIALOG_FORMATS };
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
        std::wstring szFormatsDialogResize;
        std::wstring szFormatsListColumns;
        CString szLastFormatsBrowse;
        volatile bool bUpdate;
        int nSelectedFormat;
        std::vector<config::CFormat> m_Formats;
    public:
        controls::CMyStatic m_GrpPipes;
        controls::CMyStatic m_GrpTypes;
        controls::CMyStatic m_StcId;
        controls::CMyStatic m_StcName;
        controls::CMyStatic m_StcPriority;
        controls::CMyStatic m_StcExtension;
        controls::CMyStatic m_StcFormats;
        controls::CMyStatic m_StcCode;
        controls::CMyStatic m_StcDefault;
        controls::CMyStatic m_StcPath;
        controls::CMyStatic m_StcTemplate;
        controls::CMyStatic m_StcProgress;
        controls::CMyListCtrl m_LstFormats;
        controls::CMyComboBox m_CmbDefault;
        controls::CMyEdit m_EdtId;
        controls::CMyEdit m_EdtName;
        controls::CMyEdit m_EdtPriority;
        controls::CMySpinButtonCtrl m_SpinPriority;
        controls::CMyEdit m_EdtExtension;
        controls::CMyEdit m_EdtFormats;
        controls::CMyEdit m_EdtCode;
        controls::CMyEdit m_EdtPath;
        controls::CMyEdit m_EdtTemplate;
        controls::CMyEdit m_EdtFunction;
        controls::CMyButton m_BtnOK;
        controls::CMyButton m_BtnCancel;
        controls::CMyButton m_BtnImport;
        controls::CMyButton m_BtnExport;
        controls::CMyButton m_BtnDuplicate;
        controls::CMyButton m_BtnRemoveAll;
        controls::CMyButton m_BtnRemove;
        controls::CMyButton m_BtnAdd;
        controls::CMyButton m_BtnUpdate;
        controls::CMyButton m_BtnEditPresets;
        controls::CMyButton m_BtnBrowsePath;
        controls::CMyButton m_BtnBrowseFunction;
    public:
        afx_msg void OnDropFiles(HDROP hDropInfo);
        afx_msg void OnBnClickedOk();
        afx_msg void OnBnClickedCancel();
        afx_msg void OnLvnGetdispinfoListFormats(NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg void OnOdfindListFormats(NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg void OnLvnItemchangedListFormats(NMHDR *pNMHDR, LRESULT *pResult);
        afx_msg LRESULT OnNotifyFormat(WPARAM wParam, LPARAM lParam);
        afx_msg void OnBnClickedButtonImport();
        afx_msg void OnBnClickedButtonExport();
        afx_msg void OnBnClickedButtonDuplicate();
        afx_msg void OnBnClickedButtonRemoveAllFormats();
        afx_msg void OnBnClickedButtonRemoveFormat();
        afx_msg void OnBnClickedButtonAddFormat();
        afx_msg void OnBnClickedButtonUpdateFormat();
        afx_msg void OnBnClickedRadioTypeEncoder();
        afx_msg void OnBnClickedRadioTypeDecoder();
        afx_msg void OnBnClickedCheckPipesInput();
        afx_msg void OnBnClickedCheckPipesOutput();
        afx_msg void OnEnChangeEditFormatId();
        afx_msg void OnEnChangeEditFormatName();
        afx_msg void OnEnChangeEditFormatPriority();
        afx_msg void OnEnChangeEditFormatExtension();
        afx_msg void OnEnChangeEditFormatFormats();
        afx_msg void OnCbnSelchangeComboDefault();
        afx_msg void OnEnChangeEditFormatPath();
        afx_msg void OnEnChangeEditFormatTemplate();
        afx_msg void OnEnChangeEditFormatFunction();
        afx_msg void OnBnClickedButtonEditPresets();
        afx_msg void OnBnClickedButtonBrowsePath();
        afx_msg void OnBnClickedButtonBrowseProgress();
        afx_msg void OnClose();
    public:
        void LoadWindowSettings();
        void SaveWindowSettings();
        void SetLanguage();
        void RedrawFormat(int nId);
        void RedrawFormats();
        void UpdateFields(config::CFormat &format);
        void UpdateDefaultComboBox(config::CFormat &format);
        void ListSelectionChange();
        bool BrowseForPath(CString szDefaultFName, CEdit *pEdit, int nID);
        bool BrowseForFunction(CString szDefaultFName, CEdit *pEdit, int nID);
    public:
        bool LoadPresets(config::xml::XmlDocumnent &doc);
        bool LoadFormat(const std::wstring& szFileXml);
        bool LoadFormat(config::xml::XmlDocumnent &doc);
        bool SaveFormat(const std::wstring& szFileXml, config::CFormat &format);
        bool SaveFormats(const std::wstring& szPath);
    };
}
