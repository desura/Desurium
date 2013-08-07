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
#include "util/UtilMisc.h"

#include<stdio.h>
#include<string.h>

#ifdef MACOS
  #include <sys/malloc.h>
#else
  #include<malloc.h>
#endif

#include<math.h>
#include<stdlib.h>

namespace UTIL
{
namespace MISC
{

//http://www.hoozi.com/post/b3mf9/secure-hash-algorithm-sha-1-reference-implementation-in-c-c-with-comments

#define rotateleft(x,n) ((x<<n) | (x>>(32-n)))
#define rotateright(x,n) ((x>>n) | (x<<(32-n)))
 
std::string SHA1(const unsigned char * str1, uint32 len)
{
	unsigned long int h0,h1,h2,h3,h4,a,b,c,d,e,f,k,temp;
 
	h0 = 0x67452301;
	h1 = 0xEFCDAB89;
	h2 = 0x98BADCFE;
	h3 = 0x10325476;
	h4 = 0xC3D2E1F0;
 
	unsigned char * str = new unsigned char[len+100];
	UTIL::STRING::zeroBuffer((char*)str, len+100);

	memcpy(str, str1, len);

 
	int current_length = len;
	int original_length = current_length;
	str[current_length] = 0x80;
	str[current_length + 1] = '\0';
 
#ifdef WIN32 // doesn't seem to get used
	char ic = str[current_length];
#endif
	current_length++;
 
	int ib = current_length % 64;
	if(ib<56)
		ib = 56-ib;
	else
		ib = 120 - ib;
 
	int i = 0;
	for(i=0; i<ib; i++)
	{
		str[current_length]=0x00;
		current_length++;
	}

	str[current_length + 1]='\0';
 
	for(i=0; i<6; i++)
	{
		str[current_length]=0x0;
		current_length++;
	}

	str[current_length] = (original_length * 8) / 0x100 ;
	current_length++;
	str[current_length] = (original_length * 8) % 0x100;
	current_length++;
	str[current_length+i]='\0';
 
	int number_of_chunks = current_length/64;
	unsigned long int word[80];

	for(int i=0; i<number_of_chunks; i++)
	{
		for(int j=0; j<16; j++)
		{
			word[j] = str[i*64 + j*4 + 0] * 0x1000000 + str[i*64 + j*4 + 1] * 0x10000 + str[i*64 + j*4 + 2] * 0x100 + str[i*64 + j*4 + 3];
		}
		for(int j=16; j<80; j++)
		{
			word[j] = rotateleft((word[j-3] ^ word[j-8] ^ word[j-14] ^ word[j-16]),1);
		}
 
		a = h0;
		b = h1;
		c = h2;
		d = h3;
		e = h4;
 
		for(int m=0; m<80; m++)
		{
			if(m<=19)
			{
				f = (b & c) | ((~b) & d);
				k = 0x5A827999;
			}
			else if(m<=39)
			{
				f = b ^ c ^ d;
				k = 0x6ED9EBA1;
			}
			else if(m<=59)
			{
				f = (b & c) | (b & d) | (c & d);
				k = 0x8F1BBCDC;
			}
			else
			{
				f = b ^ c ^ d;
				k = 0xCA62C1D6; 
			}
 
			temp = (rotateleft(a,5) + f + e + k + word[m]) & 0xFFFFFFFF;
			e = d;
			d = c;
			c = rotateleft(b,30);
			b = a;
			a = temp;
 
		}
 
		h0 = h0 + a;
		h1 = h1 + b;
		h2 = h2 + c;
		h3 = h3 + d;
		h4 = h4 + e;
 
	}

	safe_delete(str);
 
	return gcString("{0,8:x0}{1,8:x0}{2,8:x0}{3,8:x0}{4,8:x0}", h0, h1, h2, h3, h4);
}


}
}
