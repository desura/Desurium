/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Ian T. Jacobsen <iantj92@gmail.com>
          (C) Jookia <166291@gmail.com>

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

#ifndef DESURA_XML_MACROS_H
#define DESURA_XML_MACROS_H
#ifdef _WIN32
#pragma once
#endif

typedef void (*gcszFn)(const char*);


namespace XML
{

	
template <typename T>
void for_each_child(const char* name, tinyxml2::XMLNode* parent, const T& t)
{
	if (!parent || !name)
		return;

	tinyxml2::XMLElement* child = parent->FirstChildElement(name);

	while (child)
	{
		t(child);
		child = child->NextSiblingElement(name);
	};
}

inline void GetAtt(const char* name, gcString& outVal, tinyxml2::XMLElement* el)
{
	if (!el || !name)
		return;

	const char* val = el->Attribute(name);

	if (!val)
		return;

	outVal = val;
}

inline void GetAtt(const char* name, uint32& outVal, tinyxml2::XMLElement* el)
{
	gcString r;
	GetAtt(name, r, el);

	if (r.size() > 0)
		outVal = atoi(r.c_str());
}

template <class T>
inline bool GetChild(const char* name, T* obj, void (T::*func)(const char*), tinyxml2::XMLNode* node)
{
	if (!name || !func || !node || !obj)
		return false;

	tinyxml2::XMLElement* child = node->FirstChildElement(name);
	
	if (child)
		(*obj.*func)(child->GetText());

	return !!child;
}

inline bool GetChild(const char* name, gcszFn func, tinyxml2::XMLNode* node)
{
	if (!name || !func || !node)
		return false;

	tinyxml2::XMLElement* child = node->FirstChildElement(name);

	if (child)
		func(child->GetText());

	return !!child;
}

inline bool GetChild(const char* name, gcString& str, tinyxml2::XMLNode* node)
{
	if (!name || !node)
		return false;

	tinyxml2::XMLElement* child = node->FirstChildElement(name);

	if (child)
		str = child->GetText();

	return !!child;
}

inline bool GetChild(const char* name, std::string& str, tinyxml2::XMLNode* node)
{
	if (!name || !node)
		return false;

	tinyxml2::XMLElement* child = node->FirstChildElement(name);

	if (child && child->GetText())
		str = child->GetText();

	return !!child;
}

inline bool GetChild(const char* name, char*& str, tinyxml2::XMLNode* node)
{
	gcString string;
	bool res = GetChild(name, string, node);

	safe_delete(str);
	str = new char[string.length()+1];
		
#ifdef WIN32
	strcpy_s(str, string.length()+1, string.c_str());
#else
	strcpy(str, string.c_str());
#endif

	return res;
}

inline bool GetChild(const char* name, int32& num, tinyxml2::XMLNode* node)
{
	gcString string("0");
	bool res = GetChild(name, string, node);

	if (res)
		num = atoi(string.c_str());

	return res;
}

inline bool GetChild(const char* name, uint32& num, tinyxml2::XMLNode* node)
{
	int32 value = 0;
	bool res = GetChild(name, value, node);

	if (res)
		num = (uint32)value;

	return res;
}

inline bool GetChild(const char* name, int64& num, tinyxml2::XMLNode* node)
{
	gcString string("0");
	bool res = GetChild(name, string, node);

	if (res)
		num = UTIL::MISC::atoll(string.c_str());

	return res;
}

inline bool GetChild(const char* name, uint64& num, tinyxml2::XMLNode* node)
{
	int64 value = 0;
	bool res = GetChild(name, value, node);

	if (res)
		num = (uint64)value;

	return res;
}

inline bool GetChild(const char* name, uint16& num, tinyxml2::XMLNode* node)
{
	int32 value = 0;
	bool res = GetChild(name, value, node);

	if (res)
		num = (uint16)value;

	return res;
}

inline bool GetChild(const char* name, uint8& num, tinyxml2::XMLNode* node)
{
	int32 value = 0;
	bool res = GetChild(name, value, node);

	if (res)
		num = (uint8)value;

	return res;
}

inline bool GetChild(const char* name, bool& num, tinyxml2::XMLNode* node)
{
	gcString value;
	bool res = GetChild(name, value, node);

	if (res)
		num = (value == "true" || value == "1" || value == "yes");

	return res;
}

inline void WriteChild(const char* name, const char* value, tinyxml2::XMLNode* node, tinyxml2::XMLDocument& doc)
{
	if ( !value || !node || !name)
		return;

	tinyxml2::XMLElement * newEle = doc.NewElement( name );
	tinyxml2::XMLText * newTextEle = doc.NewText( value );
	newEle->InsertEndChild(newTextEle);
	node->InsertEndChild( newEle );
}

inline void WriteChild(const char* name, const gcString &val, tinyxml2::XMLNode* node, tinyxml2::XMLDocument& doc)
{
	WriteChild(name, val.c_str(), node, doc);
}

template <typename T>
inline void WriteChild(const char* name, T &val, tinyxml2::XMLNode* node, tinyxml2::XMLDocument& doc)
{
	WriteChild(name, gcString("{0}", val).c_str(), node, doc);
}

template <typename T>
inline void WriteChild(const char* name, T* val, tinyxml2::XMLNode* node, tinyxml2::XMLDocument& doc)
{
	if (val == NULL)
		WriteChild(name, "", node, doc);
	else
		WriteChild(name, gcString("{0}", val).c_str(), node, doc);
}


inline bool isValidElement(tinyxml2::XMLNode* node)
{
	if (!node)
		return false;

	return (node->ToElement()?true:false);
}


//! Returns version
inline uint32 processStatus(tinyxml2::XMLDocument& doc, const char* root)
{
	uint32 v = 1;

	tinyxml2::XMLElement *uNode = doc.FirstChildElement(root);

	if (!uNode)
		uNode = doc.FirstChildElement("servererror");

	if (!uNode)
		throw gcException(ERR_BADXML, "Missing root node");

	tinyxml2::XMLElement* sNode = uNode->FirstChildElement("status");

	if (!sNode)
		throw gcException(ERR_BADXML, "Missing status node");

	uint32 status = 0;
	const char* statStr = sNode->Attribute("code");

	if (!statStr)
		throw gcException(ERR_BADXML, "No status code");
		
	status = atoi(statStr);

	if (status != 0)
		throw gcException(ERR_BADSTATUS, status, gcString("Status: {0}", sNode->GetText()));


	const char* verStr = uNode->Attribute("version");

	if (verStr)
		v = atoi(verStr);

	if (v == 0)
		v = 1;

	return v;
}

inline void loadBuffer(tinyxml2::XMLDocument& doc, char* buff)
{
	doc.DeleteChildren();
	doc.Parse(buff);
}

}

#endif //DESURA_XML_MACROS_H
