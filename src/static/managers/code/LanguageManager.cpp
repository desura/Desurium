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
#include "LanguageManager.h"

#include "tinyxml.h"
#include <algorithm>
#include <string.h>
#include "XMLMacros.h"

#include <branding/branding.h>


LanguageManager::LanguageManager() : BaseManager<LanguageString>( true )
{
	// load english files by default
	std::wstring path = UTIL::OS::getDataPath(L"language/english.xml");
	loadFromFile(UTIL::STRING::toStr(path).c_str());
}

LanguageManager::~LanguageManager()
{
}


const char* LanguageManager::getString(const char* name)
{
	LanguageString* temp = findItem(name);
		
	if (temp)
	{
		if (temp->str.size() == 0 && temp->ustr.size() > 0)
			temp->str = temp->ustr;

		return temp->str.c_str();
	}

	return name;
}

const wchar_t* LanguageManager::getString(const wchar_t* name)
{
	gcString n(name);

	LanguageString* temp = findItem(n.c_str());
		
	if (temp)
		return temp->ustr.c_str();
	
	return name;
}


bool LanguageManager::loadFromFile(const char* file)
{
	TiXmlDocument doc;
	doc.LoadFile(file);

#ifdef WIN32 // seemingly unused
	const char* err = doc.ErrorDesc();
#endif

	TiXmlNode *cNode = doc.FirstChild("lang");

	if (!cNode)
		return false;

	auto parseString = [this](TiXmlElement* str)
	{
		const char* name = str->Attribute("name");
		const char* val = str->GetText();

		if (!name || !val)
			return;

		LanguageString* temp = dynamic_cast<LanguageString*>(this->BaseManager::findItem(name));

		if (!temp)
		{
			temp = new LanguageString(name);
			this->addItem( temp );
		}

#ifdef DESURA_OFFICAL_BUILD
		temp->ustr = val;
#else
		std::vector<std::string> res;
		UTIL::STRING::tokenize(gcString(val), res, "Desura");

		gcString out;

		for (size_t x=0; x<res.size(); x++)
		{
			out += res[x];

			if (x+1 != res.size())
				out += PRODUCT_NAME;
		}

		temp->ustr = out;
#endif
	};

	XML::for_each_child("str", cNode->FirstChild("strings"), parseString);

#ifdef WIN32
	XML::for_each_child("str", cNode->FirstChild("windows"), parseString);
#else
	XML::for_each_child("str", cNode->FirstChild("linux"), parseString);
#endif

	return true;
}


