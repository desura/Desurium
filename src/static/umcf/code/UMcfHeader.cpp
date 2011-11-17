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


#pragma warning( disable : 4627 )

#include "Common.h"
#include "umcf/UMcfHeader.h"

namespace MCFCore
{
	MCFHeaderI::~MCFHeaderI()
	{
	}
}


UMcfHeader::UMcfHeader()
{
	init();
}

UMcfHeader::UMcfHeader(MCFCore::MCFHeaderI* head)
{
	init();

	if (!head)
		return;

	m_iFileVer = head->getFileVer();
	m_iBuild = head->getBuild();
	m_iId = head->getId();
	m_iType = head->getType();
	m_iXmlStart = head->getXmlStart();
	m_iXmlSize = head->getXmlSize();
	m_iFlags = head->getFlags();
	m_iParentMcf = head->getParent();
	m_iBranch = head->getBranch();
}

void UMcfHeader::init()
{
	m_szId[0] = 'L';
	m_szId[1] = 'M';
	m_szId[2] = 'C';
	m_szId[3] = 'F';
	m_szId[4] = '\0';
	m_iFileVer = MCF_CURRENTVERSION;
	m_iId = 0;
	m_iType = 0;
	m_iXmlStart = 0;
	m_iXmlSize = 0;
	m_iFlags = 0;
	m_iParentMcf = 0;
}

#ifdef WIN32
uint8 UMcfHeader::readFromFile(HANDLE hFile)
#else
uint8 UMcfHeader::readFromFile(FILE* hFile)
#endif
{
	if (!hFile)
		return UMcfHeader::ERR_NULLHANDLE;

	uint32 size = getSize();
	uint8* str = new uint8[size];

#ifdef WIN32
	DWORD dwRead = 0;
	int readRet = ReadFile(hFile, str, size, &dwRead, NULL);

	if (dwRead != size)
	{
		safe_delete(str);
		return UMcfHeader::ERR_PARTREAD;
	}
#else
	size_t amount = fread(str, 1, size, hFile);
	if (amount != size)
	{
		safe_delete(str);	
		return UMcfHeader::ERR_PARTREAD;
	}
#endif
		
	strToHeader(str);
	safe_delete(str);

	if (isValid())
		return UMcfHeader::OK;
	else
		return UMcfHeader::ERR_INVALID;
}

const uint8* UMcfHeader::headerToStr()
{
	uint8* temp = new uint8[getSize()];

	temp[0]  = m_szId[0];
	temp[1]  = m_szId[1];
	temp[2]  = m_szId[2];
	temp[3]  = m_szId[3];
	temp[4]  = (uint8) m_iFileVer;
	temp[5]  = (uint8)(m_iBuild>>0)&0xFF;
	temp[6]  = (uint8)(m_iBuild>>8)&0xFF;
	temp[7]  = (uint8)(m_iBuild>>16)&0xFF;
	temp[8]  = (uint8)(m_iBuild>>24)&0xFF;
	temp[9]  = (uint8)(m_iId>>0)&0xFF;
	temp[10] = (uint8)(m_iId>>8)&0xFF;
	temp[11] = (uint8)(m_iId>>16)&0xFF;
	temp[12] = (uint8)(m_iId>>24)&0xFF;
	temp[13] = (uint8) m_iType;
	temp[14] = (uint8)(m_iXmlStart>>0)&0xFF;
	temp[15] = (uint8)(m_iXmlStart>>8)&0xFF;
	temp[16] = (uint8)(m_iXmlStart>>16)&0xFF;
	temp[17] = (uint8)(m_iXmlStart>>24)&0xFF;
	temp[18] = (uint8)(m_iXmlStart>>32)&0xFF;
	temp[19] = (uint8)(m_iXmlStart>>40)&0xFF;
	temp[20] = (uint8)(m_iXmlStart>>48)&0xFF;
	temp[21] = (uint8)(m_iXmlStart>>52)&0xFF;
	temp[22] = (uint8)(m_iXmlSize>>0)&0xFF;
	temp[23] = (uint8)(m_iXmlSize>>8)&0xFF;
	temp[24] = (uint8)(m_iXmlSize>>16)&0xFF;
	temp[25] = (uint8)(m_iXmlSize>>24)&0xFF;
	temp[26] = (uint8) m_iFlags;
	temp[27] = (uint8)(m_iParentMcf>>0)&0xFF;
	temp[28] = (uint8)(m_iParentMcf>>8)&0xFF;
	temp[29] = (uint8)(m_iParentMcf>>16)&0xFF;
	temp[30] = (uint8)(m_iParentMcf>>24)&0xFF;

	if (m_iFileVer >= 0x02)
	{
		temp[31] = (uint8)(m_iBranch>>0)&0xFF;
		temp[32] = (uint8)(m_iBranch>>8)&0xFF;
		temp[33] = (uint8)(m_iBranch>>16)&0xFF;
		temp[34] = (uint8)(m_iBranch>>24)&0xFF;
	}

	return temp;
};

void UMcfHeader::strToHeader(const uint8* str)
{
	m_szId[0] = str[0];
	m_szId[1] = str[1];
	m_szId[2] = str[2];
	m_szId[3] = str[3];
	m_iFileVer = str[4];

	m_iBuild = MCFBuild::BuildFromInt((((uint32)str[5])) + (((uint32)str[6])<<8) + (((uint32)str[7])<<16) + (((uint32)str[8])<<24));
	m_iId = (((uint32)str[9])) + (((uint32)str[10])<<8) + (((uint32)str[11])<<16) + (((uint32)str[12])<<24);
	m_iType = str[13];
	m_iXmlStart =	(((uint64)str[14])) + (((uint64)str[15])<< 8) +
					(((uint64)str[16])<<16) + (((uint64)str[17])<<24) +
					(((uint64)str[18])<<32) + (((uint64)str[19])<<40) +
					(((uint64)str[20])<<48) + (((uint64)str[21])<<56);

	m_iXmlSize = (str[22]) + (str[23]<<8) + (str[24]<<16) + (str[25]<<24);
	m_iFlags = str[26];
	m_iParentMcf = (str[27]) + (str[28]<<8) + (str[29]<<16) + (str[30]<<24);

	if (m_iFileVer >= 0x02)
	{
		m_iBranch = MCFBranch::BranchFromInt((str[31]) + (str[32]<<8) + (str[33]<<16) + (str[34]<<24));
	}
	else
	{
		m_iBranch = MCFBranch();
	}
}


bool UMcfHeader::isValid()
{
	if ( !(m_szId[0] == 'L' && m_szId[1] == 'M' && m_szId[2] == 'C' && m_szId[3] == 'F') )
		return false;

	return true;
}

bool UMcfHeader::matches(MCFCore::MCFHeaderI* other)
{
	return (other && getDesuraId() == other->getDesuraId() && getBranch() == other->getBranch());
}

uint8 UMcfHeader::getSize()
{
	switch (m_iFileVer)
	{
	default:
	case 1: 
		return MCF_HEADERSIZE_V1;
		
	case 2: 
		return MCF_HEADERSIZE_V2;
	};

	return 0;
}

void UMcfHeader::updateFileVersion()
{
	m_iFileVer = 2;
}
