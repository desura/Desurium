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
#include "Console.h"
#include "managers/ConCommand.h"
#include "managers/CVar.h"
#include "MainApp.h"

#include "managers/Managers.h"

CONCOMMAND(cmdlist, "cmdlist")
{
	std::vector<ConCommand*> vList;
	GetCCommandManager()->getConCommandList(vList);
	
	for (size_t x=0; x<vList.size(); x++)
	{
		ConCommand *temp = vList[x];

		if (!temp)
			continue;

		Msg(gcString("-{0}\n", temp->getName()));
	}

	Msg(gcString("{0} Commands in total.\n\n", vList.size()));
}

CONCOMMAND(cvarlist, "cvarlist")
{
	std::vector<CVar*> vList;
	GetCVarManager()->getCVarList(vList);

	uint32 skiped = 0;

	std::sort(begin(vList), end(vList), [](CVar* pA, CVar* pB)
	{
		return std::string(pA->getName()) < std::string(pB->getName()); 
	});

	for (size_t x=0; x<vList.size(); x++)
	{
		CVar *temp = vList[x];

		if (!temp)
			continue;

		if (temp->getFlags() & CFLAG_ADMIN)
		{
			skiped++;
			continue;
		}

		Msg(gcString("-{0} : {1}\n", temp->getName(), temp->getString()));
	}

	Msg(gcString("{0} Convars in total.\n\n", vList.size()-skiped));
}



//both these concommands are handled by the console form
CONCOMMAND(condump, "condump"){}
CONCOMMAND(clear, "clear"){}

CONCOMMAND(crashconsole, "crashconsole")
{
	for (int x=0; x< 500; x++)
	{
		Msg("Im gona crash you little console!!\n");
		Msg("Im gona crash you little console!!\n");
		Msg("Im gona crash you little console!!\n");
	}
}

// log everything to a text window (GUI only of course)
class  wxLogRichTextCtrl : public wxLog
{
public:
	wxLogRichTextCtrl(Console *pTextCtrl)
	{
		m_pTextCtrl = pTextCtrl;
	}

protected:
	// implement sink function
	virtual void DoLogString(const wxString& szString, time_t t)
	{
		if (m_pTextCtrl)
			m_pTextCtrl->appendText(szString.c_str().AsWChar());
	}

	void wxSUPPRESS_DOLOGSTRING_HIDE_WARNING();

private:
	// the control we use
	Console *m_pTextCtrl;
	wxDECLARE_NO_COPY_CLASS(wxLogRichTextCtrl);
};




///////////////////////////////////////////////////////////////////////////////
/// Log Form
///////////////////////////////////////////////////////////////////////////////

Console::Console(wxWindow* parent) : gcFrame(parent, wxID_ANY, wxT("#CS_TITLE"), wxDefaultPosition, wxSize( 400,300 ), wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL, true)
{
	Bind(wxEVT_CLOSE_WINDOW, &Console::onWindowClose, this);
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &Console::onSubmitClicked, this);

	SetMinSize( wxSize(300,300) );
	setupPositionSave("log", false);

	m_rtDisplay = new wxRichTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxHSCROLL|wxVSCROLL );

	m_tbInfo = new gcComboBox( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, NULL, wxTE_PROCESS_ENTER|wxWANTS_CHARS );
	m_tbInfo->Bind(wxEVT_KEY_DOWN, &Console::onKeyDown, this);

	m_butSubmit = 0;
	
	m_pSizer = new wxBoxSizer( wxHORIZONTAL );
	m_pSizer->Add( m_tbInfo, 1, wxALIGN_CENTER_VERTICAL|wxTOP|wxBOTTOM|wxLEFT, 5 );
	

	wxFlexGridSizer* fgSizer14;
	fgSizer14 = new wxFlexGridSizer( 3, 1, 0, 0 );
	fgSizer14->AddGrowableCol( 0 );
	fgSizer14->AddGrowableRow( 1 );
	fgSizer14->SetFlexibleDirection( wxBOTH );
	fgSizer14->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

	fgSizer14->Add( 0, 5, 1, wxEXPAND, 5 );
	fgSizer14->Add( m_rtDisplay, 1, wxEXPAND | wxRIGHT|wxLEFT, 5 );
	fgSizer14->Add( m_pSizer, 1, wxEXPAND|wxALIGN_CENTER_HORIZONTAL, 5 );
	
	this->SetSizer( fgSizer14 );
	this->Layout();
	
	m_tbInfo->SetFocus();
	setupAutoComplete();
	
	consoleTextEvent += guiDelegate(this, &Console::onConsoleText);
	showEvent += guiDelegate(this, &Console::onShow);

	wxLog *old_log = wxLog::SetActiveTarget( new wxLogRichTextCtrl( this ) );
	delete old_log;

	m_bCenterOnParent = false;
}

Console::~Console()
{
	wxLog *old_log = wxLog::SetActiveTarget(NULL);
	delete old_log;
}

void Console::setupAutoComplete()
{
	std::vector<ConCommand*> vCCList;
	GetCCommandManager()->getConCommandList(vCCList);

	std::vector<CVar*> vCVList;
	GetCVarManager()->getCVarList(vCVList);


	uint32 cc = vCCList.size();
	uint32 cv = vCVList.size();

	wxArrayString choices;

	for (uint32 x=0; x<cc; x++)
	{
		ConCommand *temp = vCCList[x];

		if (!temp)
			continue;

		choices.Add(wxString(temp->getName()));
	}

	for (uint32 x=0; x<cv; x++)
	{
		CVar *temp = vCVList[x];

		if (!temp || temp->getFlags() & CFLAG_ADMIN)
			continue;

		choices.Add(wxString(temp->getName()));
	}

	choices.Add(wxString("clear"));
	choices.Add(wxString("condump"));

	m_tbInfo->AutoComplete(choices);
}


void Console::onShow(uint32&)
{
	if (m_bCenterOnParent)
	{
		centerOnParent();
		m_bCenterOnParent = false;
	}

	Show(true);
	Raise();
}

void Console::postShowEvent()
{
	uint32 res;
	showEvent(res);
}

void Console::setSize()
{
	if (loadSavedWindowPos() == false)
		m_bCenterOnParent = true;
	
	enablePositionSave();
}

void Console::applyTheme()
{	
	SetTitle(Managers::GetString(L"#CS_TITLE"));

	m_tbInfo->applyTheme(); // LINUX TODO
	delete m_butSubmit;
	m_butSubmit = new gcButton(this, wxID_ANY, Managers::GetString(L"#SUBMIT"), wxDefaultPosition, wxDefaultSize, 0 );
	m_pSizer->Add( m_butSubmit, 0, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxALL, 5 );	

	Managers::LoadTheme(m_rtDisplay, "textbox");

	gcFrame::applyTheme();
	loadFrame(wxDEFAULT_FRAME_STYLE);

	//we do this here as the console form gets init before the cvar manager and it needs to use cvars for last pos
	setSize();
}

void Console::onConsoleText(ConsoleText_s& text)
{
	long size = m_rtDisplay->GetLastPosition();
	if ( size > 50000)
	{
		long stop = size - 50000;
		m_rtDisplay->Remove(0, stop);
	}

	bool endReturn = (text.str[text.str.size()-1] == '\n');

	if (endReturn)
		text.str.erase(text.str.end()-1);

	std::vector<std::string> tList;
	UTIL::STRING::tokenize(gcString(text.str), tList, "\n");

	for (std::vector<std::string>::iterator it=tList.begin(); it != tList.end(); ++it)
	{
		gcWString szText(*it);

		wxDateTime now = wxDateTime::Now();
		m_rtDisplay->AppendText(now.Format("%H:%M\t").c_str());

		if (szText.size() > 0)
		{
			m_rtDisplay->BeginTextColour( wxColor(text.col) );
			m_rtDisplay->AppendText(szText.c_str());
			m_rtDisplay->EndTextColour();
		}

		if (it+1 != tList.end())
			m_rtDisplay->AppendText(L"\n");
	}

	if (endReturn)
		m_rtDisplay->AppendText(L"\n");

	m_rtDisplay->MoveEnd();
	m_rtDisplay->DiscardEdits();
	m_rtDisplay->ShowPosition(m_rtDisplay->GetLastPosition());
}


void Console::appendText(gcWString text, Color col)
{
#ifdef DEBUG
	OutputDebugStringW(text.c_str());
#endif

	ConsoleText_s c;
	c.str = text;
	c.col = col;
	consoleTextEvent(c);
}


void Console::onWindowClose( wxCloseEvent& event )
{
	if (this->GetId() == event.GetId())
	{
		Show(false);
		event.Veto();
	}
}

void Console::onSubmitClicked( wxCommandEvent& event )
{
	processCommand();
}

void Console::onKeyDown( wxKeyEvent& event )
{ 
	if (event.GetKeyCode() == WXK_NUMPAD_ENTER || event.GetKeyCode() == WXK_RETURN)
		processCommand();

	event.Skip(); 
}

void Console::processCommand()
{
	wxString temp = m_tbInfo->GetValue();

	if (temp.size() == 0)
		return;
	
	const char* cString = temp.c_str().AsChar();
	std::vector<gcString> vArgList;

	size_t argLen = strlen(cString);
	if (argLen > 0)
	{
		char quote = 0;
	
		size_t lastIndex = 0;
		size_t remove = 0;
		for (size_t x=0; x<=argLen; x++)
		{
			if ((cString[x] == '\'' || cString[x] == '"') && cString[x] != '\\')
			{
				if (quote == cString[x])
				{
					remove = 1;
					quote = 0;
				}
				else
				{
					lastIndex++;
					quote = cString[x];
				}
			}
			else if ((quote == 0 && cString[x] == ' ') || cString[x] == '\0')
			{
				vArgList.push_back( gcString().append(cString+lastIndex, x-lastIndex-remove) );
				lastIndex = x+1;
				remove = 0;
			}
		}
	}

	if (vArgList.size() == 0)
		vArgList.push_back( gcString(cString) );

	m_tbInfo->Insert(cString, 0);
	Color c(0,150,255,255);
	MsgCol(&c, gcString("] {0}\n", cString));

	if (vArgList[0] == "condump")
	{
		conDump();
	}
	else if (vArgList[0] == "clear")
	{
		m_rtDisplay->Clear();
	}
	else if (vArgList[0].find("desura://") != std::string::npos)
	{
		g_pMainApp->handleInternalLink(cString);
	}
	else
	{
		ConCommand * cc = GetCCommandManager()->findCCommand( vArgList[0].c_str() );
		CVar * cv = GetCVarManager()->findCVar( vArgList[0].c_str() );

		if (cc)
			cc->Call(vArgList);
		else if (cv)
			cv->parseCommand(vArgList);
		else
			Msg(gcString("Couldn't find anything related to {0}.\n", vArgList[0]));
	}

	m_tbInfo->SetValue(L"");
}

void Console::conDump()
{
	Color c(0,150,255,255);
	MsgCol(&c, gcString("] {0}\n", "condump"));

	gcString dumpFile;
	bool fileExists = false;
	uint32 x=0;

	const char *comAppPath = GetUserCore()->getAppDataPath();

	do
	{
		dumpFile = gcString("{0}{2}condump{1}.txt", comAppPath, x, DIRS_STR);
		fileExists = UTIL::FS::isValidFile(UTIL::FS::PathWithFile(dumpFile));
		x++;
	}
	while (fileExists);

	try
	{
		UTIL::FS::FileHandle fh(dumpFile.c_str(), UTIL::FS::FILE_WRITE);

		wxString logStr = m_rtDisplay->GetValue();
		gcString clog((const wchar_t*)logStr.c_str());

		fh.write(clog.c_str(), clog.size());
	}
	catch (gcException &e)
	{
		Msg(gcString("Failed to write to condump file: {0}\n", e));
	}

	Msg(gcString("Console Dump Saved To:\n{0}\n\n", dumpFile));
}
