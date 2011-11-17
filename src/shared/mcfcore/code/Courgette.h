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

#ifndef DESURA_COURGETTE_H
#define DESURA_COURGETTE_H
#ifdef _WIN32
#pragma once
#endif

class CourgetteI;

class CourgetteWriteCallbackI
{
public:
	virtual bool writeData(const char* buff, size_t size)=0;
};

template <typename T>
class CourgetteWriteCallback : public CourgetteWriteCallbackI
{
public:
	CourgetteWriteCallback(T &t) : m_Callback(t)
	{
	}

	virtual bool writeData(const char* buff, size_t size)
	{
		return m_Callback(buff, size);
	}

private:
	T &m_Callback;
};

class CourgetteInstance
{
public:
	CourgetteInstance();
	~CourgetteInstance();

	template <typename T>
	bool createDiff(const char* buffOld, size_t sizeOld, const char* buffNew, size_t sizeNew, T &callback)
	{
		CourgetteWriteCallback<T> c(callback);
		return createDiffCB(buffOld, sizeOld, buffNew, sizeNew, &c);
	}

	template <typename T>
	bool createDiff(const char* buffOld, size_t sizeOld, const char* buffNew, size_t sizeNew, const T &callback)
	{
		CourgetteWriteCallback<const T> c(callback);
		return createDiffCB(buffOld, sizeOld, buffNew, sizeNew, &c);
	}

	template <typename T>
	bool applyDiff(const char* fileOld, const char* fileDiff, T &callback)
	{
		CourgetteWriteCallback<T> c(callback);
		return applyDiffCB(fileOld, fileDiff, &c);
	}

	template <typename T>
	bool applyDiff(const char* fileOld, const char* fileDiff, const T &callback)
	{
		CourgetteWriteCallback<const T> c(callback);
		return applyDiffCB(fileOld, fileDiff, &c);
	}

	bool createDiffCB(const char* buffOld, size_t sizeOld, const char* buffNew, size_t sizeNew, CourgetteWriteCallbackI* callback);
	bool applyDiffCB(const char* fileOld, const char* fileDiff, CourgetteWriteCallbackI* callback);

private:
	CourgetteI* m_pCourgette;
};



#endif //DESURA_COURGETTE_H
