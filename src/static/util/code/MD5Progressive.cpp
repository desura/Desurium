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

#include "Common.h"
#include "util/MD5Progressive.h"
#include "third_party/md5.h"



MD5Progressive::MD5Progressive()
{
	md5 = new MD5();
}

MD5Progressive::~MD5Progressive()
{
	safe_delete(md5);
}

void MD5Progressive::update(const char* buff, uint32 size)
{
	md5->update((const unsigned char*)buff, size);
}

std::string MD5Progressive::finish()
{
	md5->finalize();
	return md5->getMd5();
}


