/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>

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

#ifndef DESURA_GCJSLINKS_H
#define DESURA_GCJSLINKS_H
#ifdef _WIN32
#pragma once
#endif

#include "gcJSBase.h"



class DesuraJSLinks : public DesuraJSBase<DesuraJSLinks>
{
public:
	DesuraJSLinks();

private:
	void switchTab(int32 tabId);
	void loadUrl(int32 tabId, gcString url);

	JSObjHandle internalLink(ChromiumDLL::JavaScriptFactoryI *factory, ChromiumDLL::JavaScriptContextI* context, JSObjHandle object, std::vector<JSObjHandle> &args);
};

#endif //DESURA_GCJSLINKS_H
