﻿#pragma once

#define PRESET_COLUMN_NAME      0
#define PRESET_COLUMN_OPTIONS   1

namespace dialogs
{
    class CPresetsDlg;

    class CPresetsDlg : public controls::CMyDialogEx
    {
        DECLARE_DYNAMIC(CPresetsDlg)
    public:
        CPresetsDlg(CWnd* pParent = nullptr);
        virtual ~CPresetsDlg();
        enum { IDD = IDD_DIALOG_PRESETS };
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
        std::wstring szPresetsDialogResize;
        std::wstring szPresetsListColumns;
        volatile bool bUpdate;
        int nSelectedFormat;
        std::vector<config::CFormat> m_Formats;
    public:
        controls::CMyStatic m_StcName;
        controls::CMyStatic m_StcOptions;
        controls::CMyListCtrl m_LstPresets;
        controls::CMyComboBox m_CmbFormat;
        controls::CMyEdit m_EdtName;
        controls::CMyEdit m_EdtOptions;
        controls::CMyButton m_BtnOK;
        controls::CMyButton m_BtnCancel;
        controls::CMyButton m_BtnDuplicate;
        controls::CMyButton m_BtnRemoveAll;
        controls::CMyButton m_BtnRemove;
        controls::CMyButton m_BtnAdd;
        controls::CMyButton m_BtnMoveUp;
        controls::CMyButton m_BtnMoveDown;
        controls::CMyButton m_BtnUpdate;
        controls::CMyButton m_BtnImport;
        controls::CMyButton m_BtnExport;
    public:
        afx_msg void OnDropFiles(HDROP hDropInfo);
        afx_msg void OnBnClickedOk();
        afx_msg void OnBnClickedCancel();
        afx_msg void OnLvnGetdispinfoListPresets(NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg void OnOdfindListPresets(NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg void OnLvnItemchangedListPresets(NMHDR *pNMHDR, LRESULT *pResult);
        afx_msg LRESULT OnNotifyFormat(WPARAM wParam, LPARAM lParam);
        afx_msg void OnCbnSelchangeComboPresetFormat();
        afx_msg void OnBnClickedButtonDuplicate();
        afx_msg void OnBnClickedButtonRemoveAllPresets();
        afx_msg void OnBnClickedButtonRemovePreset();
        afx_msg void OnBnClickedButtonAddPreset();
        afx_msg void OnBnClickedButtonPresetUp();
        afx_msg void OnBnClickedButtonPresetDown();
        afx_msg void OnBnClickedButtonUpdatePreset();
        afx_msg void OnEnChangeEditPresetName();
        afx_msg void OnEnChangeEditPresetOptions();
        afx_msg void OnBnClickedButtonImportPresets();
        afx_msg void OnBnClickedButtonExportPresets();
        afx_msg void OnClose();
    public:
        void LoadWindowSettings();
        void SaveWindowSettings();
        void SetLanguage();
        void RedrawPreset(int nId);
        void RedrawPresets();
        void UpdateFields(config::CPreset &preset);
        void ListSelectionChange();
    public:
        bool LoadPresets(const std::wstring& szFileXml);
        bool LoadPresets(config::xml::XmlDocumnent &doc);
        bool SavePresets(const std::wstring& szFileXml, config::CFormat &format);
    };
}
