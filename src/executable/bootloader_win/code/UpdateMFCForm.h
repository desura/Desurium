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

#ifndef DESURA_UPDATEFORM_H
#define DESURA_UPDATEFORM_H
#ifdef _WIN32
#pragma once
#endif

#include "Resource.h"
#include "DesuraProgress.h"


#ifndef UTILWEB_INCLUDE

typedef struct
{
	double dltotal;
	double dlnow;
	double ultotal;
	double ulnow;
	bool abort;
} Prog_s;

#endif

class UpdateForm : public CDialog
{
public:
	UpdateForm(int updateType, CWnd* pParent = NULL);
	~UpdateForm();

	INT_PTR DoModal();

	bool isInit(){return m_bInit;}
	void onProgressN(unsigned int& prog);
	void onProgressD(Prog_s& p);

	DesuraProgress* getProgressControl(){return m_Progress;}

protected:
	void UpdateMcf();
	void UpdateFiles();

	virtual BOOL OnInitDialog();

private:
	HICON m_hIcon;
	DesuraProgress *m_Progress;

	bool m_bInit;
	int m_updateType;

	DECLARE_MESSAGE_MAP()
};



#endif //DESURA_UPDATEFORM_H
