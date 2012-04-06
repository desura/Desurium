/*
Desura is the leading indie game distribution platform
Copyright (C) 2012 Jookia

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

#include <Common.h>
#include <mcfcore/MCFMain.h>
#include <cstdio>

void PrintfMsg(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);

#ifdef DEBUG
	char out[1024]={0};
	vsnprintf(out, 1024, format, args);
	OutputDebugString(out);
#endif

	va_end(args);
}

int main(int argCount, const char* args[])
{
	if(argCount != 3)
	{
		printf("Usage: mcf_extract FILE DIR\n");
		return 1;
	}
	
	const char* filename = args[1];
	const char* dirname  = args[2];
	
	McfHandle mcf((MCFCore::MCFI*)MCFCore::FactoryBuilder(MCF_FACTORY));
	
	mcf->setFile(filename);
	mcf->parseMCF();
	mcf->saveFiles(dirname);
	
	return 0;
}
