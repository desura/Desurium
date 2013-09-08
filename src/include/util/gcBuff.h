/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Karol Herbst <git@karolherbst.de>

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

#ifndef DESURA_GCBUFFA_H
#define DESURA_GCBUFFA_H
#ifdef _WIN32
#pragma once
#endif

class gcBuff
{
public:
	gcBuff(uint32 size);
	gcBuff(const char* src, uint32 size);
	gcBuff(gcBuff &buff);
	gcBuff(gcBuff *buff);
	~gcBuff();

	char operator[] (uint32 i); 
	operator char*();

	void cpy(const char* src, uint32 size);

	char* c_ptr(){return m_cBuff;}
	uint32 size(){return m_uiSize;}

private:
	char* m_cBuff;
	uint32 m_uiSize;

};

#endif //DESURA_GCBUFF_H
