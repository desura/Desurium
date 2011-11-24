
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

#ifndef DLB_FILES_H
#define DLB_FILES_H

#include <string> // std::wstring
#include <sys/types.h> // _stat64i32
#include <sys/stat.h> // stat()
#include <limits.h> // MAX_PATH
#include <cerrno> // errno
#include <vector>

#include "Common.h" // ERROR_OUTPUT(), define NIX
#include "util/UtilString.h" // tokenize

#define STR_APPDATA "/AppData"

std::string GetAppPath(std::string extra = "");
std::string GetAppDataPath(std::string extra = "");
bool ChangeToAppDir();
bool FileExists(const char* file = NULL);
bool DeleteFile(const char* file = NULL);
bool DeleteFile(const wchar_t* file = NULL);

#endif
