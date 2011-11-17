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

#ifndef DESURA_CONCOMMAND_H
#define DESURA_CONCOMMAND_H
#ifdef _WIN32
#pragma once
#endif

#include "BaseManager.h"

typedef void (*ConCommandCallBackFn)(std::vector<gcString> &vArgList);

#define CONCOMMAND( name, printname )		void name##_cc_func(std::vector<gcString> &vArgList);				\
											ConCommand name = ConCommand( printname, & name##_cc_func);		\
											void name##_cc_func(std::vector<gcString> &vArgList)

//! ConCommand repersents a console command provided in the application console
class ConCommand : public BaseItem
{
public:
	//! @param printName Name that shows up in the console to call this by
	//! @param callBack Function to call when the con command is executed
	ConCommand(const char* printName, ConCommandCallBackFn callBack);
	~ConCommand();

	//! Call the ConCommand with arg list
	//!
	//! @param vArgList List of arguments
	//!
	void Call(std::vector<gcString> &vArgList);

	//! Same as Call
	void operator() (std::vector<gcString> &vArgList);
	
	//! Same as Call with empty list
	void operator() ();

private:
	//! Callback function
	ConCommandCallBackFn m_pCallBack;
	
	//! Has this ConCommand been registered with the manager
	bool m_bReg;
};

#endif //DESURA_CONCOMMAND_H
