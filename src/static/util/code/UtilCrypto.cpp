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

#include "Common.h"


//aes headers

#ifdef WIN32
#include <wincon.h>
#endif

namespace UTIL
{
namespace CRYPTO
{

//this will encrypet a string using aes
void ciphertext(char** dest, const char* str, const char* key)
{
}

//this will decrypet a string using aes
void deCiphertext(char** dest, const char* str, const char* key)
{
}

#ifdef WIN32

typedef struct _ASTAT_
{

  ADAPTER_STATUS adapt;
  NAME_BUFFER    NameBuff [30];

}ASTAT, * PASTAT;

ASTAT Adapter;



void getAMacAddress(char* macAdd, uint32 size, uint32 adapterNum)
{
	NCB Ncb;
	UCHAR uRetCode;
	LANA_ENUM   lenum;

	memset( &Ncb, 0, sizeof(Ncb) );
	Ncb.ncb_command = NCBENUM;
	Ncb.ncb_buffer = (UCHAR *)&lenum;
	Ncb.ncb_length = sizeof(lenum);
	uRetCode = Netbios( &Ncb );
	//printf( "The NCBENUM return code is: 0x%x \n", uRetCode );

	if (adapterNum > lenum.length)
		adapterNum = 0;

	if (lenum.length > 0)
	{
		memset( &Ncb, 0, sizeof(Ncb) );
		Ncb.ncb_command = NCBRESET;
		Ncb.ncb_lana_num = lenum.lana[adapterNum];

		uRetCode = Netbios( &Ncb );
		//printf( "The NCBRESET on LANA %d return code is: 0x%x \n", lenum.lana[i], uRetCode );

		memset( &Ncb, 0, sizeof (Ncb) );
		Ncb.ncb_command = NCBASTAT;
		Ncb.ncb_lana_num = lenum.lana[adapterNum];

		Safe::strcpy((char*)Ncb.ncb_callname, 16, "*               " );
		Ncb.ncb_buffer = (unsigned char *) &Adapter;
		Ncb.ncb_length = sizeof(Adapter);

		uRetCode = Netbios( &Ncb );
		//printf( "The NCBASTAT on LANA %d return code is: 0x%x \n", lenum.lana[i], uRetCode );

		if ( uRetCode == 0 )
		{
			Safe::snprintf(macAdd, 13, "%02x%02x%02x%02x%02x%02x",Adapter.adapt.adapter_address[0],Adapter.adapt.adapter_address[1],Adapter.adapt.adapter_address[2],Adapter.adapt.adapter_address[3],Adapter.adapt.adapter_address[4],Adapter.adapt.adapter_address[5] );
			return;
		}
	}
}

#endif

}
}
