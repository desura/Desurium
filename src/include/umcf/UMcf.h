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


#ifndef DESURA_UMCF_H
#define DESURA_UMCF_H

#include "umcf/UMcfFile.h"
#include "umcf/UMcfHeader.h"

#define TYPE_APP (0x8)

enum
{
	MCF_OK = 0,
	UMCF_OK = 0,
	MCF_ERR_NULLPATH,
	MCF_ERR_WRONGOS,
	MCF_ERR_INVALIDHANDLE,
	MCF_ERR_EMPTYFILE,
	MCF_ERR_BADHEADER,
	MCF_ERR_FAILEDSEEK,
	MCF_ERR_FAILEDREAD,
	MCF_ERR_FAILEDWRITE,
	MCF_ERR_PARTREAD,
	MCF_ERR_PARTWRITE,
	MCF_ERR_BZ2CFAIL,
	MCF_ERR_BZ2DFAIL,
	MCF_ERR_XML_NOPRIMENODE,
	MCF_ERR_THREADERROR,
	MCF_ERR_SAVE_NOFILES,
	MCF_ERR_SAVE_NOHEADER,
	MCF_ERR_SAVE_THREAD,
	MCF_ERR_SAVE_NOFILE,
	MCF_ERR_NOFILE,
	MCF_ERR_DUPSRC,
	MCF_ERR_WEBDL_FAILED,
	MCF_ERR_BUFF_TOSMALL,
	MCF_ERR_NULLSRCFILE,
	MCF_ERR_NULLDESTFILE,
	UMCF_ERR_FILEDOE,
	UMCF_ERR_XML_NOPRIMENODE,
};


class UMcf
{
public:
	UMcf();
	~UMcf();

	//sets the working umcf file
	void setFile(const wchar_t* file, uint64 offset = 0);

	//parsers that MCF file
	uint8 parseMCF();

	//this extracts all files into path.
	void install(const wchar_t* path);

	//checks local files and updates as necassary
	bool checkFiles();

	//checks the header and makes sure its a valid installer
	bool isValidInstaller();

	//this updates the appid and build in the registry
	void setRegValues();

	//checks reg values againts MCF version
	bool isUpdateNewer();

	//loads xml from a file
	void loadFromFile(const wchar_t* file);

	//saves the xml to the path
	void dumpXml(const wchar_t* path);

	void removeOldFiles(UMcf* oldMcf, const wchar_t* dir);

	//! Deletes all files in the mcf from the path
	//!
	void deleteAllFiles(const wchar_t* path);

	std::string getFileHash(const std::wstring& fileName, const std::wstring& path);

	void disableMoveOldFiles();
	void cancel();

	Event<uint32> onProgressEvent;
	Event<Prog_s> onDownloadProgressEvent;

protected:
	virtual uint32 progressUpdate(Prog_s *info, uint32 other=0);

	//parsers a xml buffer
	uint8 parseXml(char* buff, size_t buffLen);
	uint8 parseXml(TiXmlNode *fNode);
	
	//parsers the update xml
	void parseUpdateXml(TiXmlDocument &doc);

	void updateAllCB(Prog_s& p);

	void removeOldFiles(const wchar_t* installPath);
	void moveOldFiles(const wchar_t* installPath, const wchar_t* fileName);


protected:
	uint32 m_iAppId;
	uint32 m_iAppBuild;	
	uint64 m_uiOffset;
	
	std::wstring m_szFile;
	std::string m_szUrl;


	UMcfHeader* m_sHeader;
	std::vector<UMcfFile*> m_pFileList;

	friend class UMcfEx;

	uint64 m_uiTotProgress;
	uint64 m_uiCurProgress;

	void onFileProgress(ProgressCB& prog);

private:
	bool m_bShouldMoveOldFiles;
	volatile bool m_bCanceled;
};

#endif

