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


#include "Common.h"
#include "util/UtilWeb.h"

#define CURL_STATICLIB
#include "curl/curl.h"
#include "curl/easy.h"


#ifdef WIN32
#include "Winhttp.h"
#endif

class MemoryStruct
{
public:
	MemoryStruct()
	{
		memory = NULL;
		size = 0;
		obj=NULL;
	}

	~MemoryStruct()
	{
		if (memory)
			free (memory);
	}

	void* obj;
	char *memory;
	uint32 size;
};

typedef struct curl_slist curl_slist_s;

HttpHandleI::~HttpHandleI()
{

}

class PostWrapperI
{
public:
	virtual ~PostWrapperI()
	{
	}

	virtual void addToPost(curl_httppost *&first, curl_httppost* &last)=0;
};

class HttpHInternal : public HttpHandleI
{
public:
	HttpHInternal(const char* url, bool useSsl = false);
	HttpHInternal(bool useSsl = false);
	~HttpHInternal();

	virtual uint8 getFtp();
	virtual uint8 getWeb();
	virtual uint8 postWeb();
	virtual uint8 getWebToFile();

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Adders
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual void addPostText(const char* key, const char* value);
	virtual void addPostText(const char* key, uint32 value);
	virtual void addRawPost(const char* postdata, size_t dataSize);
	virtual void addPostFileAsBuff(const char* key, const char* fileName, const char* buffer, uint32 size);
	virtual void addPostFile(const char* key, const char* filePath);
	virtual void addCookie(const char* key, const char* value);
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Setters
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual void setDownloadRange(uint64 offset, uint32 size);
	virtual void setUrl(const char* url);
	virtual void setCertFile(const char* path);
	virtual void setUp();
	virtual void setUpProxy();
	virtual void setUserAgent(const char* useragent);
	virtual void setOutFile(const char* file);
	virtual void setUserPass(const char* user, const char* pass);
	

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Getters
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual const char* getData();
	virtual uint32 getDataSize();

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Misc
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual void cleanUp(bool delUrl = false);
	virtual void clearCookies();
	virtual void abortTransfer();
	virtual void cleanPostInfo();


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Events
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Event<Prog_s> onProgressEvent;
	Event<WriteMem_s> onWriteMemoryEvent;

	Event<Prog_s>& getProgressEvent(){return onProgressEvent;}
	Event<WriteMem_s>& getWriteEvent(){return onWriteMemoryEvent;}

	void del(){delete this;}

protected:
	void addHeader(const char* header);

	static size_t write_cbs(void* clientp, size_t size, size_t nmemb, void *data);
	size_t write_cb(size_t size, size_t nmemb, void *data);

	static int progress_cbs(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
	int progress_cb(double dltotal, double dlnow, double ultotal, double ulnow);

	CURL* getHandle(){return m_pCurlHandle;}

	curl_slist_s* setUpHeaders();

	void init(bool useSsl);

	void lock(){m_bLock = true;}
	void unlock(){m_bLock = false;}

	uint8 processResult(CURLcode res);

	void setUp(bool setRange);

private:
	bool m_bLock;
	bool m_bAbort;
	bool m_bWritingToFile;

	FILE* m_hFile;
	CURL* m_pCurlHandle;
	MemoryStruct *m_pMemStruct;

	std::vector<std::string> m_vHeaders;
	std::vector<PostWrapperI*> m_vFormPost;

	std::string m_szRawPost;
	gcString m_szUrl;
	gcString m_szCookies;
	gcString m_szUserAgent;
	gcString m_szFile;
	gcString m_szCertFile;
	gcString m_szUser;
	gcString m_szPass;

	uint64 m_uiOffset;
	uint64 m_uiSize;

	char m_szErrBuff[CURL_ERROR_SIZE];
};


class PostAsText : public PostWrapperI
{
public:
	PostAsText(const char* key, const char* text)
	{
		if (text)
			m_szText = text;

		if (key)
			m_szKey = key;
	}

	virtual void addToPost(curl_httppost *&first, curl_httppost* &last)
	{
		curl_formadd(&first, &last, CURLFORM_COPYNAME, m_szKey.c_str(), CURLFORM_COPYCONTENTS, m_szText.c_str(), CURLFORM_END);
	}

	std::string m_szText;
	std::string m_szKey;
};

class PostAsFile : public PostAsText
{
public:
	PostAsFile(const char* key, const char* file) : PostAsText(key, file)
	{
	}

	virtual void addToPost(curl_httppost *&first, curl_httppost* &last)
	{
		curl_formadd(&first, &last, CURLFORM_COPYNAME, m_szKey.c_str(), CURLFORM_FILE, m_szText.c_str(), CURLFORM_END);
	}
};

class PostAsFileBuff : public PostAsText
{
public:
	PostAsFileBuff(const char* key, const char* file, const char* buffer, uint32 size) : PostAsText(key, file)
	{
		m_szBuffer = buffer;
		m_uiSize = size;
	}

	virtual void addToPost(curl_httppost *&first, curl_httppost* &last)
	{
		curl_formadd(&first, &last, CURLFORM_COPYNAME, m_szKey.c_str(), CURLFORM_BUFFER, m_szText.c_str(), CURLFORM_BUFFERPTR, m_szBuffer, CURLFORM_BUFFERLENGTH, (long)m_uiSize, CURLFORM_END);
	}

	const char* m_szBuffer;
	uint32 m_uiSize;
};

class curlGlobalInit
{
public: 
	curlGlobalInit()
	{
		curl_global_init(CURL_GLOBAL_ALL);
	}

	~curlGlobalInit()
	{
		curl_global_cleanup();
	}
};


HttpHandleI* newHttpHandle(const char* url, bool usessl)
{
	if (url)
		return new HttpHInternal(url, usessl);
	
	return new HttpHInternal(usessl);
}


static void *myrealloc(void *ptr, size_t size)
{
  /* There might be a realloc() out there that doesn't like reallocing

	 NULL pointers, so we take care of it here */
  if(ptr)
	return realloc(ptr, size);
  else
	return malloc(size);
}


curlGlobalInit cgi;


HttpHInternal::HttpHInternal(const char* url, bool useSsl)
{
	init(useSsl);
	setUrl(url);
}

HttpHInternal::HttpHInternal(bool useSsl)
{
	init(useSsl);
}

HttpHInternal::~HttpHInternal()
{
	m_bAbort = true;

	//if we are locked, wait till we are done before cleaning up
	while (m_bLock)
		gcSleep(500);

	cleanUp(true);

	curl_easy_cleanup(m_pCurlHandle);
	safe_delete(m_pMemStruct);
}

void HttpHInternal::init(bool useSsl)
{
	m_pCurlHandle = curl_easy_init();
	m_pMemStruct = new MemoryStruct;

	m_bLock = false;
	m_bAbort = false;
	m_bWritingToFile = false;

	m_uiOffset = 0;
	m_uiSize = 0;

	m_hFile = NULL;
	addHeader("Expect: ");
}

void HttpHInternal::abortTransfer()
{
	m_bAbort = true;
}

size_t HttpHInternal::write_cbs(void *data, size_t size, size_t nmemb, void *ptr)
{
	return static_cast<HttpHInternal*>(ptr)->write_cb(size, nmemb, data);
}

size_t HttpHInternal::write_cb(size_t size, size_t nmemb, void *data)
{
	if (m_bAbort)
		return 0;

	size_t realsize = size * nmemb;
	
	m_pMemStruct->size += (uint32)realsize;

	WriteMem_s wms;
	wms.size = realsize;
	wms.data = data;
	wms.handled = false;
	wms.stop = false;
	wms.wrote = 0;

	onWriteMemoryEvent(wms);

	if (wms.stop)
		m_bAbort = true;

	if (wms.handled)
		return wms.wrote;


	if (m_bWritingToFile)
	{
		if (m_hFile)
			fwrite(data, realsize, 1, m_hFile);
		else
			return 0;
	}
	else
	{
		m_pMemStruct->memory = (char *)myrealloc(m_pMemStruct->memory, m_pMemStruct->size + 1);
		if (m_pMemStruct->memory) 
		{
			memcpy(&(m_pMemStruct->memory[m_pMemStruct->size-realsize]), data, realsize);
			m_pMemStruct->memory[m_pMemStruct->size] = 0;
		}
	}

	return realsize;
}

void HttpHInternal::setUp()
{
	setUp(true);
}

void HttpHInternal::setUp(bool setRange)
{
	if(!m_pCurlHandle)
		throw gcException(ERR_BADCURLHANDLE);

	if (m_szUrl == "")
		throw gcException(ERR_BADURL);

	curl_easy_setopt(m_pCurlHandle, CURLOPT_URL, m_szUrl.c_str());
	curl_easy_setopt(m_pCurlHandle, CURLOPT_WRITEFUNCTION, HttpHInternal::write_cbs);
	curl_easy_setopt(m_pCurlHandle, CURLOPT_WRITEDATA, (void *)this);

	if (m_szUserAgent != "")
		curl_easy_setopt(m_pCurlHandle, CURLOPT_USERAGENT, m_szUserAgent.c_str());
	else
		curl_easy_setopt(m_pCurlHandle, CURLOPT_USERAGENT, USERAGENT);

	curl_easy_setopt(m_pCurlHandle, CURLOPT_NOPROGRESS, false);
	curl_easy_setopt(m_pCurlHandle, CURLOPT_PROGRESSFUNCTION, HttpHInternal::progress_cbs);
	curl_easy_setopt(m_pCurlHandle, CURLOPT_PROGRESSDATA, this);

	if (m_szCookies != "")
		curl_easy_setopt(m_pCurlHandle, CURLOPT_COOKIE, m_szCookies.c_str());

	if (m_szCertFile != "")
		curl_easy_setopt(m_pCurlHandle, CURLOPT_CAINFO, m_szCertFile.c_str());

	memset(m_szErrBuff, 0, CURL_ERROR_SIZE);
	curl_easy_setopt(m_pCurlHandle, CURLOPT_ERRORBUFFER , m_szErrBuff );

	if (setRange && m_uiSize != 0)
		addHeader(gcString("Range: bytes={0}-{1}", m_uiOffset, m_uiOffset+m_uiSize-1).c_str());

	setUpProxy();
}

bool g_bInitProxy = false;
bool g_bNeedProxy = false;

void HttpHInternal::setUpProxy()
{
#ifdef WIN32

	if (g_bInitProxy && !g_bNeedProxy)
		return;

	WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ieProxyConfig;
	WINHTTP_AUTOPROXY_OPTIONS autoProxyOptions;
	WINHTTP_PROXY_INFO autoProxyInfo;

	memset(&ieProxyConfig, 0, sizeof(WINHTTP_CURRENT_USER_IE_PROXY_CONFIG));
	memset(&autoProxyOptions, 0, sizeof(WINHTTP_AUTOPROXY_OPTIONS));
	memset(&autoProxyInfo, 0, sizeof(WINHTTP_PROXY_INFO));

	BOOL res = WinHttpGetIEProxyConfigForCurrentUser(&ieProxyConfig);

	//if we got the proxy settings and we dont need it return
	if (res && ieProxyConfig.fAutoDetect == false && ieProxyConfig.lpszAutoConfigUrl == NULL && ieProxyConfig.lpszProxy == NULL)
		return;

	if (ieProxyConfig.lpszAutoConfigUrl != NULL && ieProxyConfig.lpszProxy != NULL)
	{
		autoProxyOptions.dwFlags |= WINHTTP_AUTOPROXY_CONFIG_URL;
		autoProxyOptions.lpszAutoConfigUrl = ieProxyConfig.lpszAutoConfigUrl;
	
		// basic flags you almost always want
		autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT;
		autoProxyOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP|WINHTTP_AUTO_DETECT_TYPE_DNS_A;
		autoProxyOptions.fAutoLogonIfChallenged = TRUE;

		// here we reset fAutoProxy in case an auto-proxy isn't actually
		// configured for this url

		HINTERNET hiOpen = WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

		if (!hiOpen)
			return;

		gcWString url(m_szUrl);
		res = WinHttpGetProxyForUrl(hiOpen, url.c_str(), &autoProxyOptions, &autoProxyInfo);
		WinHttpCloseHandle(hiOpen);
	}

	gcString proxy;
	gcString noProxy;
	gcString noProxyRaw;

	if (autoProxyInfo.lpszProxy != NULL)
	{
		// set proxy options for libcurl based on autoProxyInfo
		g_bNeedProxy = true;
		proxy = gcString(autoProxyInfo.lpszProxy);
		noProxyRaw = gcString(autoProxyInfo.lpszProxyBypass);
	}
	else if (ieProxyConfig.lpszProxy != NULL)
	{
		// based on ieProxyConfig
		g_bNeedProxy = true;
		proxy = gcString(ieProxyConfig.lpszProxy);
		noProxyRaw = gcString(ieProxyConfig.lpszProxyBypass);
	}
	else
	{
		g_bInitProxy = true;
		return;
	}

	g_bInitProxy = true;

	std::vector<std::string> roundOne;
	std::vector<std::string> roundTwo;

	UTIL::STRING::tokenize(noProxyRaw, roundOne, " ");

	for (size_t x=0; x<roundOne.size(); x++)
		UTIL::STRING::tokenize(roundOne[x], roundTwo, ";");

	for (size_t x=0; x<roundTwo.size(); x++)
	{
		if (roundTwo[x].size() == 0)
			continue;

		if (noProxy.size() > 0)
			noProxy += ",";

		noProxy += roundTwo[x];
	}

	curl_easy_setopt(m_pCurlHandle, CURLOPT_PROXY, proxy.c_str());
	curl_easy_setopt(m_pCurlHandle, CURLOPT_NOPROXY, noProxy.c_str());
#endif
}

curl_slist_s* HttpHInternal::setUpHeaders()
{
	if (m_vHeaders.size() == 0)
		return NULL;

	curl_slist_s *headers = NULL;

	for (size_t x=0; x<m_vHeaders.size(); x++)
		headers = curl_slist_append(headers, m_vHeaders[x].c_str());

	curl_easy_setopt(m_pCurlHandle, CURLOPT_HTTPHEADER, headers);
	return headers;
}

uint8 HttpHInternal::processResult(CURLcode res)
{
	if (res != CURLE_OK && res != CURLE_ABORTED_BY_CALLBACK)
		throw gcException(ERR_LIBCURL, res, gcString("{0} ({1}) [{2}]", curl_easy_strerror(res), m_szUrl, m_szErrBuff));

	if (res == CURLE_ABORTED_BY_CALLBACK)
		return UWEB_USER_ABORT;

	return UWEB_OK;
}

uint8 HttpHInternal::getWeb()
{
	m_bWritingToFile = false;

	if (m_bLock)
		throw gcException(ERR_WEBLOCKED);

	if (!m_pCurlHandle)
		throw gcException(ERR_BADCURLHANDLE);

	if (m_szUrl == "")
		throw gcException(ERR_BADURL);

	lock();
	setUp();

	curl_slist_s* headers = setUpHeaders();
	CURLcode res = curl_easy_perform(m_pCurlHandle);
	curl_slist_free_all(headers);

	unlock();

	return processResult(res);
}

uint8 HttpHInternal::getWebToFile()
{
	m_bWritingToFile = true;

	if (m_bLock)
		throw gcException(ERR_WEBLOCKED);

	if(!m_pCurlHandle)
		throw gcException(ERR_BADCURLHANDLE);

	if (m_szUrl == "")
		throw gcException(ERR_BADURL);

	if (m_szFile == "")
		throw gcException(ERR_NOFILE);

	m_hFile = Safe::fopen(m_szFile.c_str(), "wb");

	if (!m_hFile)
		throw gcException(ERR_NULLHANDLE);

	lock();
	setUp();

	curl_slist_s* headers = setUpHeaders();
	CURLcode res = curl_easy_perform(m_pCurlHandle);
	curl_slist_free_all(headers);
	unlock();

	m_bWritingToFile = false;
	fclose(m_hFile);

	return processResult(res);
}

uint8 HttpHInternal::postWeb()
{
	m_bWritingToFile = false;

	if (m_bLock)
		throw gcException(ERR_WEBLOCKED);

	if (!m_pCurlHandle)
		throw gcException(ERR_BADCURLHANDLE);

	if (m_szUrl == "")
		throw gcException(ERR_BADURL);

	lock();
	setUp();

	curl_httppost *formPost = NULL; 
	curl_httppost *formLast = NULL;

	if (m_vFormPost.size() > 0)
	{
		for (size_t x=0; x<m_vFormPost.size(); x++)
			m_vFormPost[x]->addToPost(formPost, formLast);

		curl_easy_setopt(m_pCurlHandle, CURLOPT_HTTPPOST, formPost);
	}
	else if (m_szRawPost != "")
	{
		curl_easy_setopt(m_pCurlHandle, CURLOPT_POST, 1);
		curl_easy_setopt(m_pCurlHandle, CURLOPT_POSTFIELDS, m_szRawPost.c_str());
		curl_easy_setopt(m_pCurlHandle, CURLOPT_POSTFIELDSIZE, m_szRawPost.size());
	}

	curl_slist_s* headers = setUpHeaders();

	CURLcode res = curl_easy_perform(m_pCurlHandle);

	curl_formfree(formPost);
	curl_slist_free_all(headers);

	unlock();

	return processResult(res);
}

uint8 HttpHInternal::getFtp()
{
	m_bWritingToFile = false;

	if (m_bLock)
		throw gcException(ERR_WEBLOCKED);

	if (!m_pCurlHandle)
		throw gcException(ERR_BADCURLHANDLE);

	if (m_szUrl == "")
		throw gcException(ERR_BADURL);

	lock();
	setUp(false);

	if (m_szUser != "")
		curl_easy_setopt(m_pCurlHandle, CURLOPT_USERNAME, m_szUser.c_str());

	if (m_szPass != "")
		curl_easy_setopt(m_pCurlHandle, CURLOPT_PASSWORD, m_szPass.c_str());

	curl_slist_s* commands = NULL;
	commands = curl_slist_append(commands, gcString("REST {0}", m_uiOffset).c_str());
	commands = curl_slist_append(commands, gcString("XCHK {0}", m_uiSize).c_str());
	curl_easy_setopt(m_pCurlHandle, CURLOPT_QUOTE, commands);

	CURLcode res = curl_easy_perform(m_pCurlHandle);

	curl_slist_free_all(commands);

	unlock();

	return processResult(res);
}

int HttpHInternal::progress_cbs(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	return static_cast<HttpHInternal*>(clientp)->progress_cb(dltotal, dlnow, ultotal, ulnow);
}

int HttpHInternal::progress_cb(double dltotal, double dlnow, double ultotal, double ulnow)
{
	Prog_s temp;
	temp.dltotal = dltotal;
	temp.dlnow = dlnow;
	temp.ultotal = ultotal;
	temp.ulnow = ulnow;
	temp.abort = m_bAbort;

	onProgressEvent(temp);

	m_bAbort = temp.abort;

	return m_bAbort;
}

void HttpHInternal::addRawPost(const char* postdata, size_t dataSize)
{
	if (!postdata)
		return;

	m_szRawPost = std::string(postdata, dataSize);
}

void HttpHInternal::addPostText(const char* key, uint32 value)
{
	if (!key)
		return;

	addPostText(key, gcString("{0}", value).c_str());
}

void HttpHInternal::addPostText(const char* key, const char* value)
{
	if (!key || !value)
		return;

	m_vFormPost.push_back(new PostAsText(key, value));
}

void HttpHInternal::addPostFileAsBuff(const char* key, const char* fileName, const char* buffer, uint32 size)
{
	if (!key || !fileName || !buffer || size == 0)
		return;

	m_vFormPost.push_back(new PostAsFileBuff(key, fileName, buffer, size));
}

void HttpHInternal::addPostFile(const char* key, const char* filePath)
{
	if (!key || !filePath)
		return;

	m_vFormPost.push_back(new PostAsFile(key, filePath));
}

void HttpHInternal::cleanPostInfo()
{
	safe_delete(m_vFormPost);
}

void HttpHInternal::setDownloadRange(uint64 offset, uint32 size)
{
	m_uiOffset = offset;
	m_uiSize = size;
}

void HttpHInternal::setOutFile(const char* file)
{
	m_szFile = file;
}

void HttpHInternal::setCertFile(const char* file)
{
	m_szCertFile = file;
}

//gets a pointer to the data. Do not delete!
const char* HttpHInternal::getData()
{
	if (m_pMemStruct)
		return m_pMemStruct->memory;

	return NULL;
}


//gets the size of data which has been downloaded.
uint32 HttpHInternal::getDataSize()
{
	if (m_pMemStruct)
		return m_pMemStruct->size;

	return 0;
}

void HttpHInternal::setUserAgent(const char* useragent)
{
	m_szUserAgent = useragent;
}


void HttpHInternal::setUrl(const char* url)
{
	m_szUrl = url;
}

//use this to clean up the data before use again.
void HttpHInternal::cleanUp(bool delUrl)
{
	m_vHeaders.clear();
	addHeader("Expect: ");

	if (m_pMemStruct && m_pMemStruct->memory)
	{
		free (m_pMemStruct->memory);
		m_pMemStruct->memory = NULL;
	}

	m_pMemStruct->size = 0;

	if (delUrl)
		m_szUrl = "";

	m_szFile = "";
	m_bAbort = false;

	m_uiOffset = 0;
	m_uiSize = 0;

	cleanPostInfo();
}

void HttpHInternal::addHeader(const char* header)
{
	if (!header)
		return;

	m_vHeaders.push_back(header);
}

void HttpHInternal::clearCookies()
{
	m_szCookies = "";
}

void HttpHInternal::addCookie(const char* key, const char* value)
{
	if (!key || !value)
		return;

	m_szCookies += gcString("{0}={1};", key, value);
}

void HttpHInternal::setUserPass(const char* user, const char* pass)
{
	m_szUser = user;
	m_szPass = pass;
}
