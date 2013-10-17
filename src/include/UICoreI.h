///////////// Copyright 2010 Desura Pty Ltd. All rights reserved. /////////////
//
//   Project     : uicore
//   File        : uicore.h
//   Description :
//      [TODO: Write the purpose of uicore.h.]
//
//   Created On: 9/27/2009 3:58:27 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#ifndef DESURA_UICORE_H
#define DESURA_UICORE_H
#ifdef _WIN32
#pragma once
#endif


typedef void (*RestartFP)(const char*);
typedef void (*DumpLevelFP)(unsigned char);

#ifdef WIN32
typedef void (*DumpSettingsFP)(const wchar_t*, bool);
#else
typedef void (*DumpSettingsFP)(const char*, bool);
#endif

//! Interface to setup and start the ui part of desura
class UICoreI
{
public:
#ifdef WIN32
	//! Initilizes wx widgets
	//!
	//! @param hInst HInstance of loading executable
	//! @param cmdShow Default show state
	//! @param argc Number of arguments
	//! @param argv Argument array
	//! @return true if successful
	//! 
	virtual bool initWxWidgets(HINSTANCE hInst, int cmdShow, int argc, char** argv)=0;
	
	//! On idle callback
	//!
	virtual BOOL onIdle()=0;
	
	//! Translates a windows msg and handles it if it can
	//!
	//! @param msg Windows msg
	//! @return true if handle
	//!
	virtual bool preTranslateMessage(MSG *msg)=0;
	
	//! Get handle of main window
	//!
	virtual HWND getHWND()=0;	
#else
	//! Initilizes wx widgets. Warning, will block until UI shutsdown
	//!
	//! @param argc Number of arguments
	//! @param argv Argument array
	//! @return true if successful
	//!
	virtual bool initWxWidgets(int argc, char** argv)=0;
	
#ifdef NIX
	//! Check to see if any other instances are running
	//!
	//! @return true if no other instances are running
	//!
	virtual bool singleInstantCheck(const char* args = NULL)=0;
	
	//! Kills single instance check
	//!
	virtual void destroySingleInstanceCheck()=0;
	
	//! Disables single instance check and stops it been deleted in this process
	//!
	virtual void disableSingleInstanceLock()=0;
#endif
#endif

	//! Closes the main window causing the end of the application
	//!	
	virtual void closeMainWindow()=0;
	
	//! Sets the bootloader version
	//!	
	virtual void setDesuraVersion(const char* version)=0;

	//! Stops the UI and grabs the return code
	//!
	//! @param result variable to save return code into
	//!
	virtual void exitApp(int* result)=0;
	
	//! Sets the restart function callback
	//!
	virtual void setRestartFunction(RestartFP rfp)=0;
	
	//! Sets the crash dump settings callback
	//!	
	virtual void setCrashDumpSettings(DumpSettingsFP dsfp)=0;
	
	//! Sets the crash dump level callback
	//!	
	virtual void setCrashDumpLevel(DumpLevelFP dlfp)=0;

	//runs all unit tests
	virtual int runUnitTests(int argc, char** argv) = 0;
};

typedef UICoreI* (*UICoreFP)();

#endif //DESURA_UICORE_H
