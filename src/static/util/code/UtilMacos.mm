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

#import "Common.h"
#import "util/UtilMacos.h"
#import "UtilUnix.h"

#import <AppKit/NSRunningApplication.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSURL.h>

namespace UTIL
{
namespace MAC
{
	bool is64OS()
	{
		switch ([[NSRunningApplication currentApplication] executableArchitecture])
		{
			case kCFBundleExecutableArchitectureX86_64:
			case kCFBundleExecutableArchitecturePPC64:
				return true;
			default:
				return false;
		}
	}
	
	uint64 getFreeSpace(const char* path)
	{
		NSDictionary* fileAttributes = [[NSFileManager defaultManager]
										attributesOfFileSystemForPath:[NSString stringWithUTF8String:path] error:nil];
		return [[fileAttributes objectForKey:NSFileSystemFreeSize] longLongValue];
	}
	
	void setConfigValue(const std::string &configKey, const std::string &value)
	{
		return UTIL::UNIX::setConfigValue(configKey, value);
	}
	
	void setConfigValue(const std::string &configKey, uint32 value)
	{
		return UTIL::UNIX::setConfigValue(configKey, value);
	}
	
	std::wstring getApplicationsPath(std::wstring extra)
	{
		NSLog(@"not implemented method %s called", __FUNCTION__);
	}
	
	std::wstring getDesktopPath(std::wstring extra)
	{
		NSLog(@"not implemented method %s called", __FUNCTION__);
	}

	gcString getAbsPath(const gcString& path)
	{
		NSLog(@"not implemented method %s called", __FUNCTION__);
	}
	
	gcString getRelativePath(const gcString &path)
	{
		NSLog(@"not implemented method %s called", __FUNCTION__);
	}
	
	std::wstring getAppPath(std::wstring extra)
	{
		@autoreleasepool {
			return UTIL::STRING::toWStr([[[NSBundle mainBundle] bundlePath] UTF8String]);
		}
	}
	
	std::wstring getAppDataPath(std::wstring extra)
	{
		@autoreleasepool {
			NSError *error;
			NSURL *appSupportDir = [[NSFileManager defaultManager]
			                        URLForDirectory:NSApplicationSupportDirectory
			                        inDomain:NSUserDomainMask
			                        appropriateForURL:nil
			                        create:YES
			                        error:&error];
			NSString *str = [appSupportDir.path stringByAppendingString:@"/desura/"];
			return UTIL::STRING::toWStr([str UTF8String]) + extra;
		}
	}
	
	std::wstring getCommonProgramFilesPath(std::wstring extra)
	{
		NSLog(@"not implemented method %s called", __FUNCTION__);
	}
	
	std::wstring getTempInternetPath(std::wstring extra)
	{
		NSLog(@"not implemented method %s called", __FUNCTION__);
	}
	
	std::wstring getLocalAppDataPath(std::wstring extra)
	{
		NSLog(@"not implemented method %s called", __FUNCTION__);
	}
	
	std::wstring getAppInstallPath(std::wstring extra)
	{
		NSLog(@"not implemented method %s called", __FUNCTION__);
	}
	
	std::wstring getCachePath(std::wstring extra)
	{
		NSLog(@"not implemented method %s called", __FUNCTION__);
	}
	
	std::string getConfigValue(const std::string &configKey)
	{
		return UTIL::UNIX::getConfigValue(configKey);
	}
	
	std::string getOSString()
	{
		@autoreleasepool {
			return [[[NSProcessInfo processInfo] operatingSystemVersionString] UTF8String];
		}
	}
	
	std::string getCmdStdout(const char* command, int stdErrDest)
	{
		NSLog(@"not implemented method %s called", __FUNCTION__);
	}
	
	std::string getExecuteDir()
	{
		NSLog(@"not implemented method %s called", __FUNCTION__);
		return "";
	}
	
	bool launchFolder(const char* path)
	{
		NSLog(@"not implemented method %s called", __FUNCTION__);
		return false;
	}
	
	bool canLaunchBinary(OS::BinType type)
	{
		if(type == OS::BinType::MACH32
		#ifdef MACOS64
			|| type == OS::BinType::MACH64
		#endif
			|| type == OS::BinType::SH)
			return true;
		return false;
	}
}
}
