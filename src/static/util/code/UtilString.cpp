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
#include "util/UtilString.h"

#include <iostream>


#include "tinyxml.h"

#include "third_party/utf8.h"

namespace UTIL
{
namespace STRING
{

void zeroBuffer(char* str, size_t maxlen)
{
	uint32 x = 0;
	while (str && x<maxlen)
	{
		*str = 0;

		str++;
		x++;
	}
}

void zeroBuffer(wchar_t* str, size_t maxlen)
{
	uint32 x = 0;
	while (str && x<maxlen)
	{
		*str = 0;

		str++;
		x++;
	}
}


void printBuffer(char* buf, uint32 size)
{
#ifdef printf
#define RE_DEF_PRINTF
#undef printf
#endif

	printf("\n");

	for (uint32 x=0; x< size; x++)
	{
		printf("%02X ", buf[x]&0xFF);


			if ((x+1)%4 == 0 && x != 0)
				printf(" ");

			if ((x+1)%24 == 0 && x != 0)
				printf("\n");

	}
	printf("\n\n");

#ifdef RE_DEF_PRINTF
#define printf PrintfMsg
#endif
}



void splitURL(const char *buff, char* server, uint32 serSize, char* path, uint32 pathSize)
{
	uint32 x=0;

	if ( strncmp(buff, "http://", 7) == 0)
	{

		while (x < 7 && serSize > 0)
		{
			server[x] = buff[x];
			x++;
			serSize--;
		}
	}

	char curChar = '\0';

	do
	{
		server[x] = buff[x];
		x++;
		curChar = buff[x];
		serSize--;
	}
	while ( serSize > 0 && curChar != '/' && curChar != '\0');

	server[x] = '\0';

	if (curChar == '\0')
	{
		path[0]='/';
		path[1]='\0';
		return;
	}

	uint32 y=0;
	do
	{
		path[y]=buff[x];
		y++;
		x++;
		curChar = buff[x];
		pathSize--;
	}
	while ( pathSize > 0 && curChar != '\0');
	path[y] = '\0';
}


std::string getShortName(const std::string &src)
{
	size_t len = src.size();

	std::string temp;
	std::string dest;

	size_t y=0;
	bool nextCap = false;
	for (size_t x=0; x<len; x++)
	{
		if ((src[x] == ' ' || src[x] == ':' || src[x] == '-'))
		{
			nextCap = true;
			continue;
		}
		else if ((src[x] >= '0' && src[x] <= '9') || (src[x] >= 'A' && src[x] <= 'Z') || (src[x] >= 'a' && src[x] <= 'z'))
		{
			if (nextCap && (src[x] >= 'a' && src[x] <= 'z'))
				temp += src[x] - 32;
			else
				temp += src[x];

			nextCap = false;
			y++;
		}
	}

	len = temp.size();

	if (temp[0] >= 'a' && temp[0] <= 'z')
		dest += temp[0] - 32;
	else if (temp[0] >= 'A' && temp[0] <= 'Z')
		dest += temp[0];

	size_t x=1;
	y=1;

	while (x<len)
	{
		if (x<(len-1) && (temp[x] >= 'a' && temp[x] <= 'z') && (temp[x+1] >= 'A' && temp[x+1] <= 'Z'))
		{
			dest += temp[x+1];
			y++;
		}
		else if (temp[x] >= '0' && temp[x] <= '9')
		{
			dest += temp[x];
			y++;
		}

		x++;
	}

	return dest;
}

std::string sanitizeFileName(const std::string &str)
{
	std::string out;
	size_t size = str.size();

	for (size_t x=0; x<size; x++)
	{
		if (str[x] == '\\' || str[x] == '/')
			continue;

		if (str[x] == '.' && x+1<size && str[x+1] == '.')
			continue;

		out += str[x];
	}

	return out;
}

std::string sanitizeFilePath(const std::string &str, char slash)
{
	std::string out;
	size_t size = str.size();

	for (size_t x=0; x<size; x++)
	{
		if (str[x] == '\\' || str[x] == '/')
		{
			out += slash;
			continue;
		}

		if (str[x] == '.' && x+1<size && str[x+1] == '.')
		{
			out += "..";
			x++;
			continue;
		}

		out += str[x];
	}

	return out;
}


//source http://sourceforge.net/projects/utfcpp/
std::wstring toWStr(const std::string& utf8string)
{
	std::wstring res;

	if (sizeof(wchar_t) == 2)
		utf8::unchecked::utf8to16(utf8string.begin(), utf8string.end(), back_inserter(res));
	else if (sizeof(wchar_t) == 4)
		utf8::unchecked::utf8to32(utf8string.begin(), utf8string.end(), back_inserter(res));

	return res;
};

std::string toStr(const std::wstring& widestring)
{
	std::string res;

	if (sizeof(wchar_t) == 2)
		utf8::unchecked::utf16to8(widestring.begin(), widestring.end(), back_inserter(res));
	else if (sizeof(wchar_t) == 4)
		utf8::unchecked::utf32to8(widestring.begin(), widestring.end(), back_inserter(res));

	return res;
}

void tokenize(std::string str, std::vector<std::string>& tokens, std::string delimiter)
{
	std::string::size_type lastPos = 0;

	do
	{
		std::string::size_type pos = str.find(delimiter, lastPos);

		if (pos == std::string::npos && lastPos == 0)
			tokens.push_back(str);
		else if (pos == std::string::npos && lastPos != 0)
			tokens.push_back(str.substr(lastPos, str.length()));
		else
			tokens.push_back(str.substr(lastPos, pos - lastPos));

		if (pos == std::string::npos)
			lastPos = pos;
		else
			lastPos = pos + delimiter.length();
	}
	while (std::string::npos != lastPos && lastPos != str.size());
}




std::string urlDecode(const std::string& url)
{
	std::string res;
	std::string::size_type i;

	for (i = 0; i < url.size(); ++i)
	{
	  if (url[i] == '+')
	  {
	    res.push_back(' ');
	  }
	  else if (url[i] == '%' && url.size() > i+2)
		{
			char hexStr[3];

			hexStr[0] = url[i+1];
			hexStr[1] = url[i+2];
			hexStr[2] = 0;

			res.push_back(UTIL::MISC::hextoDec(hexStr));
			i += 2;
		}
		else
		{
			res.push_back(url[i]);
		}
	}

	return res;
}



std::string char2hex(char dec)
{
	char dig1 = (dec&0xF0)>>4;
	char dig2 = (dec&0x0F);

	if ( 0<= dig1 && dig1<= 9) 
		dig1+=48;    //0,48inascii

	if (10<= dig1 && dig1<=15) 
		dig1+=97-10; //a,97inascii

	if ( 0<= dig2 && dig2<= 9) 
		dig2+=48;

	if (10<= dig2 && dig2<=15) 
		dig2+=97-10;

	std::string r;
	r.append(&dig1, 1);
	r.append(&dig2, 1);
	return r;
}

//based on javascript encodeURIComponent()
//http://www.zedwood.com/article/111/cpp-urlencode-function
std::string urlEncode(const std::string& c)
{
	std::string escaped="";
	for(size_t i=0; i<c.length(); i++)
	{
		bool isDigit = (48 <= c[i] && c[i] <= 57);
		bool isSmallAlph = (65 <= c[i] && c[i] <= 90);
		bool isCapAlph = (97 <= c[i] && c[i] <= 122);
		bool isSpecial = (c[i]=='~' || c[i]=='!' || c[i]=='*' || c[i]=='(' || c[i]==')' || c[i]=='\'');

		if (isDigit || isSmallAlph || isCapAlph || isSpecial)
		{
			escaped.append( &c[i], 1);
		}
		else
		{
			escaped.append("%");
			//converts char 255 to string "ff"
			escaped.append(char2hex(c[i]));
		}
	}

	return escaped;
}




const char base64_chars[] =
{
	'A','B','C','D','E','F','G','H','I','J',
	'K','L','M','N','O','P','Q','R','S','T',
	'U','V','W','X','Y','Z',
	'a','b','c','d','e','f','g','h','i','j',
	'k','l','m','n','o','p','q','r','s','t',
	'u','v','w','x','y','z',
	'0','1','2','3','4','5','6','7','8','9',
	'+','/',
	(char)NULL,
};

static inline bool is_base64(unsigned char c)
{
  return (isalnum(c) || (c == '+') || (c == '/'));
}

void decodeHelper(unsigned char *buff, size_t &pos, unsigned char char_array_4[4], int i)
{
	unsigned char a = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
	unsigned char b = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
	unsigned char c = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

	if (i > 1)
	{
		buff[pos] = a;
		pos++;
	}

	if (i > 2)
	{
		buff[pos] = b;
		pos++;
	}

	if (i > 3)
	{
		buff[pos] = c;
		pos++;
	}
}

void base64_decodeCB(const std::string &encoded_string, UTIL::CB::CallbackI* callback)
{
	if (!callback)
		return;

	int in_len = encoded_string.size();
	int i = 0;
	int in_ = 0;
	unsigned char char_array_4[4];
  
	unsigned char buff[512];
	size_t pos = 0;

	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
	{
		char_array_4[i++] = encoded_string[in_]; 
		in_++;

		if (i ==4) 
		{
			for (i = 0; i <4; i++)
			{
				size_t x=0;

				while (base64_chars[x])
				{
					if (base64_chars[x] == char_array_4[i])
					{
						char_array_4[i] = x;
						break;
					}

					x++;
				}
			}

			if (pos >= 512-3)
			{
				callback->onData(buff, pos);
				pos = 0;
			}

			decodeHelper(buff, pos, char_array_4, i);
			i = 0;
		}
	}

	if (i != 0)
	{
		for (int j = 0; j<4; j++)
		{
			if (j >= i)
				char_array_4[j] = 0;

			size_t x=0;

			while (base64_chars[x])
			{
				if (base64_chars[x] == char_array_4[j])
				{
					char_array_4[j] = x;
					break;
				}

				x++;
			}
		}

		if (pos >= 512-3)
		{
			callback->onData(buff, pos);
			pos = 0;
		}

		decodeHelper(buff, pos, char_array_4, i);
	}

	if (pos != 0)
		callback->onData(buff, pos);
}




std::string base64_encode(const char* bytes, size_t in_len)
{
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--)
  {
	char_array_3[i++] = *(bytes++);
	if (i == 3) {
	  char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
	  char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
	  char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
	  char_array_4[3] = char_array_3[2] & 0x3f;

	  for(i = 0; (i <4) ; i++)
		ret += base64_chars[char_array_4[i]];
	  i = 0;
	}
  }

  if (i)
  {
	for(j = i; j < 3; j++)
	  char_array_3[j] = '\0';

	char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
	char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
	char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
	char_array_4[3] = char_array_3[2] & 0x3f;

	for (j = 0; (j < i + 1); j++)
	  ret += base64_chars[char_array_4[j]];

	while((i++ < 3))
	  ret += '=';

  }

  return ret;
}

void base64_encodeCB(const char* bytes, size_t len, UTIL::CB::CallbackI* callback)
{

}


class OutFunctor
{
public:
	bool operator()(const unsigned char* buff, uint32 size)
	{
		vOut.reserve(vOut.size() + size);

		for (size_t x=0; x<size; x++)
			vOut.push_back(buff[x]);

		return true;
	}

	std::vector<unsigned char> vOut;
};



unsigned char* base64_decode(const std::string &encoded_string, size_t &outlen)
{
	OutFunctor of;
	base64_decode(encoded_string, of);

	outlen = of.vOut.size();

	if (outlen == 0)
		return NULL;

	unsigned char* ret = new unsigned char[outlen];

	for (size_t x=0; x<outlen; x++)
		ret[x] = of.vOut[x];

	return ret;
}



std::string escape(const std::string &in)
{
	std::string out;
	out.reserve(in.size());

	std::string::const_iterator it = in.begin();

	while (it != in.end())
	{
		if (*it == '\\')
		{
			out += "\\\\";
		}
		else
		{
			out += *it;
		}

		it++;
	}

	return out;
}


}
}
