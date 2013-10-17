/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Karol Herbst <git@karolherbst.de>

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

#ifndef DESURA_BASEMANAGER_H
#define DESURA_BASEMANAGER_H
#ifdef _WIN32
#pragma once
#endif

//this disables this warning:
//warning C4275: non dll-interface class 'BaseManager' used as base for dll-interface class 'wildcardMng'
#pragma warning( disable : 4275)
#include <map>

class BaseItem
{
public:
	BaseItem()
	{
		m_uiHash = 0;
	}

	BaseItem(const char* name)
	:	m_szName(name)
	{
		m_uiHash = UTIL::MISC::RSHash_CSTR(name);
	}

	uint64 getHash(){return m_uiHash;}
	const char* getName(){return m_szName.c_str();}

protected:
	gcString m_szName;
	uint64 m_uiHash;
};



template <class T>
class BaseManager
{
public:
	BaseManager<T>(bool cleanUp = false)
	{
		m_bCleanUp = cleanUp;
	}

	virtual ~BaseManager<T>()
	{
		removeAll();
	}

#if defined WIN32 || GCC_VERSION > 40500 || ( __clang_major__ > 2 && __clang_minor__ > 0 )
	template <typename F>
	void for_each(F &f)
	{
		std::for_each(m_mItemMap.begin(), m_mItemMap.end(), [&f](std::pair<uint64, T*> p){
			f(p.second);
		});
	}
	
	template <typename F>
	void for_each(const F &f)
	{
		std::for_each(m_mItemMap.begin(), m_mItemMap.end(), [&f](std::pair<uint64, T*> p){
			f(p.second);
		});
	}	
#endif


	T* findItem(uint64 hash)
	{
		if (m_mItemMap.empty())
			return NULL;

		typename std::map<uint64,T*>::iterator it = m_mItemMap.find(hash);

		if (it == m_mItemMap.end())
			return NULL;

		return it->second;
	}

	T* findItem(const char* name)
	{
		uint64 hash = UTIL::MISC::RSHash_CSTR(name);
		return findItem(hash);
	}

	T* getItem(uint32 index)
	{
		typename std::map<uint64, T*>::iterator it;
		typename std::map<uint64, T*>::iterator endit = m_mItemMap.end();

		uint32 count = 0;

		for (it = m_mItemMap.begin(); it != endit; ++it)
		{
			if (count == index)
				return it->second;	

			count++;
		}

		return NULL;
	}

	uint32 getCount(){return (uint32)m_mItemMap.size();}

	void removeAll()
	{
		if (m_bCleanUp)
			safe_delete(m_mItemMap);
	}

protected:
	void addItem(T* item)
	{
		if (item)
			addItem(item->getHash(), item);
	}

	void addItem(uint64 index, T* item)
	{
		if (item)
			m_mItemMap.insert(std::pair<uint64,T*>(index, item));
	}

	void removeItem(const char* name, bool del = false)
	{
		uint64 hash = UTIL::MISC::RSHash_CSTR(name);
		removeItem(hash, del);
	}

	void removeItem(uint64 hash, bool del = false)
	{
		if (m_mItemMap.empty())
			return;

		typename std::map<uint64, T*>::iterator it = m_mItemMap.find(hash);

		if (it == m_mItemMap.end())
			return;

		if (del)
			safe_delete(it->second);

		m_mItemMap.erase(it);
	}

	std::map<uint64, T*> m_mItemMap;

private:
	bool m_bCleanUp;
};


#endif //DESURA_BASEMANAGER_H
