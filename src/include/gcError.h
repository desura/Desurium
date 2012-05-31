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

#ifndef DESURA_GCEXCEPTION_H
#define DESURA_GCEXCEPTION_H
#ifdef _WIN32
#pragma once
#endif

#include <string>
#include <stdarg.h>

//Error Id's
enum ERROR_ID
{
	WARN_OK = 0,

	ERR_UNKNOWNERROR,
	ERR_FAILEDSEEK,
	ERR_PARTWRITE,
	ERR_FAILEDWRITE,
	ERR_PARTREAD,				// 5

	ERR_FAILEDREAD,
	ERR_INVALID,
	ERR_NULLHANDLE,
	ERR_INVALIDFILE,
	ERR_ZEROFILE,				// 10

	ERR_ZEROSIZE,
	ERR_BZ2CFAIL,
	ERR_BZ2CFAIL_ZEROSIZE,
	ERR_BZ2CFAIL_FULLBUFF,
	ERR_BZ2DFAIL,				// 15

	ERR_INVALIDDATA,
	ERR_BADPATH,
	ERR_BADHEADER,
	ERR_XML_NOPRIMENODE,
	ERR_THREADERROR,			// 20

	ERR_SAVE_THREAD,
	ERR_SAVE_NOFILES,
	ERR_SAVE_NOHEADER,
	ERR_NOFILE,
	ERR_DUPSRC,					// 25

	ERR_WEBDL_FAILED,
	ERR_BUFF_TOSMALL,
	ERR_NULLSRCFILE,
	ERR_NULLDESTFILE,
	ERR_LIBRARY_NOTLOADED,		// 30

	ERR_LIBRARY_LOADFAILED,
	ERR_WEBLOCKED,
	ERR_BADCURLHANDLE,
	ERR_BADURL,
	ERR_BADID,					// 35

	ERR_BADXML,
	ERR_BADRESPONSE,
	ERR_BADSTATUS,
	ERR_ALREADYLOGGEDIN,
	ERR_COMPLETED,				// 40

	ERR_NOUSER,
	ERR_BADITEM,
	ERR_BAD_PORU,
	ERR_NULLWEBCORE,
	ERR_BADCLASS,				// 45

	ERR_PARENT_NOTINSTALLED,
	ERR_INVALIDMCF,
	ERR_INVALIDSIZE,
	ERR_CSUM_FAILED,
	ERR_HANDLE_INUSE,			// 50

	ERR_ZEROFILECOUNT,
	ERR_NOUPDATE,
	ERR_NULLSCMANAGER,
	ERR_NULLSERVICE,
	ERR_SERVICE,				// 55

	ERR_PIPE,
	ERR_WILDCARD,
	ERR_LIBCURL,
	ERR_SOCKET,
	ERR_MCFSERVER,				// 60

	ERR_LAUNCH,
	ERR_PERMISSIONS,
	ERR_HASHMISSMATCH,
	ERR_IPC,
	ERR_CDKEY,					// 65

	ERR_XMPP,
	ERR_WIN,
	ERR_BZ2,
	ERR_TOOLINSTALL,
	ERR_V8,						// 70

	ERR_UNKNOWNDISTRO,
	ERR_UNKNOWNARCH,
	ERR_USERCANCELED,
	ERR_UNSUPPORTEDPLATFORM,
	ERR_NO32LIBS,
	ERR_NOBITTEST,

	ERR_LASTERR,
};


//Error Msgs
const static char* errMsg [] =
{
	"OK",
	"An unknown error occurred",
	"Failed to seek in the file",
	"Only part of the file could be written.",
	"Failed to write to the file",
	"Only part of the file could be read",		// 5
	"Failed to read from the file",
	"The thing is invalid",
	"The file handle was null",
	"The file was invalid",
	"The file size was zero",					// 10
	"The size was zero",
	"Failed to compress the data using BZIP2",
	"Failed to compress the data using BZIP2 because the size was zero",
	"Failed to compress the data using BZIP2 because out buffer is full",
	"Failed to decompress the data using BZIP2",	// 15
	"The data was invalid",
	"The file path was NULL or invalid",
	"The file header was NULL or invalid",
	"Cannot find the primary node in the XML",
	"The thread had a generic error",			// 20
	"The save thread had an generic error",
	"No files to save",
	"No header to save",
	"No file found",
	"Duplicate source detected",				// 25
	"Download from web failed",
	"The buffer is to small",
	"The source file is NULL or invalid",
	"The destination file is NULL or invalid",
	"The library hasn't been loaded yet",		// 30
	"Failed to load library",
	"The web control is in use by another thread",
	"The curl handle for this web control is NULL",
	"The URL is ether NULL or incorrect",
	"Bad ID",									// 35
	"Bad XML",
	"Bad response",
	"Bad status",
	"User is all ready logged in",
	"All ready completed",						// 40
	"User is not logged in",
	"Bad item",
	"Bad password or user name",
	"WebCore hasn't been initialised",
	"NULL pointers to either WebCore or UserCore",	// 45
	"The parent item is not installed!",
	"The file is not an MCF file",
	"The item size is to big or out of bounds",
	"The check sum on the item failed",
	"The file handle is all ready in use",		// 50
	"There are no files to do work on",
	"There is no update available for this item",
	"Failed to open Service Control Manager",
	"Failed to open or create service",
	"Failed to perform action on service",		// 55
	"Generic pipe error",
	"Generic WildCard error",
	"Curl library had an error",
	"Socket error",
	"MCF Server Error",							// 60
	"Failed to launch item",
	"Don't have the correct permissions",
	"The file hash didn't match the expected hash",
	"There was an error with IPC",
	"Generic CD key error",						// 65
	"Generic XMPP error",
	"Windows API error",
	"BZ2 error",
	"Error with tool install",
	"JavaScript exception",						// 70
	"Your Linux distribution could not be determined.\n\nPlease contact support@desura.com or refer to\nhttp://www.desura.com/groups/desura/linux",
	"Your CPU archtecture could not be determined.\n\nPlease contact support@desura.com or refer to\nhttp://www.desura.com/groups/desura/linux",
	"User cancelled action",
	"This platform is unsupported",
	"You need 32 bit libraries to play this game",
	"Not compiled with 32 bit application support.",

	"THIS SHOULDN'T BE USED AS AN ERR ID!",
};

//these are bzip2 return errors

#define BZ_OK                0
#define BZ_RUN_OK            1
#define BZ_FLUSH_OK          2
#define BZ_FINISH_OK         3
#define BZ_STREAM_END        4
#define BZ_SEQUENCE_ERROR    (-1)
#define BZ_PARAM_ERROR       (-2)
#define BZ_MEM_ERROR         (-3)
#define BZ_DATA_ERROR        (-4)
#define BZ_DATA_ERROR_MAGIC  (-5)
#define BZ_IO_ERROR          (-6)
#define BZ_UNEXPECTED_EOF    (-7)
#define BZ_OUTBUFF_FULL      (-8)
#define BZ_CONFIG_ERROR      (-9)

#ifndef uint32
#define uint32 unsigned int
#endif

#ifndef int32
#define int32 int
#endif

class gcException : public std::exception
{
public:
	gcException()
	{
		m_uiErrId = ERR_UNKNOWNERROR;
		m_uiSecErrId = 0;
		
		assign(errMsg[ERR_UNKNOWNERROR]);
	}

	gcException(gcException const &e)
	{
		m_uiErrId = e.getErrId();
		m_uiSecErrId = e.getSecErrId();
		
		assign(e.getErrMsg());
	}

	gcException(ERROR_ID errId, int32 secErrId, const char* message = NULL)
	{
		setErrId(errId, secErrId);

		if (message)
			assign(message);
	}

	gcException(ERROR_ID errId, int32 secErrId, const std::string& message)
	{
		setErrId(errId, secErrId);

		if (message.size() > 0)
			assign(message.c_str());
	}

	gcException(ERROR_ID errId, const std::string& message)
	{
		setErrId(errId, 0);

		if (message.size() > 0)
			assign(message.c_str());
	}

	gcException(ERROR_ID errId, const char* message = NULL)
	{
		setErrId(errId, 0);

		if (message)
			assign(message);
	}

	gcException(gcException *gcE)
	{
		if (gcE)
		{
			m_uiErrId = gcE->getErrId();
			m_uiSecErrId = gcE->getSecErrId();
			assign(gcE->getErrMsg());
		}
		else
		{
			m_uiErrId = ERR_UNKNOWNERROR;
			assign(errMsg[ERR_UNKNOWNERROR]);
		}
	}

	explicit gcException(uint64 vs)
	{
		gcException *gcE = (gcException*)vs;
		
		if (gcE)
		{
			m_uiErrId = gcE->getErrId();
			m_uiSecErrId = gcE->getSecErrId();
			assign(gcE->getErrMsg());
		}
		else
		{
			m_uiErrId = ERR_UNKNOWNERROR;
			assign(errMsg[ERR_UNKNOWNERROR]);
		}
	}

	gcException(const gcException *gcE)
	{
		

		if (gcE)
		{
			gcException *temp = const_cast<gcException*>(gcE);
			m_uiErrId = temp->getErrId();
			m_uiSecErrId = temp->getSecErrId();
			assign(temp->getErrMsg());
		}
		else
		{
			m_uiErrId = ERR_UNKNOWNERROR;
			assign(errMsg[ERR_UNKNOWNERROR]);
		}
	}

	~gcException() throw()
	{

	}

	void setErrId(ERROR_ID errId, int secErrId = 0)
	{
		m_uiErrId = errId;
		m_uiSecErrId = secErrId;
		assign(errMsg[errId]);
	}

	gcException& operator =(gcException const &e)
	{
		if (this != &e) // protect against invalid self-assignment
		{
			m_uiErrId = e.getErrId();
			m_uiSecErrId = e.getSecErrId();
			assign(e.getErrMsg());
		}
		// by convention, always return *this
		return *this;
	}

	const char* getErrMsg() const
	{
		return m_szErrMsg.c_str();
	}

	uint32 getErrId() const
	{
		return m_uiErrId;
	}

	int32 getSecErrId() const
	{
		return m_uiSecErrId;
	}

	bool isCurlError() const
	{
		return (m_uiErrId == ERR_LIBCURL);
	}

	virtual const char* what() const throw()
	{
		return getErrMsg();
	}

	void assign(const char* message)
	{
		if (message)
			m_szErrMsg = message;
		else
			m_szErrMsg = "";
	}

private:
	int32 m_uiSecErrId;
	uint32 m_uiErrId;
	std::string m_szErrMsg;
};


#endif //DESURA_GCEXCEPTION_H
