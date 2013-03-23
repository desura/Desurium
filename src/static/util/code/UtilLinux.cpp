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

#include <sys/statvfs.h>
#include <wordexp.h>

#include "Common.h"
#include "util/UtilLinux.h"
#include "UtilUnix.h"

#include <unistd.h>
#include <fstream>
#include <limits.h>

#include <sqlite3x.hpp>

#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <dirent.h>
#include <utime.h>
#include <errno.h>

#define COUNT_CONFIGTABLE_BLOB "SELECT count(*) FROM sqlite_master WHERE name='config_blob';"

#define CREATE_CONFIGTABLE_BLOB "CREATE TABLE config_blob(key TEXT primary key, value BLOB);"

static inline std::string &ltrim(std::string &input)
{
	input.erase(input.begin(), std::find_if(input.begin(), input.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return input;
}

static inline std::string &rtrim(std::string &input)
{
	input.erase(std::find_if(input.rbegin(), input.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), input.end());
	return input;
}

static inline std::string &trim(std::string &input)
{
	return ltrim(rtrim(input));
}

bool readFile(const char* file, char *buff, size_t size)
{
	if (!file || !buff)
		return false;
	
	FILE* fh = fopen(file, "r");
	
	if (!fh)
		return false;
		
	fseek(fh, 0, SEEK_END);
	size_t s = ftell(fh);
	
	if (s > size)
		s = size;
	
	fseek(fh, 0, SEEK_SET);
	size_t n = fread(buff, s, 1, fh);
	
	fclose(fh);
	return (n == 1);
}



/*class ExeDir
{
public:
	ExeDir()
	{
		char result[PATH_MAX] = {0};
		ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);

		if (count <= 0)
		{
			ERROR_OUTPUT("Could not get program directory!");
			return;
		}

		for(size_t i = count - 1; i > 0; i--)
		{
			if(result[i] == '/')
			{
				result[i] = '\0';
				break;
			}
		}

		path = result;
	}

	gcWString path;
};*/

namespace UTIL
{
namespace LIN
{
#include <cstdio>

// Go to UtilLinux.h for documentation on this function
std::string getExecuteDir(void) {
	
	// Read path the link /proc/self/exe links to
	// We are making the array a size of PATH_MAX, which is the maximum amount of bytes a Linux path can be
	char result[PATH_MAX];
	//std::cout << "Size of result: " << PATH_MAX << std::endl;
	ssize_t amountRead = readlink("/proc/self/exe", (char *)result, PATH_MAX);
	//std::cout << "amountRead = " << amountRead;

	if(amountRead == -1) {
		ERROR_OUTPUT("Failed to read /proc/self/exe!");
		return "";
	}
	
	// Convert result to a std::string
	std::string r(result, amountRead);
	// Remove anything before the last /
	r.resize(r.find_last_of('/'));
	// Add the null byte
	r.push_back('\0');

	return r;
}

// This is copied here due to bootloader not having util::fs (due to boost dep)
std::string expandPath(const char* file)
{
	if (!file)
		return "";
	
	std::string ret;

	wordexp_t exp_result;
	memset(&exp_result, 0, sizeof(wordexp_t));

	wordexp(file, &exp_result, 0);
	
	if (exp_result.we_wordv[0])
		ret = exp_result.we_wordv[0];

	wordfree(&exp_result);

	if (ret == "")
		ret = file;

	return ret;
}

bool is64OS()
{
#ifndef DEBUG
	std::string out(getCmdStdout("uname -m", 1));
	if(out == "x86_64")
		return true;
	else
		return false;
#else
	#ifdef NIX64
	return true;
	#else
	return false;
	#endif
#endif
}

std::wstring getAppPath(std::wstring extra)
{
	std::string dir = getExecuteDir();

	gcWString dirW(UTIL::STRING::toWStr(dir));

	// Convert std::string to std::wstring using std::copy
	/*std::wstring dirW(dir.length(),L' ');
	std::copy(dir.begin(), dir.end(), dirW.begin());*/

	if (extra.size() > 0)
	{
		dirW += DIRS_WSTR;
		dirW += extra;
	}

	return dirW;
}

void setConfigValue(const std::string &configKey, const std::string &value)
{
	UTIL::UNIX::setConfigValue(configKey, value);
}

void setConfigValue(const std::string &configKey, uint32 value)
{
	UTIL::UNIX::setConfigValue(configKey, value);
}

std::string getConfigValue(const std::string &configKey)
{
	return UTIL::UNIX::getConfigValue(configKey);
}

uint64 getFreeSpace(const char* path)
{
	if (!path)
		return -1;
	
	UTIL::FS::Path p(path, "", false);
	
	std::vector<std::string> folders;
	
	for (size_t x=0; x<p.getFolderCount(); x++)
		folders.push_back(p.getFolder(x));
		
	struct statvfs fsinfo;
		
	for (size_t x=1; x<=folders.size(); x++)
	{
		UTIL::FS::Path fullP;
		fullP.m_absolutePath = p.m_absolutePath;

		for (size_t y=0; y<folders.size()-x; y++)
			fullP += folders[y];
	
		std::string szPath = fullP.getFullPath();
	
		if(statvfs(szPath.c_str(), &fsinfo) != 0)
			continue;
	
		return (fsinfo.f_bsize * fsinfo.f_bavail); 
	}
	
	char buffer[1024];
	snprintf(buffer, 1024, "Free space for '%s' could not be determined!", path);
	ERROR_OUTPUT(buffer);
	return -1;
}


static void ConvertToArgs(const char* args, std::vector<std::string> &argv)
{
	size_t argLen = strlen(args);

	if (argLen > 0)
	{
		size_t count = 0;

		bool bSQuote = false;
		bool bDQuote = false;

		for (size_t x=0; x<argLen; x++)
		{
			if (args[x] == '\'')
				bSQuote = !bSQuote;

			if (args[x] == '"')
				bDQuote = !bDQuote;

			if (!bSQuote && !bDQuote && args[x] == ' ')
				count++;
		}

		size_t lastIndex = 0;

		for (size_t x=0; x<=argLen; x++)
		{
			if (args[x] == '\'')
				bSQuote = !bSQuote;

			if (args[x] == '"')
				bDQuote = !bDQuote;

			if (!bSQuote && !bDQuote && (args[x] == ' ' || args[x] == '\0'))
			{
				size_t end = x-1;

				while ((args[lastIndex] == '\'' && args[end] == '\'') || (args[lastIndex] == '"' && args[end] == '"'))
				{
					lastIndex++;
					end--;
				}

				if (end < lastIndex)
					continue;

				size_t len = end-lastIndex+1;

				std::string str(args+lastIndex, len);
				lastIndex = x+1;

				argv.push_back(str);
			}
		}
	}
}

bool launchProcessXDG(const char* exe, const char* libPath)
{
	if (!exe)
		return false;

	ERROR_OUTPUT(__func__);
	std::string fullExe = expandPath(exe);
	
	pid_t id = fork();

	if (id != 0)
	{
		int status;
		waitpid(id, &status, 0);
		
		if (WEXITSTATUS(status) == 0)
			return true;
		else
			return false;
	}

	char* old_lc_all = NULL;
	old_lc_all = getenv("OLD_LC_ALL");
	if (old_lc_all)
		setenv("LC_ALL", old_lc_all, 1);
	else
		setenv("LC_ALL", "", 1);
	
	if (libPath)
		setenv("LD_LIBRARY_PATH", libPath, 1);
	else
		unsetenv("LD_LIBRARY_PATH");
	
	std::string wd = UTIL::FS::Path(fullExe.c_str(), "", true).getFolderPath();
	(void)chdir(wd.c_str());

	ERROR_OUTPUT("Launching with xdg-open");
	execlp("xdg-open", "xdg-open", fullExe.c_str(), NULL);
	
	//um shit. We shouldnt be here. :(
	printf("Failed to exec xdg-open for %s. Error: %d\n", fullExe.c_str(), errno);
	exit(-1);
	return false;
}

bool launchFolder(const char* path)
{
	if (!path)
		return false;

	ERROR_OUTPUT(__func__);
	std::string fullPath = expandPath(path);

	pid_t id = fork();

	if (id != 0)
	{
		int status;
		waitpid(id, &status, 0);
		
		if (WEXITSTATUS(status) == 0)
			return true;
		else
			return false;
	}

	setenv("LD_LIBRARY_PATH", "", 1);

	execlp("xdg-open", "xdg-open", fullPath.c_str(), NULL);
	
	//um shit. We shouldnt be here. :(
	printf("Failed to execlp %s. Error: %d\n", fullPath.c_str(), errno);
	exit(-1);
	return false;
}

bool launchProcess(const char* exe, const std::map<std::string, std::string> &info)
{
	if (!exe)
		return false;
		
	ERROR_OUTPUT(__func__);
	std::string fullExe = expandPath(exe);
	
	if (!fileExists(fullExe.c_str()))
		return false;	
	
	//we double fork so that the new process is not a child of this process
	pid_t pid = fork();
	
	if (pid != 0)
	{
		int status;
		waitpid(pid, &status, 0);
		
		if (WEXITSTATUS(status) == 0)
			return true;
		else
			return false;
	}
	
	UTIL::FS::Path path(fullExe.c_str(), "", true);
	
	std::string workingDir;
	std::string libPath;
	std::string args;
	std::string e = path.getFullPath();	
	
	typedef const std::map<std::string, std::string>::const_iterator MapIterator;

	MapIterator &wd = info.find("wd");
	if (wd != info.end())
		workingDir = expandPath( (*wd).second.c_str() );
	
	if (workingDir == "")
		workingDir = path.getFolderPath();

	MapIterator &lp = info.find("lp");
	if (lp != info.end())
		libPath = (*lp).second.c_str();
	
	MapIterator &cla = info.find("cla");
	if (cla != info.end())
		args = (*cla).second.c_str();
	
	
	gcString orgLibPath = getenv("OLD_LIB_PATH");
	
	if (orgLibPath.size() > 0)
	{
		if (libPath.size() > 0)
			libPath += ":";
			
		libPath += orgLibPath;
	}
	
	if (libPath.size() > 0)
		setenv("LD_LIBRARY_PATH", libPath.c_str(), 1);
	else
		unsetenv("LD_LIBRARY_PATH");
	

	std::vector<std::string> out;
	const char* argv[10] = {0};
	argv[0] = e.c_str();

	if (args.size() > 0)
	{
		ConvertToArgs(args.c_str(), out);

		for (size_t x=0; x<out.size(); x++)
		{
			if (x == 9)
				break;
			
			argv[x+1] = out[x].c_str();
		}
	}

	(void)chdir(workingDir.c_str());
	execv(fullExe.c_str(), (char* const*)argv);
	
	printf("Failed to execl %s [%s] error: %d\n", fullExe.c_str(), args.c_str(), errno);
	exit(-1);

	return false;
}

static std::string getDescFromLSB(std::string &input)
{
	std::vector<std::string> tokens;
	std::vector<std::string>::iterator itr;
	
	UTIL::STRING::tokenize(input, tokens, "\n");
	
	for (itr = tokens.begin(); itr < tokens.end(); ++itr)
	{
		std::vector<std::string> subtokens;
		UTIL::STRING::tokenize(*itr, subtokens, ":");
		if (subtokens.size() > 1)
			if (trim(subtokens[0]) == "Description")
				return trim(subtokens[1]);
	}
	
	return std::string("");
}



std::string getOSString()
{	
	std::string distro, arch;

#ifdef DEBUG
	distro = "Unknown (debug build)";
	arch = "XXX";
#else
	std::string lsbInfo = getCmdStdout("lsb_release -a", 2);
	
	arch = getCmdStdout("uname -m", 1);
	
	if (lsbInfo.size() > 0)
		distro = getDescFromLSB(lsbInfo);
	
	char outBuff[255] = {0};
	
	if (distro.size() == 0 && (readFile("/etc/fedora-release", outBuff, 254) || readFile("/etc/redhat-release", outBuff, 254) || readFile("/etc/gentoo-release", outBuff, 254)))
		distro = outBuff;
		
	if (distro.size() == 0)
		distro = "Unknown (indeterminate)";
#endif
	
	return distro.insert(0, "Linux ").append(" ").append(arch);
}

std::string getCmdStdout(const char* command, int stdErrDest)
{
	ERROR_OUTPUT("\ngetCmdStdout() is being called from inside a debug build. This should be avoided as GDB promptly packs it in. See appstaff ticket #711 for more information\n");

	if (!command)
		return "No command - error";

	std::string newCommand(command);
	
	if (stdErrDest == 1) // suppress it
		newCommand += " 2>/dev/null";
	else if (stdErrDest == 2) // redir to stdout
		newCommand += " 2>&1";

	FILE *fd = popen(newCommand.c_str(), "r");
	
	if (!fd)
	{
		ERROR_OUTPUT(gcString("Failed to run command: [{0}]\n", command).c_str());
		return "Failed to run - error";
	}

	std::string output = "";
	const unsigned int size = 512;
	char buffer[size];
	
	while (fgets(buffer, size, fd) != NULL)
		output.append(buffer);

	pclose(fd);
	return trim(output); 
}

std::wstring getDesktopPath(std::wstring extra)
{
	// Convert std::string to std::wstring using std::copy
	std::string temp(getCmdStdout("xdg-user-dir DESKTOP", 1));
	std::wstring desktop(temp.length(),L' ');
	std::copy(temp.begin(), temp.end(), desktop.begin());
	//std::wstring desktop((wchar_t*) getCmdStdout("xdg-user-dir DESKTOP", 1).c_str());
	if(!desktop.empty())
	{
		desktop += L"/";
		desktop += extra;
	}
	// Do some error checking if desktop is empty
	return desktop;
}

std::wstring getApplicationsPath(std::wstring extra)
{
	std::wstring data_home(UTIL::STRING::toWStr(getenv("XDG_DATA_HOME")));
	data_home += L"/applications/";
	data_home += L"/";
	data_home += extra;

	return data_home;
}

bool fileExists(const char* file) 
{
	char buffer[PATH_MAX];
	snprintf(buffer, PATH_MAX, "%s (%s)", __func__, file);
	ERROR_OUTPUT(buffer);
	
	std::string fullFile = expandPath(file);

	struct stat stFileInfo;
	int intStat;

	// Attempt to get the file attributes
	intStat = stat(fullFile.c_str(), &stFileInfo);

	if (intStat == 0) 
		return true;
	else 
		return false;
}

const char g_cBadChars[] = {
	'\\',
	'/',
	':',
	'*',
	'?',
	'"',
	'<',
	'>',
	'|',
	'%',
	'"',
	NULL
};

gcString getAbsPath(const gcString& path)
{
	if (path.size() == 0 || path[0] == '/')
		return path;

	gcString wd = UTIL::LIN::getAppPath(L"");

	if (path.find(wd) == std::string::npos)
		return wd + "/" + path;

	return path;
}

gcString getRelativePath(const gcString &path)
{
	gcString wd = UTIL::LIN::getAppPath(L"");

	if (path.find(wd) == 0)
		return path.substr(wd.size()+1, std::string::npos);

	return path;
}

std::string sanitiseFileName(const char* name)
{
	if (!name)
		return "";

	std::string out;
	size_t size = strlen(name);
	out.reserve(size);

	for (size_t x=0; x<size; x++)
	{
		size_t y=0;
		bool bad = false;

		while (g_cBadChars[y])
		{
			if (g_cBadChars[y] == name[x])
			{
				bad = true;
				break;
			}

			y++;
		}

		if (!bad)
			out.push_back(name[x]);
	}

	//remove any spaces on the end
	while (out.size() > 0 && out[out.size()-1] == ' ')
		out.erase(out.end()-1);

	return out;
}

void setupXDGVars()
{
	gcString homeDir(getenv("HOME")); // Used for falling back.
	gcString configDir(getenv("XDG_CONFIG_HOME"));
	gcString cacheDir(getenv("XDG_CACHE_HOME"));
	
	if (homeDir.empty())
	{
		// Below we just use 'falling back' and don't note we're setting it,
		// but as $HOME is referenced later on it might confuse anybody reading the
		// logs.
		// Also use 'passwd' instead of 'password' as that may freak out some users.
		printf("$HOME not set, temporarily setting it to the user's passwd entry.\n");
		
		struct passwd* pass = getpwuid(getuid());
		homeDir = pass->pw_dir;
	}
	
	if (configDir.empty())
	{
		printf("$XDG_CONFIG_HOME not set, falling back to $HOME/.config.\n");
		configDir = homeDir + "/.config";
	}
	
	if (cacheDir.empty())
	{
		printf("$XDG_CACHE_HOME not set, falling back to $HOME/.cache.\n");
		cacheDir = homeDir + "/.cache";
	}
	
	setenv("HOME", homeDir.c_str(), 1);
	setenv("XDG_CONFIG_HOME", configDir.c_str(), 1);
	setenv("XDG_CACHE_HOME", cacheDir.c_str(), 1);
}

bool canLaunchBinary(OS::BinType type)
{
	if(type == OS::BinType::ELF32
	#ifdef NIX64
		|| type == OS::BinType::ELF64
	#endif
		|| type == OS::BinType::SH)
		return true;
	return false;
}

}
}
