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

#ifndef DESURA_MCF_FILE_I_H
#define DESURA_MCF_FILE_I_H


#define MCF_FILE_FACTORY "MCF_FILE"

#define MCF_NAME_LEN 256
#define MCF_PATH_LEN 2048
#define MCF_FPATH_LEN MCF_NAME_LEN+MCF_PATH_LEN+MCF_PATH_LEN
#define MCF_CSUM_LEN 33
#define DEFAULT_FLAGS (FLAG_SAVE) //should always save unless proven otherwise 

namespace MCFCore
{

//! MCFFileI is the MCFFile interface
//! 
//! @see MCFFile
class MCFFileI
{
public:
	enum FILE_CMP
	{
		CMP_NOTEQUAL = 0,	//!< not equal
		CMP_SAME,			//!< same file same date
		CMP_NEWER,			//!< same file newer date
		CMP_OLDER,			//!< same file older date
	};

	enum FILE_FLAGS
	{
		FLAG_NONE		= 0,		//!< No flags 
		FLAG_SAVE		= 1<<1,		//!< file is saved into the MCF
		FLAG_COMPLETE	= 1<<2,		//!< file is complete
		FLAG_COMPRESSED	= 1<<3,		//!< file is compressed using bzip2
		FLAG_ZEROSIZE	= 1<<4,		//!< the file has no data
		FLAG_STARTEDDL	= 1<<5,		//!< the file has started to download
		FLAG_HASDIFF	= 1<<6,		//!< has a diff block associated with this file
		FLAG_XECUTABLE  = 1<<7,		//!< +x has been set in linux
		FLAG_CANUSEDIFF	= 1<<8,		//!< means when the file is verified it can use the diff
	};

	virtual ~MCFFileI()=0;



	//! Gets the name of the file
	//!
	//! @return The name
	//!
	virtual const char* getName()=0;

	//! Gets the path of the file inside the root folder. i.e. \scripts\python
	//!
	//! @return The path
	//!
	virtual const char* getPath()=0;

	//! Gets the directory to the root folder. i.e. c:\root
	//!
	//! @return The root dir
	//!
	virtual const char* getDir()=0;

	//! Gets the full path (dir+path)
	//!
	//! @return The full path
	//!
	virtual std::string getFullPath()=0;

	//! Gets md5 checksum on non compressed data
	//!
	//! @return Md5 checksum
	//!
	virtual const char* getCsum()=0;

	//! Gets the md5 checksum on compressed data
	//!
	//! @return Md5 checksum
	//!
	virtual const char* getCCsum()=0;

	//! Gets the non compressed data size
	//!
	//! @return Size
	//!
	virtual uint64 getSize()=0;

	//! Gets compressed data size
	//!
	//! @return Size
	//!
	virtual uint64 getCSize()=0;

	//! Gets the current size. i.e. if not compressed, not compressed size else compressed size
	//!
	//! @return Size
	//!
	virtual uint64 getCurSize()=0;


	//! Checks to see if this file is saved in the MCF
	//!
	//! @return true for is, false for not
	//!
	virtual bool isSaved()=0;

	//! Checks to see if this file has completed downloading
	//!
	//! @return true for yes, false for no
	//!
	virtual bool isComplete()=0;

	//! Checks to see if this file is compressed
	//!
	//! @return true for is, false for not
	//!
	virtual bool isCompressed()=0;

	//! Checks to see if this file has zero size. i.e. empty file
	//!
	//! @return true for is, false for not
	//!
	virtual bool isZeroSize()=0;


	//! Gets the file flags
	//!
	//! @return Flags
	//!
	virtual uint16 getFlags()=0;


	//! Does this file have a diff to the last version
	//!
	//! @return true if has diff, false if not
	//!
	virtual bool hasDiff()=0;

	//! Gets the diff block size
	//!
	//! @return diff block size
	//!
	virtual uint64 getDiffSize()=0;

	//! Gets the diff offset in the mcf
	//!
	//! @return offset
	//!
	virtual uint64 getDiffOffSet()=0;

	//! Gets the diff block hash
	//!
	//! @return md5 hash
	//!
	virtual const char* getDiffHash()=0;
	
	//! Gets the original file hash the diff was made against
	//!
	//! @return md5 hash
	//!
	virtual const char* getDiffOrgFileHash()=0;

	//! Removes diff info
	//!
	virtual void clearDiff()=0;
};



}
#endif
