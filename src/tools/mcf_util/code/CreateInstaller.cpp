///////////// Copyright 2010 DesuraNet. All rights reserved. /////////////
//
//   Project     : mcf_util
//   File        : CreateInstaller.cpp
//   Description :
//      [TODO: Write the purpose of CreateInstaller.cpp.]
//
//   Created On: 4/5/2011 4:21:42 PM
//   Created By:  <mailto:>
////////////////////////////////////////////////////////////////////////////

#include "Common.h"
#include "UtilFunction.h"



typedef struct
{
	int32 size;
	int32 crc;
	char exe[64];
} ExePackerHeader;


#ifdef WIN32

#pragma pack(push, 4)

typedef struct
{
	uint64 size;
	uint64 offset;
	uint32 crc;
} McfInfo;

typedef struct
{
	McfInfo installMcf;
	McfInfo contentMcf;
} InstallerPackerHeader;


#pragma pack(pop)


uint32 GetExeSize(const char* filename);

class Installer : public UtilFunction
{
public:
	virtual uint32 getNumArgs()
	{
		return 3;
	}

	virtual const char* getArgDesc(size_t index)
	{
		if (index == 0)
			return "Src Folder";
		else if (index == 1)
			return "Dest Exe";

		return "Exe to run";
	}

	virtual const char* getFullArg()
	{
		return "installer";
	}

	virtual const char getShortArg()
	{
		return 'i';
	}

	virtual const char* getDescription()
	{
		return "Creates a installer from a exe";
	}

	void makeMCF(const char* path)
	{
		MCFCore::MCFI* mcfHandle = mcfFactory();
		mcfHandle->getProgEvent() += delegate((UtilFunction*)this, &UtilFunction::printProgress);
		mcfHandle->getErrorEvent() += delegate((UtilFunction*)this, &UtilFunction::mcfError);

		mcfHandle->setFile("temp.mcf");
		mcfHandle->parseFolder(path);
		mcfHandle->saveMCF();

		mcfDelFactory(mcfHandle);
	}

	virtual int performAction(std::vector<std::string> &args)
	{
		UTIL::FS::delFile("temp.mcf");
		UTIL::FS::delFile(args[1].c_str());

		makeMCF(args[0].c_str());

		uint32 exeHeadSize = GetExeSize("exe_packer.exe");
		uint32 exeSize = UTIL::FS::getFileSize("exe_packer.exe");

		uint32 diff = exeHeadSize - exeSize;


		uint32 size = UTIL::FS::getFileSize("temp.mcf");

		ExePackerHeader ep;
		strcpy(ep.exe, args[2].c_str());
		ep.crc = 0;
		ep.size = size;

		UTIL::FS::FileHandle fh(args[1].c_str(), UTIL::FS::FILE_APPEND);
		UTIL::FS::FileHandle fhExe("exe_packer.exe", UTIL::FS::FILE_READ);

		char buff[10*1024];

		while (exeSize > 0)
		{
			uint32 read = 10*1024;

			if (read > exeSize)
				read = exeSize;

			fhExe.read(buff, read);
			fh.write(buff, read);

			exeSize -= read;
		}

		fhExe.close();

		printf("Diff is %d\n", diff);
		while (diff > 0)
		{
			uint32 read = 10*1024;

			if (read > diff)
				read = diff;

			fh.write(buff, read);

			diff -= read;
		}

		UTIL::FS::FileHandle mcf("temp.mcf", UTIL::FS::FILE_READ);
		fh.write((char*)(&ep), sizeof(ExePackerHeader));

	
		while (size > 0)
		{
			uint32 read = 10*1024;

			if (read > size)
				read = size;

			mcf.read(buff, read);
			fh.write(buff, read);

			size -= read;
		}

		fh.close();
		mcf.close();

		return 0;
	}
};

REG_FUNCTION(Installer)


class InstallerBootloader : public UtilFunction
{
public:
	virtual uint32 getNumArgs()
	{
		return 3;
	}

	virtual const char* getArgDesc(size_t index)
	{
		if (index == 0)
			return "Installer mcf";
		else if (index == 1)
			return "Content mcf";

		return "dest exe";
	}

	virtual const char* getFullArg()
	{
		return "standalone";
	}

	virtual const char getShortArg()
	{
		return 'q';
	}

	virtual const char* getDescription()
	{
		return "Packages a stand alone installer";
	}


	virtual int performAction(std::vector<std::string> &args)
	{
		uint32 exeHeadSize = GetExeSize(args[2].c_str());
		uint32 exeSize = UTIL::FS::getFileSize(args[2].c_str());

		InstallerPackerHeader iph;
		memset(&iph, 0, sizeof(InstallerPackerHeader));

		iph.installMcf.offset = exeHeadSize + sizeof(InstallerPackerHeader);
		iph.installMcf.size = UTIL::FS::getFileSize(args[0].c_str());
		iph.installMcf.crc = UTIL::FS::CRC32(args[0].c_str());
		
		iph.contentMcf.offset = iph.installMcf.size + iph.installMcf.offset;
		iph.contentMcf.size = UTIL::FS::getFileSize(args[1].c_str());
		iph.contentMcf.crc = UTIL::FS::CRC32(args[1].c_str());

		printf("Install crc: %u\n", iph.installMcf.crc);
		printf("Content crc: %u\n", iph.contentMcf.crc);

		UTIL::FS::FileHandle fh(args[2].c_str(), UTIL::FS::FILE_APPEND);
		fh.seek(exeHeadSize);

		fh.write((char*)&iph, sizeof(InstallerPackerHeader));

		char buff[10 * 1024];

		{
			UTIL::FS::FileHandle rfh(args[0].c_str(), UTIL::FS::FILE_READ);

			uint64 done = 0;
			uint32 buffSize = 10 * 1024;

			while (done < iph.installMcf.size)
			{
				if (buffSize > (iph.installMcf.size - done))
					buffSize = (uint32)(iph.installMcf.size-done);

				rfh.read(buff, buffSize);
				fh.write(buff, buffSize);

				done+=buffSize;
			}
		}

		{
			UTIL::FS::FileHandle rfh(args[1].c_str(), UTIL::FS::FILE_READ);

			uint64 done = 0;
			uint32 buffSize = 10 * 1024;

			while (done < iph.contentMcf.size)
			{
				if (buffSize > (iph.contentMcf.size - done))
					buffSize = (uint32)(iph.contentMcf.size-done);

				rfh.read(buff, buffSize);
				fh.write(buff, buffSize);

				done+=buffSize;
			}
		}

		return 0;
	}
};
REG_FUNCTION(InstallerBootloader)





class InstallerSeek : public UtilFunction
{
public:
	virtual uint32 getNumArgs()
	{
		return 1;
	}

	virtual const char* getArgDesc(size_t index)
	{
		return "Installer exe";
	}

	virtual const char* getFullArg()
	{
		return "seekheader";
	}

	virtual const char getShortArg()
	{
		return 'h';
	}

	virtual const char* getDescription()
	{
		return "Seek to the pos where the install header should start";
	}


	virtual int performAction(std::vector<std::string> &args)
	{
		uint32 exeHeadSize = GetExeSize(args[0].c_str());

		printf("Header Size: %u\n", exeHeadSize);

		UTIL::FS::FileHandle fh(args[0].c_str(), UTIL::FS::FILE_APPEND);
		fh.seek(exeHeadSize-1);

		char buff[1] = {0};
		fh.write(buff, 1);

		printf("Wrote to end...\n");

		return 0;
	}
};
REG_FUNCTION(InstallerSeek)



uint32 GetExeSize(const char* filename)
{
	HANDLE hFile;
	HANDLE hFileMapping;
	LPVOID lpFileBase;
	PIMAGE_DOS_HEADER dosHeader;
	
	hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
					
	if ( hFile == INVALID_HANDLE_VALUE )
	{   
		printf("Couldn't open file with CreateFile()\n");
		return 0; 
	}
	
	hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);

	if ( hFileMapping == 0 )
	{   
		CloseHandle(hFile);
		printf("Couldn't open file mapping with CreateFileMapping()\n");
		return 0; 
	}
	
	lpFileBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);

	if (lpFileBase == 0)
	{
		CloseHandle(hFileMapping);
		CloseHandle(hFile);
		printf("Couldn't map view of file with MapViewOfFile()\n");
		return 0;
	}

	printf("Dump of file %s\n\n", filename);
	
	dosHeader = (PIMAGE_DOS_HEADER)lpFileBase;

	if (dosHeader->e_magic == IMAGE_DOS_SIGNATURE)
	{
		PIMAGE_NT_HEADERS pe = (PIMAGE_NT_HEADERS)((uint32)dosHeader+(uint32)dosHeader->e_lfanew);
		uint32 size = pe->OptionalHeader.SizeOfUninitializedData + pe->OptionalHeader.SizeOfInitializedData + pe->OptionalHeader.SizeOfCode;

		size += 4096;

		UnmapViewOfFile(lpFileBase);
		CloseHandle(hFileMapping);
		CloseHandle(hFile);

		return size;
	}

	UnmapViewOfFile(lpFileBase);
	CloseHandle(hFileMapping);
	CloseHandle(hFile);

	return 0;
}

#endif

