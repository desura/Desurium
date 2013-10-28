/*
Desura is the leading indie game distribution platform
Copyright (C) 2011 Mark Chandler (Desura Net Pty Ltd)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>
*/


#ifndef DESURA_UNITTEST_FORM_H
#define DESURA_UNITTEST_FORM_H
#ifdef _WIN32
#pragma once
#endif

#include <wx/imaglist.h>
#include "wx_controls/gcControls.h"

class wxListCtrl;
class CUnitTestThread;

class gcUnitTestForm : public gcDialog
{
public:
	gcUnitTestForm(wxWindow* parent);
	~gcUnitTestForm();

	void canClose();
	void postShowEvent();

protected:
	void onShow(uint32&);

	void onStart();
	void onEnd();

	void onTestStart(gcString &strTest);
	void onTestResult(std::pair<gcString, bool> &result);
	void onTestEnd(std::pair<gcString,uint64> &time);

	void runTests();

	void onButtonClicked(wxCommandEvent& event);

	void OnRunOnStartupClicked(wxCommandEvent& event);
	void OnAssertOnErrorClicked(wxCommandEvent& event);
	void OnTextChanged(wxCommandEvent& event);
	void OnClose(wxCloseEvent& event);

private:
	wxCheckBox* m_cbAssertOnError;
	wxCheckBox* m_cbRunOnStartup;
	wxStaticText* m_staticText1;
	wxTextCtrl* m_textCtrl1;
	wxButton* m_butRun;
	wxListCtrl* m_lcStats;
	wxListCtrl* m_lcUnitTests;
	wxButton* m_butClose;

	wxImageList m_ImageList;
	CUnitTestThread *m_pThread;

	gcImageHandle m_icoTestGood;
	gcImageHandle m_icoTestBad;

	std::map<gcString, int> m_mTestIndex;

	bool m_bShouldClose;
	Event<uint32> showEvent;

	int m_nPassed;
	int m_nFailed;
	uint64 m_nRunTime;
};


#endif