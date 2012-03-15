///////////// Copyright © 2010 Desura Pty Ltd. All rights reserved.  /////////////
//
//   Project     : desura_libcef_dll_wrapper
//   File        : SchemePost.cpp
//   Description :
//      [TODO: Write the purpose of SchemePost.cpp.]
//
//   Created On: 6/17/2010 5:04:54 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#include "SchemePost.h"
#include <string>

#define _CRT_SECURE_NO_WARNINGS

int mystrncpy_s(char* dest, size_t destSize, const char* src, size_t srcSize);

PostElement::PostElement()
{
}

PostElement::PostElement(CefRefPtr<CefPostDataElement> element)
{
	m_rPostElement = element;
}

bool PostElement::isFile()
{
	return m_rPostElement->GetType() == PDE_TYPE_BYTES;
}

bool PostElement::isBytes()
{
	return m_rPostElement->GetType() == PDE_TYPE_FILE;
}

void PostElement::setToEmpty()
{
	m_rPostElement->SetToEmpty();
}

void PostElement::setToFile(const char* fileName)
{
	m_rPostElement->SetToFile(fileName);
}

void PostElement::setToBytes(size_t size, const void* bytes)
{
	m_rPostElement->SetToBytes(size, bytes);
}

void PostElement::getFile(char *buff, size_t buffsize)
{
	std::string file = m_rPostElement->GetFile();

	if (buff)
		mystrncpy_s(buff, buffsize, file.c_str(), file.size());
}

size_t PostElement::getBytesCount()
{
	return m_rPostElement->GetBytesCount();
}

size_t PostElement::getBytes(size_t size, void* bytes)
{
	return m_rPostElement->GetBytes(size, bytes);
}






PostData::PostData()
{

}

PostData::PostData(CefRefPtr<CefPostData> data)
{
	m_rPostData = data;
}

size_t PostData::getElementCount()
{
	return m_rPostData->GetElementCount();
}

ChromiumDLL::PostElementI* PostData::getElement(size_t index)
{
	CefPostData::ElementVector eles;
	m_rPostData->GetElements(eles);

	if (index >= eles.size())
		return NULL;

	return new PostElement(eles[index]);
}

bool PostData::removeElement(ChromiumDLL::PostElementI* element)
{
	PostElement *pe = (PostElement*)element;

	bool res = false;

	if (pe)
		res = m_rPostData->RemoveElement(pe->getHandle());

	element->destroy();

	return res;
}

bool PostData::addElement(ChromiumDLL::PostElementI* element)
{
	PostElement *pe = (PostElement*)element;

	bool res = false;

	if (pe)
		res = m_rPostData->AddElement(pe->getHandle());

	element->destroy();

	return res;
}

void PostData::removeElements()
{
	m_rPostData->RemoveElements();
}
