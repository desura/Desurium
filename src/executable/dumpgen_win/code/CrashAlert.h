///////////// Copyright 2010 DesuraNet. All rights reserved. /////////////
//
//   Project     : dumpgen
//   File        : CrashAlert.h
//   Description :
//      [TODO: Write the purpose of CrashAlert.h.]
//
//   Created On: 9/24/2010 7:37:23 PM
//   Created By:  <mailto:>
////////////////////////////////////////////////////////////////////////////

#ifndef DESURA_CRASHALERT_H
#define DESURA_CRASHALERT_H
#ifdef _WIN32
#pragma once
#endif

#include "Resource.h"
#include "DesuraWnd.h"

// CrashAlert frame

class CrashAlert : public Desurium::CDesuraDialog
{
public:
	CrashAlert();
	virtual ~CrashAlert();

protected:
	void OnCommand(HWND hWnd, int nId) override;
	bool OnInitDialog() override;

private:
	HICON m_hIcon;

public:
	void OnBnClickedYes();
};

#endif //DESURA_CRASHALERT_H






