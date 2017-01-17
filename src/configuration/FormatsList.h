﻿// Copyright (c) Wiesław Šoltés. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include <afxstr.h>
#include <afxtempl.h>
#include "Format.h"

class CFormatsList
{
public:
    CList<CFormat, const CFormat&> m_Formats;
public:
    void SetData(CFormat& format, int idx)
    {
        m_Formats.SetAt(m_Formats.FindIndex(idx), format);
    }
    CFormat& GetData(int idx)
    {
        return m_Formats.GetAt(m_Formats.FindIndex(idx));
    }
public:
    CFormatsList()
    {
    }
    CFormatsList(const CFormatsList &other)
    {
        Copy(other);
    }
    CFormatsList& operator=(const CFormatsList &other)
    {
        Copy(other);
        return *this;
    }
    virtual ~CFormatsList()
    {
        if (m_Formats.GetCount() != 0)
            m_Formats.RemoveAll();
    }
public:
    void Copy(const CFormatsList &other)
    {
        int nFormats = (int)other.m_Formats.GetCount();
        for (int i = 0; i < nFormats; i++)
        {
            CFormat format = other.m_Formats.GetAt(other.m_Formats.FindIndex(i));
            this->InsertNode(format);
        }
    }
public:
    bool IsEmpty()
    {
        return (m_Formats.GetCount() == 0) ? true : false;
    }
    int GetSize()
    {
        return (int)m_Formats.GetCount();
    }
public:
    void InsertNode(CFormat &format)
    {
        m_Formats.AddTail(format);
    }
    void RemoveNode(int pstn = -1)
    {
        m_Formats.RemoveAt(m_Formats.FindIndex(pstn));
    }
    void RemoveAllNodes(void)
    {
        if (m_Formats.GetCount() != 0)
            m_Formats.RemoveAll();
    }
public:
    void Copy(CFormatsList& other)
    {
        int nFormats = this->GetSize();
        for (int i = 0; i < nFormats; i++)
        {
            CFormat& format = this->GetData(i);
            CFormat copy;
            format.Copy(copy);
            other.InsertNode(copy);
        }
    }
public:
    void SwapItems(int idx1, int idx2)
    {
        CFormat& format1 = this->GetData(idx1);
        CFormat& format2 = this->GetData(idx2);
        if ((idx1 < 0) || (idx2 < 0) || (idx1 >= GetSize()) || (idx2 >= GetSize()))
            return;
        this->SetData(format1, idx2);
        this->SetData(format2, idx1);
    }
public:
    int GetFormatById(CString szFormatId)
    {
        szFormatId.MakeUpper();
        int nFormats = this->GetSize();
        for (int i = 0; i < nFormats; i++)
        {
            CFormat& format = this->GetData(i);
            if (szFormatId.Compare(format.szId.MakeUpper()) == 0)
                return i;
        }
        return -1;
    }
public:
    int GetInFormatByExt(CString szInExt)
    {
        szInExt.MakeUpper();
        int nFormats = this->GetSize();
        for (int i = 0; i < nFormats; i++)
        {
            CFormat& format = this->GetData(i);
            if (format.nType == 1 && szInExt.Compare(format.szExtension.MakeUpper()) == 0)
                return i;
        }
        return -1;
    }
    int GetOutFormatByExt(CString szOutExt)
    {
        szOutExt.MakeUpper();
        int nFormats = this->GetSize();
        for (int i = 0; i < nFormats; i++)
        {
            CFormat& format = this->GetData(i);
            if (format.nType == 0 && szOutExt.Compare(format.szExtension.MakeUpper()) == 0)
                return i;
        }
        return -1;
    }
public:
    int GetInFormatById(CString szFormatId)
    {
        szFormatId.MakeUpper();
        int nFormats = this->GetSize();
        for (int i = 0; i < nFormats; i++)
        {
            CFormat& format = this->GetData(i);
            if (format.nType == 1 && szFormatId.Compare(format.szId.MakeUpper()) == 0)
                return i;
        }
        return -1;
    }
    int GetOutFormatById(CString szFormatId)
    {
        szFormatId.MakeUpper();
        int nFormats = this->GetSize();
        for (int i = 0; i < nFormats; i++)
        {
            CFormat& format = this->GetData(i);
            if (format.nType == 0 && szFormatId.Compare(format.szId.MakeUpper()) == 0)
                return i;
        }
        return -1;
    }
public:
    bool IsValidInExtension(CString szInExt)
    {
        szInExt.MakeUpper();
        int nFormats = this->GetSize();
        for (int i = 0; i < nFormats; i++)
        {
            CFormat& format = this->GetData(i);
            if (format.nType == 1 && szInExt.Compare(format.szExtension.MakeUpper()) == 0)
                return true;
        }
        return false;
    }
    bool IsValidOutExtension(CString szOutExt)
    {
        szOutExt.MakeUpper();
        int nFormats = this->GetSize();
        for (int i = 0; i < nFormats; i++)
        {
            CFormat& format = this->GetData(i);
            if (format.nType == 0 && szOutExt.Compare(format.szExtension.MakeUpper()) == 0)
                return true;
        }
        return false;
    }
};
