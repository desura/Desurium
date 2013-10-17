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

#ifndef DESURA_MCFUSERCOOKIES_H
#define DESURA_MCFUSERCOOKIES_H
#ifdef _WIN32
#pragma once
#endif

namespace MCFCore
{
namespace Misc
{

//! This class is a container for desura user cookies that are needed to get
//! download urls for the mcfs
//!
class UserCookies
{
public:
	UserCookies()
	{
		m_uiUserId = 0;
	}

	//! Sets the cookies into a HttpHandle
	//!
	//! @param hh HttpHandle
	//! @see HttpHandle
	void set(HttpHandle& hh)
	{
		hh->addCookie("freeman", m_szIdCookie.c_str());
		hh->addCookie("masterchief", m_szSessCookie.c_str());
		hh->setUserAgent(m_szUserAgent.c_str());
	}

	//! Sets the user name cookie
	//!
	//! @param name Username cookie
	//!
	void setId(const char* id)
	{
		m_szIdCookie = gcString(id);
	}

	//! Sets the session cookie
	//!
	//! @param sess Session cookie
	//!
	void setSess(const char* sess)
	{
		m_szSessCookie = gcString(sess);
	}

	//! Sets the useragent to use when doing http request
	//!
	//! @param uagent Useragent
	//!
	void setUAgent(const char* uagent)
	{
		m_szUserAgent = gcString(uagent);
	}

	//! Sets the user id number
	//!
	//! @param id Userid
	//!
	void setUserId(uint32 id)
	{
		m_uiUserId = id;
	}
		
	//! Gets the user id number
	//!
	//! @return Userid
	//!
	uint32 getUserId()
	{
		return m_uiUserId;
	}

	const char* getIdCookie()
	{
		return m_szIdCookie.c_str();
	}	

	const char* getSessCookie()
	{
		return m_szSessCookie.c_str();
	}	

private:
	uint32 m_uiUserId;

	gcString m_szIdCookie;
	gcString m_szSessCookie;

	gcString m_szUserAgent;
};

}}

#endif //DESURA_MCFUSERCOOKIES_H
