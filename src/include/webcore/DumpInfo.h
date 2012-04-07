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

#ifndef DESURA_DUMPINFO_H
#define DESURA_DUMPINFO_H
#ifdef _WIN32
#pragma once
#endif

#include "webcore/DLLVersion.h"

namespace WebCore
{
	namespace Misc
	{

		class DumpInfo
		{
		public:
			DumpInfo()
			{
				nextFileSize = 0;
				numberDone = 0;
				stop = false;
			}

			volatile bool stop;
			gcString dumpFolderPath;
			gcString user;

			Event<uint8> onProgressEvent;


			std::vector<gcString> fileStack;

			/// functions for upload
			uint32 nextFileSize;
			uint32 numberDone;

			void onProgress(Prog_s &prog)
			{
				double unitProg = (double)100/(double)fileStack.size();
				double curProg = prog.ultotal*unitProg/nextFileSize;

				uint8 porg = (uint8)(numberDone*unitProg + curProg);
				onProgressEvent(porg);

				prog.abort = stop;
			}
		};

	}
}

#endif //DESURA_DUMPINFO_H
