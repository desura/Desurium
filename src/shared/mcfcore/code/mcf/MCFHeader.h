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


#ifndef MCF_HEADER_H
#define MCF_HEADER_H

#include "mcfcore/MCFHeaderI.h"
#include "umcf/UMcfHeader.h"


/// \brief MCFCore namespace houses all functionality relating to MCF files.
namespace MCFCore
{

//! MCFHeader represents the binary header section of a MCF file which holds
//! the key information about the content within the MCF
//! 
//! If you make changes here make sure you update UMCFHeader as well.
class MCFHeader : public UMcfHeader
{
public:
	//! Default Constructor
	MCFHeader();

	//! Copy Constuctor
	MCFHeader(MCFHeaderI* head);

	//! Load from string Constuctor
	MCFHeader(const uint8* string);


	//! Gets the size of the header for in a MCF file always the newest version
	//!
	//! @return size of the header in a MCF file
	//! 
	static uint8 getSizeS(){return MCF_HEADERSIZE_V2;}


	//! Saves the MCF header to a MCF file
	//!
	//! @param hFile File Handle to save the header to
	//!
	void saveToFile(UTIL::FS::FileHandle& hFile);

	//! Reads the MCF header from a MCF file
	//!
	//! @param hFile File Handle to read the header from
	//!
	void readFromFile(UTIL::FS::FileHandle& hFile);


protected:

};


}



#endif
