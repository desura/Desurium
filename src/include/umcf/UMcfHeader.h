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

#ifndef DESURA_UMCF_HEADER_H
#define DESURA_UMCF_HEADER_H



#define UMCF_CURRENTVERSION 0x01

#include "mcfcore/MCFHeaderI.h"

#ifdef WIN32
#include <windows.h>
#endif

#ifdef NIX
#include <stdio.h>
#endif



//! @see MCFHeader
class UMcfHeader : public MCFCore::MCFHeaderI
{
public:

	enum UMCF_ERRORS
	{
		OK = 0,			//!< Ok
		ERR_PARTWRITE,		//!< Only wrote part of the header
		ERR_PARTREAD,		//!< Only read part of the header
		ERR_INVALID,		//!< Invalid header
		ERR_NULLHANDLE,	//!< Null file handle pased in
	};

	//! Default Constructor
	UMcfHeader();

	//! Copy Constuctor
	UMcfHeader(MCFCore::MCFHeaderI* head);


	//! Gets the size of the header for in a MCF file
	//!
	//! @return size of the header in a MCF file
	//! 
	uint8 getSize();


	//! Reads the MCF header from a MCF file
	//!
	//! @param hFile File Handle to read the header from
	//! @return Error status
	//!
#ifdef WIN32
	uint8 readFromFile(HANDLE hFile);
#else
	uint8 readFromFile(FILE* fHandle);
#endif

	//! Checks to see if the header is valid
	//!
	//! @return true for valid, false for invalid
	//!
	bool isValid();

	//! Gets the magic number for the MCF header. Should be LMCF
	//!
	//! @return Magic number as string
	//!
	const char* getMagicNumber(){return m_szId;}

	//! Gets the MCF file version
	//!
	//! @return File version
	//!
	uint8 getFileVer(){return m_iFileVer;}

	//! Gets the item version
	//!
	//! @return Item Version
	//!
	MCFBuild getBuild(){return m_iBuild;}

	//! Gets the item desura Id
	//!
	//! @return Desura Id
	//!
	uint32 getId(){return m_iId;}

	//! Gets the item desura type
	//!
	//! @return Desura Type
	//!
	uint8 getType(){return m_iType;}

	//! Gets the Xml Start position
	//!
	//! @return Xml offset
	//!
	uint64 getXmlStart(){return m_iXmlStart;}

	//! Gets the Xml Length
	//!
	//! @return Xml Length
	//!
	uint32 getXmlSize(){return m_iXmlSize;}

	//! Gets the Mcf Flags
	//!
	//! @return Mcf Flags
	//!
	uint8 getFlags(){return m_iFlags;}

	//! Gets the MCF parent version (i.e. the MCF this one was based off)
	//!
	//! @return Mcf Parent Version
	//!
	uint32 getParent(){return m_iParentMcf;}

	//! Gets the MCF branch
	//!
	//! @return Mcf branch
	//!
	MCFBranch getBranch(){return m_iBranch;}


	//! Sets the Desura type
	//!
	//! @param type Desura type id
	//!
	void setType(uint8 type){m_iType = type;}

	//! Sets the item version
	//!
	//! @param version Item version
	//!
	void setBuild(MCFBuild build){m_iBuild = build;}

	//! Sets the Desura item Id
	//!
	//! @param id Desura Id
	//!
	void setId(uint32 id){m_iId = id;}

	//! Sets the MCF parent version (i.e. the one this is based off)
	//!
	//! @param version Patent Version
	//!
	void setParent(uint32 version){m_iParentMcf = version;}

	//! Sets the MCF branch
	//!
	//! @param branch MCF branch
	//!
	void setBranch(MCFBranch branch){m_iBranch = branch;}


	//! Sets the Xml Start position
	//!
	//! @param start Xml offset
	//!
	void setXmlStart(uint64 start){m_iXmlStart=start;}

	//! Sets the Xml Length
	//!
	//! @param size Xml Length
	//!
	void setXmlSize(uint32 size){m_iXmlSize=size;}


	//! Adds a flag to the MCF
	//!
	//! @param flag Flags to add
	//!
	void addFlags(uint8 flag){m_iFlags |= flag;}

	//! Removes a flag from the MCF
	//!
	//! @param flag Flags to remove
	//!
	void delFlags(uint8 flag){m_iFlags &= ~flag;}

	//! Compares two Mcf Headers and see if they are the same
	//!
	//! @param other Header to compare against
	//! @return true If matches, false if doesnt
	//!
	bool matches(MCFCore::MCFHeaderI* other);

	DesuraId getDesuraId(){return DesuraId(m_iId, m_iType);}

	virtual void updateFileVersion();

protected:
	//! Loads header information from a string
	//!
	//! @param str Header string to load
	//!
	void strToHeader(const uint8* str);

	//! Saves header information to a string. Note: Must delete the string after
	//!
	//! @return Header String
	//!
	const uint8* headerToStr();

	//! Sets all values to default
	//!
	void init();

private:
	char		m_szId[5];		//4 id LMCF
	uint8		m_iFileVer;		//1 file version
	MCFBuild	m_iBuild;		//4 MCF build version
	uint32		m_iId;			//4 id
	uint8		m_iType;		//1 type
	uint64		m_iXmlStart;	//8 xml offset
	uint32		m_iXmlSize;		//4 xml size
	uint8		m_iFlags;		//1 part file (doesnt have all files)
	uint32		m_iParentMcf;	//the MCF this was a child to or zero if no parent
	MCFBranch	m_iBranch;		//mcf branch
};

#endif
