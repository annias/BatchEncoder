﻿// Copyright (c) Wiesław Šoltés. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include <afxwin.h>
#include <afxcmn.h>
#include "utilities\TimeCount.h"
#include "Configuration.h"
#include "worker\WorkerContext.h"
#include "dialogs\BatchEncoderDlg.h"

class CBatchEncoderWorkerContext : public CWorkerContext
{
private:
    CTimeCount timer;
    CBatchEncoderDlg *pDlg;
public:
    CBatchEncoderWorkerContext(CConfiguration* pConfig, CBatchEncoderDlg* pDlg);
    virtual ~CBatchEncoderWorkerContext();
public:
    void Init();
    void Next(int nItemId);
    void Done();
    bool Callback(int nItemId, int nProgress, bool bFinished, bool bError = false);
    void Status(int nItemId, CString szTime, CString szStatus);
};
