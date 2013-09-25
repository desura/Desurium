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


#include "Common.h"
#include "gcUnitTestPage.h"

#include <wx/listctrl.h>
#include <gtest/gtest.h>
#include "managers/CVar.h"

using namespace testing;

CVar g_bAssertOnFailure("unittest_assertonfailure", "true");
CVar g_bRunOnStartup("unittest_runonstartup", "true");
CVar g_strUnitTestFilter("unittest_filter", "*");

class gcUnitTestWatcher : public EmptyTestEventListener
{
public:
	void OnTestIterationStart(const UnitTest& unit_test, int iteration)
	{
		onStartEvent();
	}

	void OnTestIterationEnd(const UnitTest& unit_test, int iteration)
	{
		onEndEvent();
	}

	void OnTestStart(const TestInfo& test_info)
	{
		m_szLastTest = gcString("{0}/{1}", test_info.test_case_name(), test_info.name());
		onTestStartEvent(m_szLastTest);
	}

	void OnTestPartResult(const TestPartResult& test_part_result)
	{
		if (test_part_result.failed() && g_bAssertOnFailure.getBool())
			wxASSERT(FALSE);
	}

	void OnTestEnd(const TestInfo& test_info)
	{
		std::pair<gcString, bool> res = std::make_pair(m_szLastTest, test_info.result()->Passed());
		std::pair<gcString, uint64> end = std::make_pair(m_szLastTest, (uint64)test_info.result()->elapsed_time());

		onTestResultEvent(res);
		onTestEndEnd(end);
	}

	gcString m_szLastTest;

	EventV onStartEvent;
	EventV onEndEvent;

	Event<gcString> onTestStartEvent;
	Event<std::pair<gcString,bool>> onTestResultEvent;
	Event<std::pair<gcString,uint64>> onTestEndEnd;
};

class CUnitTestThread : public Thread::BaseThread
{
public:
	CUnitTestThread()
		: BaseThread("UnitTestThread")
	{

	}

	void run()
	{
		m_strFilter = g_strUnitTestFilter.getString();

		GTEST_FLAG(filter) = m_strFilter;
		RUN_ALL_TESTS();
		GTEST_FLAG(filter) = NULL;
	}

	gcString m_strFilter;
};


static gcUnitTestWatcher* SetupTestWatcher()
{
	gcUnitTestWatcher* pWatcher = new gcUnitTestWatcher();

	TestEventListeners& listeners = UnitTest::GetInstance()->listeners();
	listeners.Append(pWatcher);

	int argc = 0;
	InitGoogleTest(&argc, (char**)NULL);

	return pWatcher;
}

static gcUnitTestWatcher *g_pTestWatcher = SetupTestWatcher();

gcUnitTestForm::gcUnitTestForm(wxWindow* parent) 
	: gcDialog(parent, wxID_ANY, wxT("Unit Tests Results"), wxDefaultPosition, wxSize(500, 354), wxCAPTION | wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL)
	, m_pThread(NULL)
	, m_ImageList(16, 16)
	, m_bShouldClose(false)
{
	showEvent += guiDelegate(this, &gcUnitTestForm::onShow);

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &gcUnitTestForm::onButtonClicked, this);
	Bind(wxEVT_CLOSE_WINDOW, &gcUnitTestForm::OnClose, this);

	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer(4, 1, 0, 0);
	fgSizer1->AddGrowableCol(0);
	fgSizer1->AddGrowableRow(2);
	fgSizer1->SetFlexibleDirection(wxBOTH);
	fgSizer1->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);

	wxBoxSizer* bSizer2;
	bSizer2 = new wxBoxSizer(wxHORIZONTAL);

	m_cbAssertOnError = new wxCheckBox(this, wxID_ANY, wxT("Assert On Error"), wxDefaultPosition, wxDefaultSize, 0);
	m_cbAssertOnError->SetValue(true);
	bSizer2->Add(m_cbAssertOnError, 0, wxALL, 5);

	m_cbRunOnStartup = new wxCheckBox(this, wxID_ANY, wxT("Run On Startup"), wxDefaultPosition, wxDefaultSize, 0);
	m_cbRunOnStartup->SetValue(true);
	bSizer2->Add(m_cbRunOnStartup, 0, wxALL, 5);

	fgSizer1->Add(bSizer2, 1, wxEXPAND, 5);

	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer(wxHORIZONTAL);

	m_staticText1 = new wxStaticText(this, wxID_ANY, wxT("Filter:"), wxDefaultPosition, wxDefaultSize, 0);
	m_staticText1->Wrap(-1);
	bSizer1->Add(m_staticText1, 0, wxALIGN_CENTER_VERTICAL | wxTOP | wxBOTTOM | wxLEFT, 5);

	m_textCtrl1 = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(m_textCtrl1, 1, wxEXPAND | wxALL, 5);

	m_butRun = new wxButton(this, wxID_ANY, wxT("Run"), wxDefaultPosition, wxDefaultSize, 0);
	bSizer1->Add(m_butRun, 0, wxTOP | wxBOTTOM | wxRIGHT, 5);

	fgSizer1->Add(bSizer1, 1, wxEXPAND, 5);

	m_lcUnitTests = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SORT_ASCENDING);
	fgSizer1->Add(m_lcUnitTests, 1, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 5);

	m_butClose = new wxButton(this, wxID_ANY, wxT("Close"), wxDefaultPosition, wxDefaultSize, 0);
	fgSizer1->Add(m_butClose, 0, wxALIGN_RIGHT | wxBOTTOM | wxRIGHT | wxLEFT, 5);

	this->SetSizer(fgSizer1);
	this->Layout();

	this->Centre(wxBOTH);

	m_cbRunOnStartup->SetValue(g_bRunOnStartup.getBool());
	m_cbAssertOnError->SetValue(g_bAssertOnFailure.getBool());

	m_textCtrl1->SetValue(g_strUnitTestFilter.getString());

	m_cbRunOnStartup->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &gcUnitTestForm::OnRunOnStartupClicked, this);
	m_cbAssertOnError->Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &gcUnitTestForm::OnAssertOnErrorClicked, this);
	m_textCtrl1->Bind(wxEVT_COMMAND_TEXT_UPDATED, &gcUnitTestForm::OnTextChanged, this);


	m_icoTestGood = GetGCThemeManager()->getImageHandle("#test_good");
	m_icoTestBad = GetGCThemeManager()->getImageHandle("#test_bad");

	m_ImageList.Add(wxBitmap(m_icoTestBad.getImg()->Scale(16, 16)));
	m_ImageList.Add(wxBitmap(m_icoTestGood.getImg()->Scale(16, 16)));

	m_lcUnitTests->SetImageList(&m_ImageList, wxIMAGE_LIST_NORMAL);
	m_lcUnitTests->SetImageList(&m_ImageList, wxIMAGE_LIST_SMALL);

	g_pTestWatcher->onStartEvent += guiDelegate(this, &gcUnitTestForm::onStart);
	g_pTestWatcher->onEndEvent += guiDelegate(this, &gcUnitTestForm::onEnd);

	g_pTestWatcher->onTestStartEvent += guiDelegate(this, &gcUnitTestForm::onTestStart);
	g_pTestWatcher->onTestResultEvent += guiDelegate(this, &gcUnitTestForm::onTestResult);
	g_pTestWatcher->onTestEndEnd += guiDelegate(this, &gcUnitTestForm::onTestEnd);

	if (g_bRunOnStartup.getBool())
		runTests();
}

gcUnitTestForm::~gcUnitTestForm()
{
	g_pTestWatcher->onStartEvent -= guiDelegate(this, &gcUnitTestForm::onStart);
	g_pTestWatcher->onEndEvent -= guiDelegate(this, &gcUnitTestForm::onEnd);

	g_pTestWatcher->onTestStartEvent -= guiDelegate(this, &gcUnitTestForm::onTestStart);
	g_pTestWatcher->onTestResultEvent -= guiDelegate(this, &gcUnitTestForm::onTestResult);
	g_pTestWatcher->onTestEndEnd -= guiDelegate(this, &gcUnitTestForm::onTestEnd);

	if (m_pThread)
		m_pThread->stop();

	delete m_pThread;
}

void gcUnitTestForm::onButtonClicked(wxCommandEvent& event)
{
	if (event.GetId() == m_butRun->GetId())
	{
		runTests();
	}
	else
	{
		Show(false);
	}
}

void gcUnitTestForm::runTests()
{
	m_butRun->Enable(false);
	
	if (m_pThread)
		m_pThread->stop();

	delete m_pThread;

	m_pThread = new	CUnitTestThread();
	m_pThread->start();
}

void gcUnitTestForm::onStart()
{
	m_lcUnitTests->ClearAll();

	m_lcUnitTests->InsertColumn(0, "", 0, 25);
	m_lcUnitTests->InsertColumn(1, "Test Name", 0, 375);
	m_lcUnitTests->InsertColumn(2, "Run Time");

	m_mTestIndex.clear();
}

void gcUnitTestForm::onEnd()
{
	m_butRun->Enable(true);
}

void gcUnitTestForm::onTestStart(gcString &strTest)
{
	m_mTestIndex[strTest] = m_lcUnitTests->InsertItem(-1, "");
	m_lcUnitTests->SetItem(m_mTestIndex[strTest], 1, strTest);
}

void gcUnitTestForm::onTestResult(std::pair<gcString, bool> &result)
{
	m_lcUnitTests->SetItem(m_mTestIndex[result.first], 0, "", result.second);
}

void gcUnitTestForm::onTestEnd(std::pair<gcString, uint64> &time)
{
	m_lcUnitTests->SetItem(m_mTestIndex[time.first], 2, gcString("{0}", time.second));
}

void gcUnitTestForm::OnRunOnStartupClicked(wxCommandEvent& event)
{
	g_bRunOnStartup.setValue(m_cbRunOnStartup->IsChecked());
}

void gcUnitTestForm::OnAssertOnErrorClicked(wxCommandEvent& event)
{
	g_bAssertOnFailure.setValue(m_cbAssertOnError->IsChecked());
}

void gcUnitTestForm::OnTextChanged(wxCommandEvent& event)
{
	g_strUnitTestFilter.setValue((const char*)m_textCtrl1->GetValue());
}

void gcUnitTestForm::OnClose(wxCloseEvent& event)
{
	if (!m_bShouldClose)
	{
		event.Veto();
		Show(false);
	}
	else
	{
		event.Skip();
	}
}

void gcUnitTestForm::canClose()
{
	m_bShouldClose = true;
}

void gcUnitTestForm::onShow(uint32&)
{
	Show(true);
	Raise();
}

void gcUnitTestForm::postShowEvent()
{
	uint32 res;
	showEvent(res);
}