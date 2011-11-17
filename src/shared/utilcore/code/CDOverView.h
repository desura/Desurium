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

#ifndef DESURA_CDOVERVIEW_H
#define DESURA_CDOVERVIEW_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"

enum
{
	CD_CLOSE = 1000,
	CD_UPLOAD,
	CD_GENERATE,
};

class CDOverView : public gcPanel 
{
public:
	CDOverView(wxWindow* parent);
	~CDOverView();	

protected:
	gcButton* m_butGenerate;
	gcButton* m_butUpload;
	gcButton* m_butCancel;

	wxStaticText* m_labGenerate;
	wxStaticText* m_labUpload;

private:
	
};

#endif //DESURA_CDOVERVIEW_H
