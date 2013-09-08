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

#ifndef DESURA_PROGRESSIVECRC_H
#define DESURA_PROGRESSIVECRC_H
#ifdef _WIN32
#pragma once
#endif

namespace MCFCore
{
namespace Misc
{

//! Allows calculation of crc values as data becomes avaliable (i.e. while compressing the files)
class ProgressiveCRC
{
public:
	//! Constuctor
	//!
	//! @param blockSize Size of blocks to do crc on
	//!
	ProgressiveCRC(uint32 blockSize);
	~ProgressiveCRC();

	//! Adds data to calc the crc's on
	//!
	//! @param buff Data buffer
	//! @param size Data size
	//!
	void addData(const unsigned char* buff, uint32 size);

	//! Gets the vector of crc's
	//!
	//! @return CRC vector
	//!
	std::vector<uint32>& getVector();

protected:
	//! Finishes the current crc generation and starts the next
	//!
	void finishCRC();

private:
	uint32 m_uiBlockSize;
	uint32 m_uiDone;

	unsigned long m_uiCurCRC;

	std::vector<uint32> m_vCRCList;
};


}
}

#endif //DESURA_PROGRESSIVECRC_H
