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

#ifndef COMMON_H
#define COMMON_H

#define DONT_INCLUDE_SHLOBJ

#ifdef _DEBUG
	#ifndef DEBUG
		#define DEBUG 1
	#endif
#endif

#ifdef _WIN32
	#ifndef WIN32
		#define WIN32
	#endif
#endif

#ifdef _WIN64
	#ifndef WIN64
		#define WIN64
	#endif
#endif

#if defined linux || defined __linux || defined __linux__
	#ifndef NIX
		#define NIX 1
	#endif
	
	#if defined __x86_64 || defined __amd64 || defined __x86_64__
		#define NIX64 1
	#endif
#endif

#ifdef __APPLE__
	#ifndef MACOS
		#define MACOS 1
	#endif
	
	#if defined __x86_64 || defined __amd64 || defined __x86_64__
		#define MACOS64 1
	#endif
	
	#define fopen64 fopen
	#define fseeko64 fseeko
#endif


#ifdef WIN32
	#define GCC_VERSION 0
#else
	#define GCC_VERSION (__GNUC__ * 10000 \
		+ __GNUC_MINOR__ * 100 \
		+ __GNUC_PATCHLEVEL__)
#endif

#ifdef ERROR_OUTPUT
	#error "ERROR_OUTPUT is already defined!"
#else
	#ifdef DEBUG
		#ifdef NIX
				#define ERROR_OUTPUT(error) fprintf(stdout, "(%d, son of %d) %s:%d - %s\n", getpid(), getppid(), __FILE__, __LINE__, error); fflush(stdout);
		#else
				#define ERROR_OUTPUT(error) fprintf(stdout, "%s:%d - %s\n",  __FILE__, __LINE__, error); fflush(stdout);
		#endif
	#else
		#define ERROR_OUTPUT(error)
	#endif
#endif

#ifdef ERROR_NOT_IMPLEMENTED
	#error "ERROR_NOT_IMPLEMENTED is already defined!"
#else
	#ifdef DEBUG
		#define ERROR_NOT_IMPLEMENTED fprintf(stdout, "%s:%d (%s) - NOT IMPLEMENTED!\n", __FILE__, __LINE__, __FUNCTION__); fflush(stdout);
	#else
		#define ERROR_NOT_IMPLEMENTED
	#endif
#endif

#ifdef DEBUG
	#ifdef WIN32
		#define PAUSE_DEBUGGER() __asm int 3
	#else
		#define PAUSE_DEBUGGER() asm("int $3")
	#endif
#else
	#define PAUSE_DEBUGGER()
#endif

#ifdef BUILDID_INTERNAL
	#undef BUILDID_INTERNAL
	#undef BUILDID_BETA
	#undef BUILDID_PUBLIC
#endif

#ifdef WIN32 // WINDOWS DEFINES
	#ifndef DONT_INCLUDE_WINH
		#include <windows.h>
	#else
		#include <afx.h>
	#endif
	
	#include <sys/stat.h>

	#define vsnwprintf _vsnwprintf
	#define vscprintf _vscprintf
	#define vscwprintf _vscwprintf

	typedef unsigned char uint8;
	typedef char int8;
	typedef unsigned short uint16;
	typedef short int16;
	typedef unsigned int uint32;
	typedef int int32;
	typedef unsigned long long uint64;
	typedef long long int64;

	#define DIRS_STR "\\"
	#define DIRS_CHAR '\\'
	#define DIRS_WSTR L"\\"
	#define DIRS_WCHAR L'\\'
	
	#define DIR_COMMON "Common"
	#define DIR_WCOMMON L"Common"

	#define CEXPORT __declspec(dllexport)
	#define CIMPORT __declspec(dllimport)
	
	#define BUILDID_INTERNAL 500
	#define BUILDID_BETA 300
	#define BUILDID_PUBLIC 100
	
	void inline gcSleep(uint32 miliSecs) { Sleep(miliSecs); }
	
	// mingw needs some imports
	#ifdef __MINGW32__
		#include <limits.h>
		#include <algorithm>
		
		// this is missing in the mingw headers
		// shellapi.h:
		#ifndef SEE_MASK_DEFAULT
			#define SEE_MASK_DEFAULT 0x00000000
		#endif
		// on mingw we have the XP version of NOTIFYICONDATA*
		#ifndef NOTIFYICONDATA_V3_SIZE
			#define NOTIFYICONDATAA_V3_SIZE sizeof(NOTIFYICONDATAA)
			#define NOTIFYICONDATAW_V3_SIZE sizeof(NOTIFYICONDATAW)
			#define NOTIFYICONDATA_V3_SIZE __MINGW_NAME_AW_EXT(NOTIFYICONDATA,_V3_SIZE)
		#endif
		
		// mscoree.h:
		#ifndef STARTUP_LOADER_OPTIMIZATION_MULTI_DOMAIN_HOST
			#define STARTUP_LOADER_OPTIMIZATION_MULTI_DOMAIN_HOST 0x3<<1
		#endif
		// netfw.h:
		// included in projects directly
	#endif
#endif

#if defined NIX || MACOS // UNIX
	#define _LARGEFILE_SOURCE 1
	#define _LARGEFILE64_SOURCE 1
	#define _FILE_OFFSET_BITS   64
	#define __USE_LARGEFILE64 1
	
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/wait.h>
	#include <sys/un.h>
	#include <sys/socket.h>

	#include <unistd.h>
	#include <limits.h>
	#include <errno.h>

	#include <stdint.h>
	#include <stdarg.h>
	#include <strings.h>

	#define uint8	uint8_t
	#define int8	int8_t
	#define uint16	uint16_t
	#define int16	int16_t
	#define uint32	uint32_t
	#define int32	int32_t
	#define uint64	uint64_t
	#define int64	int64_t

	#define DIRS_STR "/"
	#define DIRS_CHAR '/'
	#define DIRS_WSTR L"/"
	#define DIRS_WCHAR L'/'
	
	#define DIR_COMMON "common"
	#define DIR_WCOMMON L"common"

	#define CEXPORT __attribute__((visibility("default")))
	#define CIMPORT
	#define WINAPI

	#define OutputDebugString(out)	fprintf(stderr, "%s", out);
	#define OutputDebugStringW(out)	fprintf(stderr, "%ls", out);

	#ifdef MACOS
		#define BUILDID_INTERNAL 530
		#define BUILDID_BETA 330
		#define BUILDID_PUBLIC 130
	#elif NIX64
		#define BUILDID_INTERNAL 520
		#define BUILDID_BETA 320
		#define BUILDID_PUBLIC 120
	#else
		#define BUILDID_INTERNAL 510
		#define BUILDID_BETA 310
		#define BUILDID_PUBLIC 110
	#endif

	inline int GetLastError() { return errno; }
	void inline gcSleep(uint32 miliSecs) { usleep(miliSecs*1000); }
#endif

#ifndef BUILDID_INTERNAL
	#error "DESURA: UNKNOWN INTERNAL BUILD"
#endif

#ifndef BUILDID_BETA
	#error "DESURA: UNKNOWN BETA BUILD"
#endif

#ifndef BUILDID_PUBLIC
	#error "DESURA: UNKNOWN PUBLIC BUILD"
#endif

///////////////////////////////////////////
// UNSORTED STUFF
///////////////////////////////////////////
#define STR( t ) #t

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <string.h>

#include "tinyxml2.h"
//using namespace openutils;

#include <vector>
#include <map>
#include <deque>
#include <string>

template <typename T>
void safe_deleteA(T*& ptr)
{
	if (ptr)
		delete [] ptr;
	ptr = NULL;
}

inline void safe_delete(char*& ptr)
{
	safe_deleteA(ptr);
}

inline void safe_delete(wchar_t*& ptr)
{
	safe_deleteA(ptr);
}

inline void safe_delete(unsigned char*& ptr)
{
	safe_deleteA(ptr);
}

inline void safe_delete(const char*& ptr)
{
	safe_deleteA(ptr);
}

inline void safe_delete(const wchar_t*& ptr)
{
	safe_deleteA(ptr);
}

inline void safe_delete(const unsigned char*& ptr)
{
	safe_deleteA(ptr);
}

template <typename T>
void safe_delete(T*& ptr)
{
	if (ptr)
		delete ptr;
	ptr = NULL;
}

template <typename T>
void safe_delete(std::vector<T>& vec)
{
	for (size_t x=0; x<vec.size(); x++)
		safe_delete( vec[x] );

	vec.clear();
}

template <typename T, typename A>
void safe_delete(std::vector<T, A>& vec)
{
	for (size_t x=0; x<vec.size(); x++)
		safe_delete( vec[x] );

	vec.clear();
}

template <typename T>
void safe_delete(std::deque<T>& deq)
{
	for (size_t x=0; x<deq.size(); x++)
		safe_delete( deq[x] );

	deq.clear();
}

template <typename T, typename S>
void safe_delete(std::map<T,S*>& map)
{
	typename std::map<T,S*>::iterator it;
	typename std::map<T,S*>::iterator endit = map.end();

	for (it = map.begin(); it != endit; ++it)
		safe_delete( it->second );

	map.clear();
}

template <typename T>
void safe_delete(size_t argc, T **argv)
{
	if ( !argv )
		return;

	for ( size_t n = 0; n < argc; n++ )
		free(argv[n]);

	delete [] argv;
}



template <typename T>
class AutoDelete
{
public:
	AutoDelete()
	{
		m_pPointer = NULL;
	}

	AutoDelete(T* t)
	{
		m_pPointer = t;
	}

	~AutoDelete()
	{
		safe_delete(m_pPointer);
	}

	T* handle()
	{
		return m_pPointer;
	}

	AutoDelete<T>& operator=(AutoDelete<T>& o)
	{
		if (this != &o)
		{
			safe_delete(m_pPointer);
			m_pPointer = o.m_pPointer;
			o.m_pPointer = NULL;
		}

		return *this;
	}

	T* operator->()
	{
		return m_pPointer;
	}

	operator T*()
	{
		return m_pPointer;
	}

protected:
	T* m_pPointer;
};

#ifdef WIN32
template <typename T>
class AutoDeleteLocal : public AutoDelete<T>
{
public:
	AutoDeleteLocal(T* t) : AutoDelete<T>(t)
	{
	}

	~AutoDeleteLocal()
	{
		LocalFree(*AutoDelete<T>::m_pPointer);
		AutoDelete<T>::m_pPointer = NULL;
	}

	AutoDeleteLocal<T>& operator=(AutoDeleteLocal<T>& o)
	{
		if (this != &o)
		{
			LocalFree(*AutoDelete<T>::m_pPointer);
			AutoDelete<T>::m_pPointer = o.m_pPointer;
			o.m_pPointer = NULL;
		}

		return *this;
	}
};
#endif

template <typename T>
class AutoDeleteV
{
public:
	AutoDeleteV(T& t) : m_pPointer(t)
	{
	}

	~AutoDeleteV()
	{
		safe_delete(m_pPointer);
	}

private:
	T& m_pPointer;
};


class MCFBranch
{
public:
	MCFBranch()
	{
		m_uiBranch = 0;
	}

	bool isGlobal()
	{
		return m_bGlobal;
	}

	operator uint32() const
	{
		return m_uiBranch;
	}

	static MCFBranch BranchFromInt(uint32 branch, bool global=false)
	{
		return MCFBranch(branch, global);
	}

protected:
	MCFBranch(uint32 branch, bool global)
	{
		m_uiBranch = branch;
		m_bGlobal = global;
	}

private:
	bool m_bGlobal;
	uint32 m_uiBranch;
};


class MCFBuild
{
public:
	MCFBuild()
	{
		m_uiBuild = 0;
	}

	operator uint32() const
	{
		return m_uiBuild;
	}

	static MCFBuild BuildFromInt(uint32 build)
	{
		return MCFBuild(build);
	}

protected:
	MCFBuild(uint32 build)
	{
		m_uiBuild = build;
	}

private:
	uint32 m_uiBuild;
};



#ifdef WIN32
	#define COMMONAPP_PATH "Desura\\DesuraApp"
	#define COMMONAPP_PATH_W L"Desura\\DesuraApp"
#else
	#define COMMONAPP_PATH "appdata"
	#define COMMONAPP_PATH_W L"appdata"
#endif

#define THROWS( e )

#define UPDATEFILE	"desura_update.mcf"
#define UPDATEFILE_W L"desura_update.mcf"
#define UPDATEXML	"desura_appfiles.xml"
#define UPDATEXML_W L"desura_appfiles.xml"

#define PRIMUPDATE "http://www.desura.com/api/appupdate"

#define REGRUN "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\Desura"
#define APPID	"HKEY_LOCAL_MACHINE\\SOFTWARE\\Desura\\DesuraApp\\appid"
#define APPBUILD "HKEY_LOCAL_MACHINE\\SOFTWARE\\Desura\\DesuraApp\\appver"

//dont change the name as the uninstaller needs it!
#define SERVICE_NAME "Desura Install Service"
#define SERVICE_DESC "Desura Install Service"

//this is disable the warning for std::vector not being an export class.
#pragma warning( disable: 4251 )


#define UNKNOWN_ITEM 0xFFFFFFFF

//#define REDEFINE_PRINTF


#define DEF_CHUNKSIZE (1024*1024*500) //500kb

#define USERAGENT "Desura Default"


#include "gcError.h"
#include "Event.h"
#include "util/UtilMisc.h"
#include "util/UtilString.h"
#include "util/UtilWeb.h"
#include "util/UtilFs.h"
#include "util/UtilFsPath.h"
#include "util/UtilOs.h"
#include "DesuraId.h"


#ifdef DESURA_GCSTRING_H

template <class charT, class traits> inline
std::basic_ostream<charT,traits>& operator<<(std::basic_ostream<charT,traits>& strm, const gcException& t)
{
	std::basic_string<charT> formatString = Template::converToStringType<charT>("{0} [{1}.{2}]");
	std::basic_string<charT> res = Template::Format(formatString.c_str(), t.getErrMsg(), t.getErrId(), t.getSecErrId());

	strm << res.c_str();	
	return strm;
}

#endif



#ifdef MODCORE_LOG
	#include "Log.h"
#else
	#include "LogBones.h"
#endif

#ifdef WIN32
	#include "util/UtilWindows.h"
#endif

#ifdef INCLUDE_CVAR_CONCOMMAND
	#include "managers\ConCommand.h"
	#include "managers\CVar.h"
#endif

#ifdef INCLUDE_WXWIDGETS
	#include <wx\wx.h>
#endif

#define Sleep gcSleep
#define sleep gcSleep

inline bool HasAnyFlags(uint32 value, uint32 flags)
{
	return (value&flags?true:false);
}

inline bool HasAllFlags(uint32 value, uint32 flags)
{
	return ((value&flags) == flags);
}

#include <memory>

#if defined(NIX) || defined(__MINGW32__) || defined(MACOS)
#  ifdef __ICC
#    include <boost/weak_ptr.hpp>
#    include <boost/shared_ptr.hpp>
#    define WeakPtr boost::weak_ptr
#    define SmartPtr boost::shared_ptr
#  else
#    define WeakPtr std::weak_ptr
#    define SmartPtr std::shared_ptr
#  endif
#else
#  define WeakPtr std::tr1::weak_ptr
#  define SmartPtr std::tr1::shared_ptr
#endif

namespace Safe
{
	inline int strcpy(char* dest, size_t destSize, const char* source)
	{
	#ifdef WIN32
		return ::strncpy_s(dest, destSize, source, _TRUNCATE);
	#else
		return ::strncpy(dest, source, destSize) == source ? 0 : 1;
	#endif
	}

	inline void strncpy(char* dest, size_t destSize, const char* source, size_t count)
	{
	#ifdef WIN32
		::strncpy_s(dest, destSize, source, count);
	#else
		::strncpy(dest, source, count);
	#endif
	}

	inline void wcscpy(wchar_t* dest, size_t destSize, const wchar_t* source)
	{
	#ifdef WIN32
		::wcsncpy_s(dest, destSize, source, _TRUNCATE);
	#else
		::wcsncpy(dest, source, destSize);
	#endif
	}

	inline void wcsncpy(wchar_t* dest, size_t destSize, const wchar_t* source, size_t count)
	{
	#ifdef WIN32
		::wcsncpy_s(dest, destSize, source, count);
	#else
		::wcsncpy(dest, source, count);
	#endif
	}

	inline void snprintf(char* dest, size_t destSize, const char* format, ...)
	{
		va_list args;
		va_start(args, format);

	#if defined(WIN32) && !defined(__MINGW32__)
		_vsnprintf_s(dest, destSize, _TRUNCATE, format, args);
	#else
		::vsnprintf(dest, destSize, format, args);
	#endif

		va_end(args);
	}

	inline void snwprintf(wchar_t* dest, size_t destSize, const wchar_t* format, ...)
	{
		va_list args;
		va_start(args, format);

	#ifdef WIN32
		_vsnwprintf_s(dest, destSize, _TRUNCATE, format, args);
	#else
		::vswprintf(dest, destSize, format, args);
	#endif

		va_end(args);
	}

	inline FILE* fopen(const char* file, const char* args)
	{
	#ifdef WIN32
		FILE* fileHandle = NULL;
		fopen_s(&fileHandle, file, args);
		return fileHandle;
	#else
		return ::fopen(file, args);
	#endif
	}

	inline char* strtok(char *strToken, const char *strDelimit, char **context)
	{
	#ifdef WIN32
		return strtok_s(strToken, strDelimit, context);
	#else
		return ::strtok_r(strToken, strDelimit, context);
	#endif
	}

	inline int stricmp(const char* str1, const char* str2)
	{
	#ifdef WIN32
		return _stricmp(str1, str2);
	#else
		return ::strcasecmp(str1, str2);
	#endif
	}

	inline int strnicmp(const char* str1, const char* str2, size_t count)
	{
	#ifdef WIN32
		return _strnicmp(str1, str2, count);
	#else
		return ::strncasecmp(str1, str2, count);
	#endif
	}

	inline void strcat(char* dest, size_t destSize, const char* source)
	{
	#if defined(WIN32) && !defined(__MINGW32__)
		strcat_s(dest, destSize, source);
	#else
		::strncat(dest, source, destSize);
	#endif
	}

	inline size_t strlen(const char* str, size_t strSize)
	{
	#if defined(WIN32) && !defined(__MINGW32__)
		return strnlen_s(str, strSize);
	#else
		return ::strnlen(str, strSize);
	#endif
	}

	inline size_t wcslen(const wchar_t* str, size_t strSize)
	{
	#if defined(WIN32) && !defined(__MINGW32__)
		return wcsnlen_s(str, strSize);
	#else
		return ::wcsnlen(str, strSize);
	#endif
	}


	inline void strcpy(char** dest, const char* src, size_t srcSize)
	{
		safe_delete(*dest);

		if (!src)
			return;

		size_t nLen = Safe::strlen(src, srcSize);

		(*dest) = new char[nLen+1];
		Safe::strcpy(*dest, nLen+1, src);
	}

	inline void wcscpy(wchar_t** dest, const wchar_t* src, size_t srcSize)
	{
		safe_delete(*dest);

		if (!src)
			return;

		size_t nLen = Safe::wcslen(src, srcSize);

		(*dest) = new wchar_t[nLen+1];
		Safe::wcscpy(*dest, nLen+1, src);
	}


	inline int wcsicmp(const wchar_t* str1, const wchar_t* str2)
	{
	#ifdef WIN32
		return _wcsicmp(str1, str2);
	#else
		return ::wcscasecmp(str1, str2);
	#endif
	}
}

template <typename T>
T Clamp(T val, T minVal, T maxVal)
{
#if defined(WIN32) && !defined(__MINGW32__)
	return max(min(val, maxVal), minVal);
#else
	return std::max(std::min(val, maxVal), minVal);
#endif
}

#include <branding/branding.h>

#define PRODUCT_NAME_CAT(x) PRODUCT_NAME x
#define PRODUCT_NAME_CATW(x) _T(PRODUCT_NAME) x

// some glib overrides for ICC
// in ICC __deprecated__ does something different, so glib 2.30 is incompatible with ICC
#ifdef __ICC
#  define __GLIB_H_INSIDE__ 1
#  include <glib/gmacros.h>
#  undef __GLIB_H_INSIDE__
#  ifdef G_DEPRECATED_FOR
#    undef G_DEPRECATED_FOR
#    define G_DEPRECATED_FOR(a)
#  endif
#endif

#endif
