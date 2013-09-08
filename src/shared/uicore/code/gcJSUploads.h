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

#ifndef DESURA_GCJSUPLOADS_H
#define DESURA_GCJSUPLOADS_H
#ifdef _WIN32
#pragma once
#endif

#include "gcJSBase.h"

namespace UserCore
{
namespace Misc
{
	class UploadInfoThreadI;
}
}

class DesuraJSUploadInfo : public DesuraJSBase<DesuraJSUploadInfo>
{
public:
	DesuraJSUploadInfo();

	virtual bool preExecuteValidation(const char* function, uint32 functionHash, JSObjHandle object, size_t argc, JSObjHandle* argv);

	
private:
	void* getUploadFromId(int32 uid);

	bool isValid(UserCore::Misc::UploadInfoThreadI* upload);
	bool isComplete(UserCore::Misc::UploadInfoThreadI* upload);
	bool hasError(UserCore::Misc::UploadInfoThreadI* upload);
	bool hasStarted(UserCore::Misc::UploadInfoThreadI* upload);
	bool isPaused(UserCore::Misc::UploadInfoThreadI* upload);
	bool shouldDeleteMcf(UserCore::Misc::UploadInfoThreadI* upload);

	void setDeleteMcf(UserCore::Misc::UploadInfoThreadI* upload, bool del);
	void remove(UserCore::Misc::UploadInfoThreadI* upload);
	void pause(UserCore::Misc::UploadInfoThreadI* upload);
	void unpause(UserCore::Misc::UploadInfoThreadI* upload);
	void cancel(UserCore::Misc::UploadInfoThreadI* upload);
	void exploreMcf(UserCore::Misc::UploadInfoThreadI* upload);

	void* getItem(UserCore::Misc::UploadInfoThreadI* upload);
	int32 getProgress(UserCore::Misc::UploadInfoThreadI* upload);
	gcString getFileName(UserCore::Misc::UploadInfoThreadI* upload);
	gcString getUploadId(UserCore::Misc::UploadInfoThreadI* upload);
	gcString getItemId(UserCore::Misc::UploadInfoThreadI* upload);

	uint32 m_uiIsValidHash;
	uint32 m_uiGetUploadFromIdHash;
};

#endif //DESURA_GCJSUPLOADS_H
