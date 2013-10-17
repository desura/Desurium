/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Karol Herbst <git@karolherbst.de>

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


#ifndef DESURA_GUIDOWNLOADPROVIDER_H
#define DESURA_GUIDOWNLOADPROVIDER_H
#ifdef _WIN32
#pragma once
#endif

#include "mcfcore/DownloadProvider.h"

namespace UserCore
{
namespace Misc
{

//! Holds information about a Mcf provider
//!
class GuiDownloadProvider
{
public:
	GuiDownloadProvider()
	{
		action = 0;
	}

	GuiDownloadProvider(uint32 a, MCFCore::Misc::DownloadProvider* dp)
	:	action(a),
		provider(dp)
	{}

	GuiDownloadProvider(GuiDownloadProvider& dp)
	:	action(dp.action),
		provider(dp.provider)
	{}

	uint32 action;
	MCFCore::Misc::DownloadProvider provider;
};

}
}

#endif //DESURA_GUIDOWNLOADPROVIDER_H
