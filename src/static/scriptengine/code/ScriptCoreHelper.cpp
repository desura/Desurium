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
#include <iostream>
#include "v8.h"


void MessageCallback(v8::Handle<v8::Message> message, v8::Handle<v8::Value> data)
{
	v8::String::AsciiValue exception_str(message->Get());
	const char* ex = *exception_str;

	Warning(gcString("v8 Message Callback: {0}\n", ex));
}

template <typename F>
void JSPrint(const v8::Handle<v8::Value>& arg, F messageFunct) 
{
	if (arg->IsObject())
	{
		//convert the args[i] type to normal char* string
		v8::String::AsciiValue str(arg->ToObject()->ObjectProtoToString());

		messageFunct("Obj: ");
		messageFunct(gcString(*str).c_str());

		for (int32 x=0; x<arg->ToObject()->InternalFieldCount(); x++)
			JSPrint(arg->ToObject()->Get(x), messageFunct);
	}
	else if (arg->IsString())
	{
		//convert the args[i] type to normal char* string
		v8::String::AsciiValue str(arg->ToString());

		messageFunct("Str: ");
		messageFunct(gcString(*str).c_str());
	}
	else if (arg->IsInt32() || arg->IsUint32())
	{
		//convert the args[i] type to normal char* string
		v8::String::AsciiValue str(arg->ToInteger());

		messageFunct("Int: ");
		messageFunct(gcString(*str).c_str());
	}
	else if (arg->IsNumber())
	{
		//convert the args[i] type to normal char* string
		v8::String::AsciiValue str(arg->ToNumber());
		messageFunct("Num: ");
		messageFunct(gcString(*str).c_str());
	}
	else if (arg->IsNull())
	{
		messageFunct("[NULL]");
	}
	else if (arg->IsUndefined())
	{
		messageFunct("[Undefined]");
	}
	else
	{
		//convert the args[i] type to normal char* string
		v8::String::AsciiValue str(arg->ToDetailString());
		messageFunct(gcString(*str).c_str());
	}
}

template <typename F>
v8::Handle<v8::Value> JSPrint(const v8::Arguments& args, F messageFunct) 
{
	bool first = true;
	for (int i = 0; i < args.Length(); i++)
	{
		v8::HandleScope handle_scope;
		if (first)
		{
			first = false;
		}
		else
		{
			fprintf(stdout, " ");
		}

		JSPrint(args[i], messageFunct);
	}

	messageFunct("\n");
	//returning Undefined is the same as returning void...
	return v8::Undefined();
}

v8::Handle<v8::Value> JSDebug(const v8::Arguments& args) 
{
	std::string out;
	v8::Handle<v8::Value> ret = JSPrint(args, [&out](const char* msg){
			out += msg;
	});

	Msg(out.c_str());
	return ret;
}

v8::Handle<v8::Value> JSWarning(const v8::Arguments& args) 
{
	std::string out;

	v8::Handle<v8::Value> ret = JSPrint(args, [&out](const char* msg){
			out += msg;
	});

	Warning(out.c_str());
	return ret;
}