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
#include "gcJSUserInfo.h"
#include "MainApp.h"

REGISTER_JSEXTENDER(DesuraJSUserInfo);

DesuraJSUserInfo::DesuraJSUserInfo() : DesuraJSBase<DesuraJSUserInfo>("user", "native_binding_userinfo.js")
{
	REG_SIMPLE_JS_FUNCTION( isUserAdmin, DesuraJSUserInfo );
	REG_SIMPLE_JS_VOIDFUNCTION( forceUpdatePoll, DesuraJSUserInfo );
	REG_SIMPLE_JS_FUNCTION( getUserUserId, DesuraJSUserInfo );
	REG_SIMPLE_JS_FUNCTION( getUserAvatar, DesuraJSUserInfo );
	REG_SIMPLE_JS_FUNCTION( getUserProfileUrl, DesuraJSUserInfo );
	REG_SIMPLE_JS_FUNCTION( getUserProfileEditUrl, DesuraJSUserInfo );
	REG_SIMPLE_JS_FUNCTION( getUserUserNameId, DesuraJSUserInfo );
	REG_SIMPLE_JS_FUNCTION( getUserUserName, DesuraJSUserInfo );
	REG_SIMPLE_JS_FUNCTION( getUserPmCount, DesuraJSUserInfo );
	REG_SIMPLE_JS_FUNCTION( getUserUpCount, DesuraJSUserInfo );
	REG_SIMPLE_JS_FUNCTION( getUserCartCount, DesuraJSUserInfo );
}

bool DesuraJSUserInfo::isUserAdmin()
{
	return GetUserCore()->isAdmin();
}

void DesuraJSUserInfo::forceUpdatePoll()
{
	GetUserCore()->forceUpdatePoll();
}

int32 DesuraJSUserInfo::getUserUserId()
{
	return GetUserCore()->getUserId();
}

gcString DesuraJSUserInfo::getUserAvatar()
{
	return GetUserCore()->getAvatar();
}

gcString DesuraJSUserInfo::getUserProfileUrl()
{
	return GetUserCore()->getProfileUrl();
}

gcString DesuraJSUserInfo::getUserProfileEditUrl()
{
	return GetUserCore()->getProfileEditUrl();
}

gcString DesuraJSUserInfo::getUserUserNameId()
{
	return GetUserCore()->getUserNameId();
}

gcString DesuraJSUserInfo::getUserUserName()
{
	return GetUserCore()->getUserName();
}

int32 DesuraJSUserInfo::getUserPmCount()
{
	return GetUserCore()->getPmCount();
}

int32 DesuraJSUserInfo::getUserUpCount()
{
	return GetUserCore()->getUpCount();
}

int32 DesuraJSUserInfo::getUserCartCount()
{
	return GetUserCore()->getCartCount();
}

