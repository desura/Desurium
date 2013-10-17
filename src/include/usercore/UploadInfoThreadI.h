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


#ifndef DESURA_UPLOADINFOTHREADI_H
#define DESURA_UPLOADINFOTHREADI_H
#ifdef _WIN32
#pragma once
#endif

namespace UserCore
{
namespace Misc
{

class UploadInfo;

class UploadInfoThreadI
{
public:
	virtual ~UploadInfoThreadI(){;}

	virtual uint64 getHash()=0;
	virtual DesuraId getItemId()=0;
	virtual void setStart(uint32 start)=0;

	virtual Event<uint32>* getCompleteEvent()=0;
	virtual Event<UploadInfo>* getUploadProgressEvent()=0;
	virtual Event<gcException>* getErrorEvent()=0;
	virtual EventV* getActionEvent()=0;

	//BaseThread
	virtual void start()=0;
	virtual void stop()=0;
	virtual void nonBlockStop()=0;
	virtual void unpause()=0;
	virtual void pause()=0;
	virtual bool isPaused()=0;


	virtual bool isDeleted()=0;
	virtual void setDeleted()=0;

	virtual bool shouldDelMcf()=0;
	virtual void setDelMcf(bool state = true)=0;
	virtual bool hasStarted()=0;
	virtual bool isCompleted()=0;
	virtual bool hasError()=0;

	virtual uint32 getProgress()=0;

	virtual const char* getFile()=0;
	virtual const char* getKey()=0;
};

}
}

#endif //DESURA_UPLOADINFOTHREAD_H
