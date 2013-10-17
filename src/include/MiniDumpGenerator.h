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

#ifndef MDUMP_HANDLE_H
#define MDUMP_HANDLE_H

#ifdef NIX
#include <cstring>
#endif

namespace google_breakpad
{
	class ExceptionHandler;
}

#ifdef WIN32
class FilterWrapper;
#endif

typedef bool (*CrashCallbackFn)(const char*);

#ifdef WIN32 
#define CHAR_T wchar_t
#else
#define CHAR_T char
#endif

//! Handles crashes in the program and writes a dump out to a file
class MiniDumpGenerator
{
public:
	MiniDumpGenerator();
	~MiniDumpGenerator();

	//! Enables/Disable showing of a message box on crash
	//!
	void showMessageBox(bool state);
	
	//! Set the desura user name of the current user
	//!	
	void setUser(const CHAR_T* user);
	
	//! Set if the crash should be auto uploaded once saved
	//!
	void setUpload(bool state);
	
	//! Sets the level of detail for crash dump. 0 to 4
	//!
	void setDumpLevel(unsigned char level);
	
	//! Sets a callback function when a crash occures
	//!
	void setCrashCallback(CrashCallbackFn callback);

#ifdef WIN32
	virtual bool filter(FilterWrapper* fw);
	virtual bool complete(const wchar_t* dump_path, const wchar_t* minidump_id, FilterWrapper* fw, bool succeeded);	
	virtual void getDumpPath(wchar_t *buffer, size_t bufSize);
#else
	virtual bool complete(const char* dump_path, const char* minidump_id, bool succeeded);
#endif

protected:
	bool dumpreport(const CHAR_T* args);	

private:
	bool m_bCreatedHandle;
	bool m_bShowMsgBox;
	bool m_bNoUpload;

	CHAR_T *m_szUser;
	CrashCallbackFn m_pCrashCallback;
	
	static google_breakpad::ExceptionHandler *s_pExceptionHandler;
};

#endif
