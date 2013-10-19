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


#ifndef DESURA_UTIL_WEB_H
#define DESURA_UTIL_WEB_H
#ifdef _WIN32
#pragma once
#endif


typedef struct
{
	size_t size;
	size_t wrote;
	void *data;
	bool handled;
	bool stop;
} WriteMem_s;


typedef struct
{
	double dltotal;
	double dlnow;
	double ultotal;
	double ulnow;
	bool abort;
} Prog_s;

enum
{
	UWEB_OK = 0,
	UWEB_USER_ABORT,
};

class HttpHandleI
{
public:
	virtual ~HttpHandleI();

	virtual void del()=0;

	virtual uint8 getFtp()=0;
	virtual uint8 getWeb()=0;
	virtual uint8 postWeb()=0;
	virtual uint8 getWebToFile()=0;


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Adders
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//this adds a post field with a key
	virtual void addPostText(const char* key, const char* value)=0;
	virtual void addPostText(const char* key, uint32 value)=0;
	virtual void addRawPost(const char* postdata, size_t dataSize)=0;

	//this adds a post a file with a key from a buffer
	virtual void addPostFileAsBuff(const char* key, const char* fileName, const char* buffer, uint32 size)=0;

	//this adds a file to the post from a dir
	virtual void addPostFile(const char* key, const char* filePath)=0;

	//this adds a new cookie
	virtual void addCookie(const char* key, const char* value)=0;
	

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Setters
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//for downloading a file
	virtual void setDownloadRange(uint64 offset, uint32 size)=0;

	//set the url to get info from
	virtual void setUrl(const char* url)=0;

	//sets the cert path to use
	virtual void setCertFile(const char* path)=0;

	//Set Up Post info. This is usefull for multi downloads where you cant post. Dont call it if using GetWeb or PostWeb
	virtual void setUp()=0;

	//Sets the useragent used when performing the http request
	virtual void setUserAgent(const char* useragent)=0;

	//set the file to save the response into
	virtual void setOutFile(const char* file)=0;

	//! Sets user name and password for ftp transfers
	//!
	virtual void setUserPass(const char* user, const char* pass)=0;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Getters
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//gets a pointer to the data. Do not delete!
	virtual const char* getData()=0;

	//gets the size of data which has been downloaded.
	virtual uint32 getDataSize()=0;


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Misc
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//use this to clean up the data before use again. Doesnt remove cookies
	virtual void cleanUp(bool delUrl = false)=0;

	//this removes all cookies
	virtual void clearCookies()=0;

	//this aborts the transfer on the next progress update.
	virtual void abortTransfer()=0;

	//Clean all post info if reusing handle.
	virtual void cleanPostInfo()=0;


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Events
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	virtual Event<Prog_s>& getProgressEvent()=0;
	virtual Event<WriteMem_s>& getWriteEvent()=0;
};


HttpHandleI* newHttpHandle(const char* url = NULL, bool useSsl = false);

class HttpHandle
{
public:
	HttpHandle()
	{
		m_pWC = NULL;
	}

	HttpHandle(const char* url, bool useSsl = false)
	{
		m_pWC = newHttpHandle(url, useSsl);
	}

	HttpHandle(HttpHandle &handle)
	{
		m_pWC = handle.m_pWC;
		handle.m_pWC = NULL;
	}

	~HttpHandle()
	{
		if (m_pWC)
			m_pWC->del();
	}

	HttpHandleI* operator ->()
	{
		if (!m_pWC)
			m_pWC = newHttpHandle();

		return m_pWC;
	}

	HttpHandle& operator=(const HttpHandle &handle)
	{
		if (this == &handle)
			return *this;

		if (m_pWC)
			m_pWC->del();

		m_pWC = handle.m_pWC;
		handle.m_pWC = NULL;

		return *this;
	}

private:
	mutable HttpHandleI* m_pWC;
};



#endif
