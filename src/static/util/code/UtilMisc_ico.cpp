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
#include "util/UtilMisc.h"

//#include "FreeImage.h"
#include "SharedObjectLoader.h"

#pragma pack(push, 2)

typedef struct {
  WORD Reserved1;       // reserved, must be 0
  WORD ResourceType;    // type is 1 for icons
  WORD ImageCount;      // number of icons in structure (1)
  BYTE Width;           // icon width (32)
  BYTE Height;          // icon height (32)
  BYTE Colors;          // colors (0 means more than 8 bits per pixel)
  BYTE Reserved2;       // reserved, must be 0
  WORD Planes;          // color planes
  WORD BitsPerPixel;    // bit depth
  DWORD ImageSize;      // size of structure
  DWORD offset;			// offset
} GROUPICON;

#pragma pack(pop)

enum FREE_IMAGE_FORMAT
{
	FIF_UNKNOWN = -1,
	FIF_BMP		= 0,
	FIF_ICO		= 1,
};

enum FREE_IMAGE_FILTER
{
};

typedef unsigned char BYTE;
struct FIBITMAP 
{
	void* data;
};

typedef FREE_IMAGE_FORMAT	(__stdcall *FreeImage_GetFileTypeFn)			(const char *filename, int size);
typedef FREE_IMAGE_FORMAT	(__stdcall *FreeImage_GetFIFFromFilenameFn)		(const char *filename);
typedef BOOL				(__stdcall *FreeImage_FIFSupportsReadingFn)		(FREE_IMAGE_FORMAT fif);
typedef FIBITMAP*			(__stdcall *FreeImage_LoadFn)					(FREE_IMAGE_FORMAT fif, const char *filename, int flags);
typedef BOOL				(__stdcall *FreeImage_SaveFn)					(FREE_IMAGE_FORMAT fif, FIBITMAP *dib, const char *filename, int flags);
typedef void				(__stdcall *FreeImage_UnloadFn)					(FIBITMAP *dib);
typedef FIBITMAP*			(__stdcall *FreeImage_MakeThumbnailFn)			(FIBITMAP *dib, int dst_width, bool convert);


static QuickMutex g_FreeImageMutex;
static SharedObjectLoader g_ImgLib;

FreeImage_GetFileTypeFn			FreeImage_GetFileType;
FreeImage_GetFIFFromFilenameFn	FreeImage_GetFIFFromFilename;
FreeImage_FIFSupportsReadingFn	FreeImage_FIFSupportsReading;
FreeImage_SaveFn				FreeImage_Save;
FreeImage_LoadFn				FreeImage_Load;
FreeImage_UnloadFn				FreeImage_Unload;
FreeImage_MakeThumbnailFn		FreeImage_MakeThumbnail;

FIBITMAP* GenericLoader(const char* lpszPathName, int flag) 
{
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;

	// check the file signature and deduce its format
	// (the second argument is currently not used by FreeImage)
	fif = FreeImage_GetFileType(lpszPathName, 0);

	// no signature ?
	// try to guess the file format from the file extension
	if (fif == FIF_UNKNOWN) 
		fif = FreeImage_GetFIFFromFilename(lpszPathName);

	// check that the plugin has reading capabilities ...
	if((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) 
		return FreeImage_Load(fif, lpszPathName, flag);

	return NULL;
}





namespace UTIL
{
namespace MISC
{

bool loadImgLib()
{
	g_FreeImageMutex.lock();

	if (FreeImage_Unload)
	{
		g_FreeImageMutex.unlock();
		return true;
	}
		
	if (!g_ImgLib.load("FreeImage.dll"))
	{
		g_FreeImageMutex.unlock();
		return false;
	}

	FreeImage_Unload				= g_ImgLib.getFunction<FreeImage_UnloadFn>("_FreeImage_Unload@4");
	FreeImage_GetFileType			= g_ImgLib.getFunction<FreeImage_GetFileTypeFn>("_FreeImage_GetFileType@8");
	FreeImage_GetFIFFromFilename	= g_ImgLib.getFunction<FreeImage_GetFIFFromFilenameFn>("_FreeImage_GetFIFFromFilename@4");
	FreeImage_Load					= g_ImgLib.getFunction<FreeImage_LoadFn>("_FreeImage_Load@12");
	FreeImage_FIFSupportsReading	= g_ImgLib.getFunction<FreeImage_FIFSupportsReadingFn>("_FreeImage_FIFSupportsReading@4");
	FreeImage_Save					= g_ImgLib.getFunction<FreeImage_SaveFn>("_FreeImage_Save@16");
	FreeImage_MakeThumbnail			= g_ImgLib.getFunction<FreeImage_MakeThumbnailFn>("_FreeImage_MakeThumbnail@12");

	if (g_ImgLib.hasFailed())
	{
		FreeImage_Unload = NULL;
		g_FreeImageMutex.unlock();
		return false;
	}

	g_FreeImageMutex.unlock();
	return true;
}


bool convertToIco(const std::string &imgPath, const std::string &icoPath)
{
	if (!loadImgLib())
		return false;

	FIBITMAP* img = GenericLoader(imgPath.c_str(), 0);

	if (!img)
		return false;

	bool res = FreeImage_Save((FREE_IMAGE_FORMAT)1, img, icoPath.c_str(), 0)?true:false;
	FreeImage_Unload(img);

	return res;
}

bool convertToPng(const std::string &imgPath, const std::string &icoPath, size_t thumbnailSize)
{
	if (!loadImgLib())
		return false;

	FIBITMAP* img = GenericLoader(imgPath.c_str(), 0);

	if (!img)
		return false;

	if (thumbnailSize != 0)
	{
		FIBITMAP* tmp = img;
		img = FreeImage_MakeThumbnail(img, thumbnailSize, TRUE); //FILTER_BICUBIC

		if (!img)
			img = tmp;
		else
			FreeImage_Unload(tmp);
	}

	bool res = FreeImage_Save((FREE_IMAGE_FORMAT)13, img, icoPath.c_str(), 0)?true:false;
	FreeImage_Unload(img);
	return res;
}

}
}