/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>

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

#ifndef DESURA_LOG_CALLBACK_H
#define DESURA_LOG_CALLBACK_H
#ifdef _WIN32
#pragma once
#endif

class Color;

typedef void (* MsgCallBackFn)(const char*, Color*);
typedef void (* MsgCallBackWFn)(const wchar_t*, Color*);

typedef void (* SpecialCallBackFn)(const char*);
typedef void (* SpecialCallBackWFn)(const wchar_t*);



class LogCallback
{
public:
	LogCallback()
	{
		m_cbMsg = NULL;
		m_cbMsgW = NULL;
		m_cbWarn = NULL;
		m_cbWarnW = NULL;
		m_cbDebug = NULL;
		m_cbDebugW = NULL;
	}

	void Msg(const char* msg, Color* col = NULL)
	{
		if (m_cbMsg)
			m_cbMsg(msg, col);
	}

	void Msg_W(const wchar_t* msg, Color* col = NULL)
	{
		if (m_cbMsgW)
			m_cbMsgW(msg, col);
	}

	void Warn(const char* msg)
	{
		if (m_cbWarn)
			m_cbWarn(msg);
	}

	void Warn_W(const wchar_t* msg)
	{
		if (m_cbWarnW)
			m_cbWarnW(msg);
	}

	void Debug(const char* msg)
	{
		if (m_cbDebug)
			m_cbDebug(msg);
	}

	void Debug_W(const wchar_t* msg)
	{
		if (m_cbDebugW)
			m_cbDebugW(msg);
	}

	void RegMsg(MsgCallBackFn cb)
	{
		m_cbMsg = cb;
	}

	void RegMsg(MsgCallBackWFn cb)
	{
		m_cbMsgW = cb;
	}

	void RegWarn(SpecialCallBackFn cb)
	{
		m_cbWarn = cb;
	}

	void RegWarn(SpecialCallBackWFn cb)
	{
		m_cbWarnW = cb;
	}

	void RegDebug(SpecialCallBackFn cb)
	{
		m_cbDebug = cb;
	}	

	void RegDebug(SpecialCallBackWFn cb)
	{
		m_cbDebugW = cb;
	}

private:
	MsgCallBackFn		m_cbMsg;
	MsgCallBackWFn		m_cbMsgW;
	
	SpecialCallBackFn	m_cbWarn;
	SpecialCallBackWFn	m_cbWarnW;

	SpecialCallBackFn	m_cbDebug;
	SpecialCallBackWFn	m_cbDebugW;
};

#endif //DESURA_LOG_CALLBACK_H
