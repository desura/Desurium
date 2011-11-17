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

#ifndef DESURA_UPDATE_FORM_NIX_H
#define DESURA_UPDATE_FORM_NIX_H
#ifdef _WIN32
#pragma once
#endif

#include "XLibWindow.h"
#include "Common.h"
#include "umcf/UMcf.h"
#include "UMcfEx.h"

class UpdateForm : public XlibWindow
{
public:
	UpdateForm();
	~UpdateForm();

	void installOnly();
	void downloadAndInstall();
	
	bool isCanceled()
	{
		return m_bCanceled;
	}
	
	bool wasInstalling()
	{
		return m_bInstalling;
	}
	
protected:
	void onProgressN(unsigned int& prog);
	void onProgressD(Prog_s& info);
	
	virtual void onCancel();
	
private:
	bool m_bCanceled;
	bool m_bInstalling;
	UMcf* m_pUmcf;
};


#endif
