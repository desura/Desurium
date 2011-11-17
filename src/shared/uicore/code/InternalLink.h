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

#ifndef DESURA_INTERNALLINK_H
#define DESURA_INTERNALLINK_H
#ifdef _WIN32
#pragma once
#endif

#include "MainApp.h"
#include "ItemForm.h"

namespace UserCore
{
	namespace Misc
	{
		class NewsItem;
	}
}

class wxWindow;

class Args
{
public:
	Args(){}
	Args(std::vector<std::string> args);

	bool containsArg(std::string key);
	std::string getArgValue(std::string key);

private:
	std::map<std::string, std::string> m_mArgMap;
};

class InternalLink
{
public:
	InternalLink(wxWindow *parent);
	~InternalLink();

	void handleInternalLink(const char* link);
	void handleInternalLink(DesuraId id, uint8 action, const std::vector<std::string> &argsList = std::vector<std::string>());
	void closeForm(int32 wxId);

	void closeAll();
	void showNews(std::vector<UserCore::Misc::NewsItem*> &newsItems, std::vector<UserCore::Misc::NewsItem*> &giftItems);
	void showAppUpdate(uint32 version);

protected:
	void showPreorderPrompt(DesuraId id, bool isPreload);

	void installedWizard();
	void showSettings(Args &args);

	void installCheck(DesuraId id);
	void installTestMCF(DesuraId id, Args args);
	void uploadMCF(DesuraId id);
	void resumeUploadMCF(DesuraId id, Args args);
	void createMCF(DesuraId id);


	void cleanComplexMod(DesuraId id);
	void installItem(DesuraId id, Args args);
	void launchItem(DesuraId id, Args args);
	void verifyItem(DesuraId id, Args args);
	void uninstallMCF(DesuraId id);

	void updateItem(DesuraId id, Args args);
	
	void showEULA(DesuraId id);
	void showUpdateLog(DesuraId id);
	void showUpdateLogApp(uint32 version);

	void showGameDisk(DesuraId id, const char* exe, bool cdKey);
	void showUpdateForm(DesuraId id, Args args);
	void showCDKey(DesuraId id, Args args);
	void showExeSelect(DesuraId id, bool hasSeenCDKey = false);

	void setPauseItem(DesuraId id, bool state = true);
	void switchBranch(DesuraId id, Args args);


	void regForm(DesuraId id, gcFrame *form);
	bool checkForm(DesuraId id);

	void onUploadTrigger(ut& info);

	void showPrompt(DesuraId id, Args args);

	UI::Forms::ItemForm* showItemForm(DesuraId id, UI::Forms::INSTALL_ACTION action, bool showForm);
	UI::Forms::ItemForm* showItemForm(DesuraId id, UI::Forms::INSTALL_ACTION action, MCFBranch branch = MCFBranch(), MCFBuild build = MCFBuild(), bool showForm=true);

	bool processItemLink(bool &badLink, std::vector<gcString> &list, const char* link);
	bool switchTab(bool &badLink, std::vector<gcString> &list, const char* link);

	bool checkForPreorder(DesuraId id);

private:
	int32 m_iNewsFormId;
	int32 m_iGiftFormId;
	wxWindow* m_pParent;

	std::map<uint64, gcFrame*> m_mFormMap;
	std::vector<wxFrame*> m_vSubForms;

	std::map<gcString, gcFrame*> m_mWaitingItemFromMap;
};


#endif //DESURA_INTERNALLINK_H
