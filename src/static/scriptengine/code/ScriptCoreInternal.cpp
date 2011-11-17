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
#include "ScriptCoreInternal.h"


bool ScriptCoreInternal::s_IsInit = false;
bool ScriptCoreInternal::s_Disabled = false;

bool IsV8Init()
{
	return ScriptCoreInternal::s_IsInit && !ScriptCoreInternal::s_Disabled;
}



v8::Handle<v8::Value> JSDebug(const v8::Arguments& args);
v8::Handle<v8::Value> JSWarning(const v8::Arguments& args);
void MessageCallback(v8::Handle<v8::Message> message, v8::Handle<v8::Value> data);

extern v8::ExtensionConfiguration* RegisterJSBindings();


const char* ToCString(const v8::String::Utf8Value& value) 
{
	return *value ? *value : "<string conversion failed>";
}

void ScriptCoreInternal::OnFatalError(const char* location, const char* message)
{
	Warning(gcString("Fatal error in v8. Disabling runtime. \n{0}: {1}\n", location, message));
	s_Disabled = false;
}

void ScriptCoreInternal::init()
{
	if (s_Disabled)
		return;

	if (!s_IsInit)
	{
		s_IsInit = true;
		v8::V8::Initialize();
		
		v8::V8::AddMessageListener(&MessageCallback);
		v8::V8::SetCaptureStackTraceForUncaughtExceptions(true);
		v8::V8::SetFatalErrorHandler(&ScriptCoreInternal::OnFatalError);
	}
	
	v8::HandleScope handle_scope;
	v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New();

	global->Set(v8::String::New("Warning"), v8::FunctionTemplate::New(JSWarning));
	global->Set(v8::String::New("Debug"), v8::FunctionTemplate::New(JSDebug));

	v8::Persistent<v8::Context> context = v8::Context::New(RegisterJSBindings(), global);
	m_v8Context = context;		
}

void ScriptCoreInternal::del()
{
	if (s_Disabled)
		return;

	m_v8Context.Dispose();
}

void ScriptCoreInternal::runString(const char* string)
{
	if (s_Disabled)
		throw gcException(ERR_V8, "V8 Internal error");

	if (!string)
		throw gcException(ERR_INVALID, "String is null");

	v8::Context::Scope context_scope(m_v8Context);
	v8::HandleScope handle_scope;

	v8::TryCatch try_catch;
	v8::Handle<v8::Script> script = v8::Script::Compile(v8::String::New(string), v8::String::New("StringExe"));

	if (script.IsEmpty()) 
	{
		throw gcException(ERR_V8, gcString("V8 Err: {0}", reportException(&try_catch)));
	} 
	else 
	{
		v8::Handle<v8::Value> result = script->Run();

		if (result.IsEmpty()) 
			throw gcException(ERR_V8, gcString("V8 Err: {0}", reportException(&try_catch)));
	}
}

void ScriptCoreInternal::runScript(const char* file, const char* buff, uint32 size)
{
	if (s_Disabled)
		throw gcException(ERR_V8, "V8 Internal error");

	// Create a stack-allocated handle scope. 
	v8::HandleScope handle_scope;

	// Enter the created context for compiling and 
	// running the hello world script.
	v8::Context::Scope context_scope(m_v8Context);

	// Compile the source code. 
	v8::Handle<v8::Script> script = v8::Script::Compile(v8::String::New(buff, size), v8::String::New(file));

	if (script.IsEmpty())
		throw gcException(ERR_INVALID, "Failed to parse script file");
	else
		doRunScript(script);
}

void ScriptCoreInternal::doRunScript(v8::Handle<v8::Script> script)
{
	v8::TryCatch trycatch;

	// Run the script to get the result. 
	v8::Handle<v8::Value> result = script->Run();

	if (result.IsEmpty()) 
	{  
		v8::Handle<v8::Value> exception = trycatch.Exception();
		v8::String::AsciiValue exception_str(exception);
		throw gcException(ERR_INVALID, gcString("v8 had exception: {0}", *exception_str));
	}
}

gcString ScriptCoreInternal::reportException(v8::TryCatch* try_catch) 
{
	gcString out;

	v8::HandleScope handle_scope;
	v8::String::Utf8Value exception(try_catch->Exception());

	const char* exception_string = ToCString(exception);
	v8::Handle<v8::Message> message = try_catch->Message();

	if (message.IsEmpty()) 
	{
		// V8 didn't provide any extra information about this error; just
		// print the exception.
		out += gcString("{0}\n", exception_string);
	} 
	else 
	{
		// Print (filename):(line number): (message).
		v8::String::Utf8Value filename(message->GetScriptResourceName());
		const char* filename_string = ToCString(filename);

		int linenum = message->GetLineNumber();
		out += gcString("{0}:{1} {2}\n", filename_string, linenum, exception_string);


		// Print line of source code.
		v8::String::Utf8Value sourceline(message->GetSourceLine());
		const char* sourceline_string = ToCString(sourceline);

		out += gcString("{0}\n", sourceline_string);

		// Print wavy underline (GetUnderline is deprecated).
		int start = message->GetStartColumn();
		for (int i = 0; i < start; i++) 
		{
			out += " ";
		}

		int end = message->GetEndColumn();
		for (int i = start; i < end; i++) 
		{
			out += "^";
		}

		out += "\n";
		v8::String::Utf8Value stack_trace(try_catch->StackTrace());

		if (stack_trace.length() > 0) 
		{
			const char* stack_trace_string = ToCString(stack_trace);
			out += gcString("{0}\n", stack_trace_string);
		}
	}

	return out;
}
