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
#include <time.h>
#include "XMLMacros.h"

#define DUMP_UPLOAD_URL "http://www.desura.com/api/crashupload"
#define DUMP_UPLOAD_AGENT "Desura CrashDump Reporter"

class Color;
void LogMsg(int type, std::string msg, Color *col){}
void LogMsg(int type, std::wstring msg, Color *col){}

class Logger
{
public:
	Logger()
	{
#ifdef WIN32
		UTIL::FS::Path path(gcString(UTIL::OS::getAppDataPath(L"\\dumps\\")), "crash_log.txt", false);
		UTIL::FS::recMakeFolder(path);
#else
		UTIL::FS::Path path(gcString(UTIL::OS::getAppDataPath()), "crash_log.txt", false);
		UTIL::FS::recMakeFolder(path);
#endif

		printf("Saving crash report to: %s\n", path.getFullPath().c_str());

		try
		{
			fh.open(path.getFullPath().c_str(), UTIL::FS::FILE_APPEND);
		}
		catch (...)
		{
		}
	}

	void write(const char* format, ...)
	{
		if (fh.isValidFile() == false)
			return;

		size_t size = 512;
		char* temp = NULL;

		va_list arglist;
		va_start( arglist, format );

		int res = 0;
		do
		{
			size *= 2;
			
			if (temp)
				delete [] temp;

			temp = new char[size];
			
#ifdef WIN32
			res = vsnprintf_s(temp, size, _TRUNCATE, format, arglist);
#else
			res = vsnprintf(temp, size, format, arglist);
#endif
		}
#ifdef WIN32
		while (res == -1);
#else
		while ((size_t)res > size);
#endif
		
		va_end( arglist );

		try
		{
			fh.write(temp, res);
		}
		catch (...)
		{
		}
	}

private:
	UTIL::FS::FileHandle fh;
};

bool CompressFile(gcString &filePath);
bool PrepDumpForUpload(gcString &dumpFile);
bool UploadDump(const char* file, const char* user, int build, int branch, DelegateI<Prog_s>* progress);


extern "C"
{
	CEXPORT bool UploadCrash(const char* file, const char* user, int build, int branch)
	{
		return UploadDump(file, user, build, branch, NULL);
	}

	CEXPORT bool UploadCrashProg(const char* file, const char* user, int build, int branch, DelegateI<Prog_s>* progress)
	{
		return UploadDump(file, user, build, branch, progress); 
	}
}

bool CompressFile(gcString &filePath)
{
	uint64 fileSize = UTIL::FS::getFileSize(UTIL::FS::Path(filePath, "", true));

	if (fileSize == 0)
		return false;

	gcString destPath(filePath);
	destPath += ".bz2";

	try
	{
		UTIL::FS::FileHandle fhRead(filePath.c_str(), UTIL::FS::FILE_READ);
		UTIL::FS::FileHandle fhWrite(destPath.c_str(), UTIL::FS::FILE_WRITE);

		if (fhRead.isValidFile() == false)
			return false;

		if (fhWrite.isValidFile() == false)
			return false;

		UTIL::MISC::BZ2Worker worker(UTIL::MISC::BZ2_COMPRESS);

		char buff[10*1024];

		const size_t buffsize = 10*1024;
		uint32 leftToDo = (uint32)fileSize;

		bool end = false;

		do
		{
			size_t curSize = buffsize;

			if (buffsize > leftToDo)
			{
				end = true;
				curSize = leftToDo;
			}

			fhRead.read(buff, curSize);
			leftToDo -= curSize;
			worker.write(buff, curSize, end);

			worker.doWork();
			size_t b = 0;

			do
			{
				b = buffsize;
				worker.read(buff, b);
				fhWrite.write(buff, b);
			}
			while (b > 0);
		}
		while (!end);
	}
	catch (gcException)
	{
		return false;
	}

	UTIL::FS::delFile(UTIL::FS::Path(filePath, "", true));
	filePath = destPath;

	return true;
}


bool PrepDumpForUpload(gcString &dumpFile)
{
	UTIL::FS::Path path(dumpFile, "", true);

	if (!UTIL::FS::isValidFile(path))
		return false;

	if (Safe::stricmp(path.getFile().getFileExt().c_str(),"dmp") == 0)
		CompressFile(dumpFile);

	return true;
}

bool UploadDump(const char* file, const char* user, int build, int branch, DelegateI<Prog_s>* progress)
{
	Logger log;
	log.write("---------------------------------------\r\n");

	time_t ltime; /* calendar time */
	ltime=time(NULL); /* get current cal time */

	
#ifdef WIN32
	char buff[255] = {0};

	struct tm t;
	localtime_s(&t, &ltime);
	asctime_s(buff, 255, &t);
#else
	struct tm *t = localtime(&ltime);
	char* buff = asctime(t);
#endif

	log.write("%s\r\n", buff);
	log.write("---------------------------------------\r\n");

	log.write("Uploaded crash dump: [%s]\r\n", file);


	gcString dump(file);

	if (PrepDumpForUpload(dump) == false)
	{
		log.write("Failed to prepare crash dump.\r\n");
		return false;
	}
	else
	{
		log.write("Prepared crash dump to: [%s]\r\n", dump.c_str());
	}

	std::string os = UTIL::OS::getOSString();

	HttpHandle hh(DUMP_UPLOAD_URL);

	if (progress)
		hh->getProgressEvent() += progress;

	hh->setUserAgent(DUMP_UPLOAD_AGENT);

	hh->cleanUp();
	hh->addPostText("os", os.c_str());
	hh->addPostText("build", build);
	hh->addPostText("appid", branch);

	if (user)
		hh->addPostText("user", user);

	hh->addPostFile("crashfile", dump.c_str());

	try
	{
		hh->postWeb();
	}
	catch (gcException &except)
	{
		log.write("Failed to upload crash: %s [%d.%d].\r\n", except.getErrMsg(), except.getErrId(), except.getSecErrId());
		return false;
	}

	TiXmlDocument doc;
	XML::loadBuffer(doc, const_cast<char*>(hh->getData()), hh->getDataSize());
	
	try
	{
		XML::processStatus(doc, "crashupload");
		log.write("Uploaded dump\r\n");
		UTIL::FS::delFile(UTIL::FS::Path(dump, "", true));		
	}
	catch (gcException &)
	{
		log.write("Bad status returned from upload crash dump.\r\n");

		gcString res;
		res.assign(hh->getData(), hh->getDataSize());

		log.write("Result: \r\n\r\n%s\r\n\r\n", res.c_str());
		
		return false;	
	}

	return true;
}

void PrintfMsg(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}
