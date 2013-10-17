// CrashAleart.cpp : implementation file
//

#include "CrashAlert.h"

using namespace Desurium;


// CrashAleart
CrashAlert::CrashAlert()
	: CDesuraDialog(IDD_DIALOG1)
{
	m_hIcon = LoadIcon(IDI_ICON1);
}

CrashAlert::~CrashAlert()
{
}

bool CrashAlert::OnInitDialog()
{
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	return true;
}

void CrashAlert::OnCommand(HWND hWnd, int nId)
{
	EndDialog(nId);
}
