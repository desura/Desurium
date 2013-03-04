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

#ifndef DESURA_GCSTRING_H
#define DESURA_GCSTRING_H
#ifdef _WIN32
#pragma once
#endif

#include <algorithm>

namespace UTIL
{
	namespace STRING
	{
		std::string toStr(const std::wstring& widestring);
		std::wstring toWStr(const std::string& utf8string);
	}
}



#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#include <sstream>
#include <vector>

#if defined(WIN32) && !defined(__MINGW32__)
	#include <xstring>
#endif

#ifdef __MINGW32__
	#include <stdlib.h>
#endif

#include <ctype.h>
#include <iostream>
#include <iomanip>


template <typename T> class gcBaseString;


namespace Template
{

class NullArg
{
};

enum FormatTypes
{
	NONE,
	CHAR,
	STRING,
	UINT,
	UINT64,
	INT,
	INT64,
	FLOAT,
	DOUBLE,
	BOOL,
	BOOL_STRING,
	HEX,
	HEX_BASE,
};

inline std::basic_string<wchar_t> ConverStringToW(const char* string)
{
	if (!string)
		return L"";

	return UTIL::STRING::toWStr(string);
}

inline std::basic_string<char> ConverStringToA(const wchar_t* string)
{
	if (!string)
		return "";

	return UTIL::STRING::toStr(string);
}


inline bool ConvertString(const char*& t, std::basic_stringstream<wchar_t> &oss)
{
	oss << ConverStringToW(t);
	return true;
}

inline bool ConvertString(const wchar_t*& t, std::basic_stringstream<char> &oss)
{
	oss << ConverStringToA(t);
	return true;
}

inline bool ConvertString(const char t[], std::basic_stringstream<wchar_t> &oss)
{
	oss << ConverStringToW(t);
	return true;
}

inline bool ConvertString(const wchar_t t[], std::basic_stringstream<char> &oss)
{
	oss << ConverStringToA(t);
	return true;
}

template <typename T>
bool ConvertString(const T&, std::basic_stringstream<char>&)
{
	return false;
}

template <typename T>
bool ConvertString(const T&, std::basic_stringstream<wchar_t>&)
{
	return false;
}



inline void ConvertStdString(const char* t, std::basic_string<wchar_t> &out)
{
	if (!t)
		out = L"";
	else
		out = UTIL::STRING::toWStr(t);
}

inline void ConvertStdString(const wchar_t* t, std::basic_string<char> &out)
{
	if (!t)
		out = "";
	else
		out = UTIL::STRING::toStr(t);
}

template <typename CT>
void ConvertStdString(const CT* t, std::basic_string<CT> &out)
{
	out = t;
}




template <typename CT>
std::basic_string<CT> converToStringType(const char* str)
{
	std::basic_string<CT> out;
	ConvertStdString(str, out);
	return out;
}



inline FormatTypes GetTypeFromString(std::basic_string<char> &typeString)
{
	if (typeString.size() == 1)
	{
		switch (typeString[0])
		{
		case 'c':
			return CHAR;

		case 's':
			return STRING;

		case 'u':
			return UINT;

		case 'i':
			return INT;

		case 'f':
			return FLOAT;

		case 'd':
			return DOUBLE;

		case 'b':
			return BOOL;

		case 'x':
			return HEX;
		}
	}

	if (strcmp(typeString.c_str(), "u64") == 0)
		return UINT64;
	else if (strcmp(typeString.c_str(), "i64") == 0)
		return INT64;
	else if (strcmp(typeString.c_str(), "bs") == 0)
		return BOOL_STRING;
	else if (strcmp(typeString.c_str(), "xb") == 0)
		return HEX_BASE;

	return NONE;
}

inline FormatTypes GetTypeFromString(std::basic_string<wchar_t> &typeString)
{
	std::basic_string<char> str = ConverStringToA(typeString.c_str());
	return GetTypeFromString(str);
}


template<typename CT, typename T>
std::basic_string<CT> FormatArg(const T& t, FormatTypes type, size_t len, size_t per);





template <typename CT>
std::basic_string<CT> ConvertTypeChar(const unsigned int t, size_t len, size_t per)
{
	if (t > ((unsigned char)-1))
	{
		PAUSE_DEBUGGER();
		return converToStringType<CT>("{UInt is bigger than char. Cant convert.}");
	}

	return FormatArg<CT, char>((char)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeChar(const int t, size_t len, size_t per)
{
	if (t > ((char)-1))
	{
		PAUSE_DEBUGGER();
		return converToStringType<CT>("{Int is bigger than char. Cant convert.}");
	}

	return FormatArg<CT, char>((char)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeChar(const char t, size_t len, size_t per)
{
	return FormatArg<CT, char>(t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeChar(const unsigned char t, size_t len, size_t per)
{
	return FormatArg<CT, unsigned char>(t, NONE, len, per);
}

template <typename CT, typename T>
std::basic_string<CT> ConvertTypeChar(const T&, size_t, size_t)
{
		PAUSE_DEBUGGER();
	return converToStringType<CT>("{Cant convert type to char!}");
}




template <typename CT>
std::basic_string<CT> ConvertTypeUInt(const unsigned int& t, size_t len, size_t per)
{
	return FormatArg<CT, unsigned int>((unsigned int)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeUInt(const int& t, size_t len, size_t per)
{
	return FormatArg<CT, unsigned int>((unsigned int)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeUInt(const char& t, size_t len, size_t per)
{
	return FormatArg<CT, unsigned int>((unsigned int)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeUInt(const unsigned char& t, size_t len, size_t per)
{
	return FormatArg<CT, unsigned int>((unsigned int)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeUInt(const float& t, size_t len, size_t per)
{
	return FormatArg<CT, unsigned int>((unsigned int)t, NONE, len, per);
}

template <typename CT, typename T>
std::basic_string<CT> ConvertTypeUInt(const T&, size_t, size_t)
{
		PAUSE_DEBUGGER();
	return converToStringType<CT>("{Cant convert type to uint!}");
}




template <typename CT>
std::basic_string<CT> ConvertTypeInt(const unsigned int& t, size_t len, size_t per)
{
	return FormatArg<CT, int>((int)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeInt(const int& t, size_t len, size_t per)
{
	return FormatArg<CT, int>((int)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeInt(const char& t, size_t len, size_t per)
{
	return FormatArg<CT, int>((int)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeInt(const unsigned char& t, size_t len, size_t per)
{
	return FormatArg<CT, int>((int)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeInt(const float& t, size_t len, size_t per)
{
	return FormatArg<CT, int>((int)t, NONE, len, per);
}

template <typename CT, typename T>
std::basic_string<CT> ConvertTypeInt(const T&, size_t, size_t)
{
		PAUSE_DEBUGGER();
	return converToStringType<CT>("{Cant convert type to int!}");
}






template <typename CT>
std::basic_string<CT> ConvertTypeUInt64(const unsigned int& t, size_t len, size_t per)
{
	return FormatArg<CT, unsigned long long>((unsigned long long)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeUInt64(const int& t, size_t len, size_t per)
{
	return FormatArg<CT, unsigned long long>((unsigned long long)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeUInt64(const char& t, size_t len, size_t per)
{
	return FormatArg<CT, unsigned long long>((unsigned long long)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeUInt64(const unsigned char& t, size_t len, size_t per)
{
	return FormatArg<CT, unsigned long long>((unsigned long long)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeUInt64(const float& t, size_t len, size_t per)
{
	return FormatArg<CT, unsigned long long>((unsigned long long)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeUInt64(const long long& t, size_t len, size_t per)

{
	return FormatArg<CT, unsigned long long>((unsigned long long)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeUInt64(const unsigned long long& t, size_t len, size_t per)
{
	return FormatArg<CT, unsigned long long>((unsigned long long)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeUInt64(const double& t, size_t len, size_t per)
{
	return FormatArg<CT, unsigned long long>((unsigned long long)t, NONE, len, per);
}

template <typename CT, typename T>
std::basic_string<CT> ConvertTypeUInt64(const T&, size_t, size_t)
{
		PAUSE_DEBUGGER();
	return converToStringType<CT>("{Cant convert type to uint64!}");
}





template <typename CT>
std::basic_string<CT> ConvertTypeInt64(const unsigned int& t, size_t len, size_t per)
{
	return FormatArg<CT, long long>((long long)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeInt64(const int& t, size_t len, size_t per)
{
	return FormatArg<CT, long long>((long long)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeInt64(const char& t, size_t len, size_t per)
{
	return FormatArg<CT, long long>((long long)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeInt64(const unsigned char& t, size_t len, size_t per)
{
	return FormatArg<CT, long long>((long long)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeInt64(const float& t, size_t len, size_t per)
{
	return FormatArg<CT, long long>((long long)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeInt64(const long long& t, size_t len, size_t per)
{
	return FormatArg<CT, long long>((long long)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeInt64(const unsigned long long& t, size_t len, size_t per)
{
	return FormatArg<CT, long long>((long long)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeInt64(const double& t, size_t len, size_t per)
{
	return FormatArg<CT, long long>((long long)t, NONE, len, per);
}

template <typename CT, typename T>
std::basic_string<CT> ConvertTypeInt64(const T&, size_t, size_t)
{
		PAUSE_DEBUGGER();
	return converToStringType<CT>("{Cant convert type to int64!}");
}






template <typename CT>
std::basic_string<CT> ConvertTypeFloat(const float& t, size_t len, size_t per)
{
	return FormatArg<CT, float>((float)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeFloat(const unsigned int& t, size_t len, size_t per)
{
	return FormatArg<CT, float>((float)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeFloat(const int& t, size_t len, size_t per)
{
	return FormatArg<CT, float>((float)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeFloat(const char& t, size_t len, size_t per)
{
	return FormatArg<CT, float>((float)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeFloat(const unsigned char& t, size_t len, size_t per)
{
	return FormatArg<CT, float>((float)t, NONE, len, per);
}

template <typename CT, typename T>
std::basic_string<CT> ConvertTypeFloat(const T&, size_t, size_t)
{
		PAUSE_DEBUGGER();
	return converToStringType<CT>("{Cant convert type to float!}");
}



template <typename CT>
std::basic_string<CT> ConvertTypeDouble(const unsigned int& t, size_t len, size_t per)
{
	return FormatArg<CT, double>((double)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeDouble(const int& t, size_t len, size_t per)
{
	return FormatArg<CT, double>((double)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeDouble(const char& t, size_t len, size_t per)
{
	return FormatArg<CT, double>((double)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeDouble(const unsigned char& t, size_t len, size_t per)
{
	return FormatArg<CT, double>((double)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeDouble(const float& t, size_t len, size_t per)
{
	return FormatArg<CT, double>((double)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeDouble(const long long& t, size_t len, size_t per)
{
	return FormatArg<CT, double>((double)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeDouble(const unsigned long long& t, size_t len, size_t per)
{
	return FormatArg<CT, double>((double)t, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeDouble(const double& t, size_t len, size_t per)
{
	return FormatArg<CT, double>((double)t, NONE, len, per);
}

template <typename CT, typename T>
std::basic_string<CT> ConvertTypeDouble(const T&, size_t, size_t)
{
		PAUSE_DEBUGGER();
	return converToStringType<CT>("{Cant convert type to double!}");
}





template <typename CT>
std::basic_string<CT> ConvertTypeBool(const unsigned int& t, size_t len, size_t per)
{
	return FormatArg<CT, double>(t?true:false, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeBool(const int& t, size_t len, size_t per)
{
	return FormatArg<CT, double>(t?true:false, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeBool(const char& t, size_t len, size_t per)
{
	return FormatArg<CT, double>(t?true:false, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeBool(const float& t, size_t len, size_t per)
{
	return FormatArg<CT, double>(t?true:false, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeBool(const long long& t, size_t len, size_t per)
{
	return FormatArg<CT, double>(t?true:false, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeBool(const unsigned long long& t, size_t len, size_t per)
{
	return FormatArg<CT, double>(t?true:false, NONE, len, per);
}

template <typename CT>
std::basic_string<CT> ConvertTypeBool(const double& t, size_t len, size_t per)
{
	return FormatArg<CT, double>(t?true:false, NONE, len, per);
}

template <typename CT, typename T>
std::basic_string<CT> ConvertTypeBool(const T*& t, size_t len, size_t per)
{
	return FormatArg<CT, bool>(t != 0, NONE, len, per);
}

template <typename CT, typename T>
std::basic_string<CT> ConvertTypeBool(const T&, size_t, size_t)
{
		PAUSE_DEBUGGER();
	return converToStringType<CT>("{Cant convert type to bool!}");
}



template<typename CT>
std::basic_string<CT> FormatArg(const std::string& t, FormatTypes type, size_t len, size_t per)
{
	return FormatArg<CT, char*>(t.c_str(), type, len, per);
}

template<typename CT>
std::basic_string<CT> FormatArg(const std::wstring& t, FormatTypes type, size_t len, size_t per)
{
	return FormatArg<CT, wchar_t*>(t.c_str(), type, len, per);
}

template<typename CT, typename T>
std::basic_string<CT> FormatArg(const NullArg&, FormatTypes, size_t, size_t)
{
	return std::basic_string<CT>();
}







inline void PrintToStream(const std::basic_string<char>& t, std::basic_stringstream<wchar_t> &oss)
{
	oss << ConverStringToW(t.c_str()).c_str();
}

inline void PrintToStream(const std::basic_string<wchar_t>& t, std::basic_stringstream<char> &oss)
{
	oss << ConverStringToA(t.c_str()).c_str();
}

template <typename CT>
void PrintToStream(const std::basic_string<CT>& t, std::basic_stringstream<CT> &oss)
{
	oss << t.c_str();
}


template <typename T, typename CT>
void PrintToStream(const gcBaseString<T>& t, std::basic_stringstream<CT> &oss)
{
	const std::basic_string<T> &s = dynamic_cast< const std::basic_string<T>& >(t);;
	PrintToStream(s, oss);
}

template<typename CT, typename T>
void PrintToStream(const T* t, std::basic_stringstream<CT> &oss)
{
	if (!t)
		PrintToStream("NULL", oss);
	else
		oss << t;
}

template<typename CT, typename T>
void PrintToStream(const T& t, std::basic_stringstream<CT> &oss)
{
	oss << t;
}

template<typename CT, typename T>
std::basic_string<CT> FormatArg(const T& t, FormatTypes type, size_t len, size_t per)
{
	std::basic_stringstream<CT> oss;

	if (ConvertString(t, oss))
		return oss.str();

	if (type != NONE)
	{
		switch (type)
		{
		case CHAR:
			return ConvertTypeChar<CT>(t, len, per);

		case UINT:
			return ConvertTypeUInt<CT>(t, len, per);

		case INT:
			return ConvertTypeInt<CT>(t, len, per);

		case UINT64:
			return ConvertTypeUInt64<CT>(t, len, per);

		case INT64:
			return ConvertTypeInt64<CT>(t, len, per);

		case FLOAT:
			return ConvertTypeFloat<CT>(t, len, per);

		case DOUBLE:
			return ConvertTypeDouble<CT>(t, len, per);

		case BOOL:
			return ConvertTypeBool<CT>(t, len, per);

		case STRING:
			if (typeid(T) != typeid(char*) || typeid(T) != typeid(wchar_t*))
			{
		PAUSE_DEBUGGER();
				return converToStringType<CT>("{Arg is not string but string type format is specified.}");
			}
		case HEX:
			oss << std::hex;
			break;

		case BOOL_STRING:
			oss << std::boolalpha;
			break;
		case HEX_BASE:
			break;
		case NONE:
			break;
		}
	}

	if (len != UINT_MAX)
	{
		if (per != UINT_MAX)
			len += per;

		oss << std::setw(len);
	}

	if (per != UINT_MAX)
	{
		oss.setf(std::ios::fixed, std::ios::floatfield);
		oss << std::setprecision(per);
	}

	PrintToStream(t, oss);
	return oss.str();
}


template <typename CT>
class FormatArgI
{
public:
	virtual void destroy()=0;
	virtual std::basic_string<CT> format(FormatTypes type, size_t len, size_t per)=0;
};

template <typename CT, typename T>
class FormatArgC : public FormatArgI<CT>
{
public:
	FormatArgC(const T& t) : m_t(t)
	{
#ifdef DEBUG
		m_bUsed = false;
#endif
	}

	void destroy()
	{
		delete this;
	}

	FormatArgC<CT, T> & operator=(const FormatArgC<CT, T> &a)
	{
		m_t = a.m_t;
		return *this;
	}

	std::basic_string<CT> format(FormatTypes type, size_t len, size_t per)
	{
#ifdef DEBUG
		m_bUsed = true;
#endif

		return FormatArg<CT, T>(m_t, type, len, per);
	}

	const T &m_t;
	
#ifdef DEBUG
	bool m_bUsed;
#endif
};


template <class CT, typename A, typename B, typename C, typename D, typename E, typename F>
std::basic_string<CT> Format(const CT* format, 
	const A &a, const B &b, const C &c, 
	const D &d, const E &e, const F &f);


template <class CT>
std::basic_string<CT> Format(const CT* t)
{
	return Format<CT, NullArg, NullArg, NullArg, NullArg, NullArg, NullArg>(t, NullArg(), NullArg(), NullArg(), NullArg(), NullArg(), NullArg());
}

template <class CT, typename A>
std::basic_string<CT> Format(const CT* t, const A &a)
{
	return Format<CT, A, NullArg, NullArg, NullArg, NullArg, NullArg>(t, a, NullArg(), NullArg(), NullArg(), NullArg(), NullArg());
}

template <class CT, typename A, typename B>
std::basic_string<CT> Format(const CT* t, const A &a, const B &b)
{
	return Format<CT, A, B, NullArg, NullArg, NullArg, NullArg>(t, a, b, NullArg(), NullArg(), NullArg(), NullArg());
}

template <class CT, typename A, typename B, typename C>
std::basic_string<CT> Format(const CT* t, const A &a, const B &b, const C &c)
{
	return Format<CT, A, B, C, NullArg, NullArg, NullArg>(t, a, b, c, NullArg(), NullArg(), NullArg());
}

template <class CT, typename A, typename B, typename C, typename D>
std::basic_string<CT> Format(const CT* t, const A &a, const B &b, const C &c, const D &d)
{
	return Format<CT, A, B, C, D, NullArg, NullArg>(t, a, b, c, d, NullArg(), NullArg());
}

template <class CT, typename A, typename B, typename C, typename D, typename E>
std::basic_string<CT> Format(const CT* t, const A &a, const B &b, const C &c, const D &d, const E &e)
{
	return Format<CT, A, B, C, D, E, NullArg>(t, a, b, c, d, e, NullArg());
}

inline int atoi(const char* str)
{
	return ::atoi(str);
}

inline int atoi(const wchar_t* str)
{
	if (!str)
		return 0;

#ifdef WIN32
	return ::_wtoi(str);
#else
	int val = 0;
	swscanf(str, L"%d", &val);
	return val;
#endif
}

template <class CT>
std::basic_string<CT> FormatString(const CT* format, std::vector<FormatArgI<CT>*> &argsList)
{
	std::basic_string<CT> t(format);
	std::basic_string<CT> ret;

#ifdef DEBUG
	CT percent = converToStringType<CT>("%")[0];
	CT space = converToStringType<CT>(" ")[0];

	size_t pos = t.find(percent);

	if (pos != std::basic_string<CT>::npos && t.size() > pos+1 && t[pos+1] != space && t[pos+1] != percent)
		PAUSE_DEBUGGER();
#endif

	CT leftBracket = converToStringType<CT>("{")[0];
	CT rightBracket = converToStringType<CT>("}")[0];
	CT colon = converToStringType<CT>(":")[0];
	CT dot = converToStringType<CT>(".")[0];
	CT comma = converToStringType<CT>(",")[0];

	while (!t.empty())
	{
		if (t.size() == 1)
		{
			ret += t[0];
			break;
		}

		if (t[0] == leftBracket)
		{
			std::basic_string<CT> temp;
			std::basic_string<CT> orig;
			t.erase(0, 1);

			while (t[0] != rightBracket)
			{
				//must have some other bracket set
				if (t[0] == leftBracket)
				{
					ret += leftBracket;
					ret += temp;
					temp.clear();
				}
				else
				{
					temp += t[0];
				}

				t.erase(0, 1);

				if (t.empty())
				{
					ret += leftBracket;
					ret += temp;
					break;
				}
			}

			if (t.empty())
				break;

			orig = temp;

			t.erase(0, 1);

			if (temp.empty())
			{
				ret += leftBracket;
				ret += rightBracket;
				continue;
			}

			if (isdigit(temp[0]) == false)
			{
				ret += leftBracket;
				ret.append(orig);
				ret += rightBracket;

				continue;
			}

			CT num[2] = {0};
			num[0] = temp[0];

			

			size_t arg = atoi(num);

			if (arg >= argsList.size())
			{
				ret += leftBracket;
				ret.append(orig);
				ret += rightBracket;

				PAUSE_DEBUGGER();
				continue;
			}

			temp.erase(0, 1);

			size_t len = (size_t)-1;
			size_t per = (size_t)-1;
			FormatTypes type = NONE;

			if (temp.size() > 1 && temp[0] == comma)
			{
				temp.erase(0, 1);
				std::basic_string<CT> temp2;

				while (!temp.empty() && temp[0] != dot && temp[0] != colon)
				{
					if (isdigit(temp[0]) == false)
					{
						ret += leftBracket;
						ret.append(orig);
						ret += rightBracket;

						PAUSE_DEBUGGER();
						continue;
					}

					temp2 += temp[0];
					temp.erase(0, 1);
				} 

				if (!temp2.empty())
					len = atoi(temp2.c_str());
			}

			if (temp.size() > 1 && temp[0] == dot)
			{
				temp.erase(0, 1);
				std::basic_string<CT> temp2;

				while (!temp.empty() && temp[0] != colon)
				{
					if (isdigit(temp[0]) == false)
					{
						ret += leftBracket;
						ret.append(orig);
						ret += rightBracket;

						PAUSE_DEBUGGER();
						continue;
					}

					temp2 += temp[0];
					temp.erase(0, 1);
				} 

				if (!temp2.empty())
					per = atoi(temp2.c_str());
			}

			if (temp.size() > 1 && temp[0] == colon)
			{
				temp.erase(0, 1);
				std::basic_string<CT> temp2;

				while (!temp.empty() && temp[0] != colon)
				{
					if (isalnum(temp[0]) == false)
					{
						ret += leftBracket;
						ret.append(orig);
						ret += rightBracket;

						PAUSE_DEBUGGER();
						continue;
					}

					temp2 += temp[0];
					temp.erase(0, 1);
				} 

				type = GetTypeFromString(temp2);
			}

			ret.append(argsList[arg]->format(type, len, per));
			continue;
		}

		ret += t[0];
		t.erase(0,1);
	}

#ifdef DEUBG
	for (size_t x=0; x<argsList.size(); x++)
	{
		if (argsList[x]->m_bUsed == false)
		{
			PAUSE_DEBUGGER();
		}
	}
#endif

	return ret;
}



template <class CT, typename A, typename B, typename C, typename D, typename E, typename F>
std::basic_string<CT> Format(const CT* format, 
	const A &a, const B &b, const C &c, 
	const D &d, const E &e, const F &f)
{
	std::vector<FormatArgI<CT>*> argsList;

	if (typeid(a) != typeid(NullArg))
	{
		argsList.push_back(new FormatArgC<CT, A>(a));
		if (typeid(b) != typeid(NullArg))
		{
			argsList.push_back(new FormatArgC<CT, B>(b));
			if (typeid(c) != typeid(NullArg))
			{
				argsList.push_back(new FormatArgC<CT, C>(c));
				if (typeid(d) != typeid(NullArg))
				{
					argsList.push_back(new FormatArgC<CT, D>(d));
					if (typeid(e) != typeid(NullArg))
					{
						argsList.push_back(new FormatArgC<CT, E>(e));
						if (typeid(f) != typeid(NullArg))
						{
							argsList.push_back(new FormatArgC<CT, F>(f));
						}
					}
				}
			}
		}
	}

	std::basic_string<CT> res = FormatString(format, argsList);

	for (size_t x=0; x<argsList.size(); x++)
	{
		if (argsList[x])
			argsList[x]->destroy();
	}

	return res;
}


}

//
//{N,L.P:T}
//
// N arg number
// L length
// P percision
// T Type

//
//types:
//c char

//s string
//u unsigned int
//i signed int
//u64 unsigned int 64
//d64 int 64
//f float
//d double
//b bool		"1"
//bs bool string	"true"
//x hex		"213"
//xb hex base 	"0x213"

template<typename T>
class gcBaseString : public std::basic_string<T>
{
public:
	gcBaseString()
	{
	}

	template<typename CT>
	gcBaseString(const std::basic_string<CT>& str)
	{
		std::basic_string<T> out;
		Template::ConvertStdString(str.c_str(), out);
		this->assign(out);
	}

	template<typename CT>
	gcBaseString(const CT* str)
	{
		if (!str)
			return;

		std::basic_string<T> out;
		Template::ConvertStdString(str, out);
		this->assign(out);
	}

	template <typename CT, typename A>
	gcBaseString(const CT* tIn, const A &a)
	{
		std::basic_string<T> t;
		Template::ConvertStdString(tIn, t);
		this->assign(Template::Format<T, A, Template::NullArg, Template::NullArg, Template::NullArg, Template::NullArg, Template::NullArg>(t.c_str(), a, Template::NullArg(), Template::NullArg(), Template::NullArg(), Template::NullArg(), Template::NullArg()));
	}

	template <typename CT, typename A, typename B>
	gcBaseString(const CT* tIn, const A &a, const B &b)
	{
		std::basic_string<T> t;
		Template::ConvertStdString(tIn, t);
		this->assign(Template::Format<T, A, B, Template::NullArg, Template::NullArg, Template::NullArg, Template::NullArg>(t.c_str(), a, b, Template::NullArg(), Template::NullArg(), Template::NullArg(), Template::NullArg()));
	}

	template <typename CT, typename A, typename B, typename C>
	gcBaseString(const CT* tIn, const A &a, const B &b, const C &c)
	{
		std::basic_string<T> t;
		Template::ConvertStdString(tIn, t);
		this->assign(Template::Format<T, A, B, C, Template::NullArg, Template::NullArg, Template::NullArg>(t.c_str(), a, b, c, Template::NullArg(), Template::NullArg(), Template::NullArg()));
	}

	template <typename CT, typename A, typename B, typename C, typename D>
	gcBaseString(const CT* tIn, const A &a, const B &b, const C &c, const D &d)
	{
		std::basic_string<T> t;
		Template::ConvertStdString(tIn, t);
		this->assign(Template::Format<T, A, B, C, D, Template::NullArg, Template::NullArg>(t.c_str(), a, b, c, d, Template::NullArg(), Template::NullArg()));
	}

	template <typename CT, typename A, typename B, typename C, typename D, typename E>
	gcBaseString(const CT* tIn, const A &a, const B &b, const C &c, const D &d, const E &e)
	{
		std::basic_string<T> t;
		Template::ConvertStdString(tIn, t);
		this->assign(Template::Format<T, A, B, C, D, E, Template::NullArg>(t.c_str(), a, b, c, d, e, Template::NullArg()));
	}

	template <typename CT, typename A, typename B, typename C, typename D, typename E, typename F>
	gcBaseString(const CT* tIn, const A &a, const B &b, const C &c, const D &d, const E &e, const F &f)
	{
		std::basic_string<T> t;
		Template::ConvertStdString(tIn, t);
		this->assign(Template::Format<T, A, B, C, D, E, F>(t.c_str(), a, b, c, d, e, f));
	}


	void vformat(const char* szFormat, va_list arglist)
	{
		if (!szFormat)
		{
			this->assign(gcBaseString<T>());
			return;
		}

#ifdef WIN32
		int size = vscprintf(szFormat, arglist)+1;
#else
		int size = vsnprintf(NULL, 0, szFormat, arglist)+1;
#endif

		char* temp = new char[size];

#ifdef WIN32
		vsnprintf_s(temp, size, _TRUNCATE, szFormat, arglist);
#else
		vsnprintf(temp, size, szFormat, arglist);
#endif

		std::basic_string<T> out;
		Template::ConvertStdString(temp, out);
		this->assign(out);

		safe_delete(temp);
	}


	void vformat(const wchar_t* szFormat, va_list arglist)
	{
		if (!szFormat)
		{
			this->assign(gcBaseString<T>());
			return;
		}

#ifdef WIN32
		int size = vscwprintf(szFormat, arglist)+1;
#else
		int size = vswprintf(NULL, 0, szFormat, arglist)+1;
#endif
		wchar_t* temp = new wchar_t[size];

#ifdef WIN32
		_vsnwprintf_s(temp, size, _TRUNCATE, szFormat, arglist);
#else
		vswprintf(temp, size, szFormat, arglist);
#endif

		std::basic_string<T> out;
		Template::ConvertStdString(temp, out);
		this->assign(out);

		safe_delete(temp);
	}
};


typedef gcBaseString<char> gcString;
typedef gcBaseString<wchar_t> gcWString;



namespace Template
{
	inline void PrintToStream(const gcString& t, std::basic_stringstream<wchar_t> &oss)
	{
		oss << ConverStringToW(t.c_str()).c_str();
	}

	inline void PrintToStream(const gcWString& t, std::basic_stringstream<char> &oss)
	{
		oss << ConverStringToA(t.c_str()).c_str();
	}
}


#endif
