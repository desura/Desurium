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
#include "WebCore.h"
#include "XMLMacros.h"

#include "BZip2.h"
#include "sqlite3x.hpp"
#include "sql/WebCoreSql.h"

#include "mcfcore/DownloadProvider.h"
#include "webcore/DownloadImageInfo.h"




namespace WebCore
{




extern gcString genUserAgent();

void WebCoreClass::sendPassReminder(const char* email)
{
	if (!email)
		throw gcException(ERR_INVALIDDATA);

	HttpHandle hh(getPassWordReminderUrl().c_str());

	gcString useragent = genUserAgent();
	
	if (useragent.size() > 0)
		hh->setUserAgent(useragent.c_str());

	hh->addPostText("emailreset", email);
	hh->postWeb();

	if (hh->getDataSize() == 0)
		throw gcException(ERR_BADRESPONSE, "Data size was zero");

	TiXmlDocument doc;
	XML::loadBuffer(doc, const_cast<char*>(hh->getData()), hh->getDataSize());

	TiXmlNode *uNode = doc.FirstChild("memberpasswordreminder");

	if (!uNode)
		throw gcException(ERR_BADXML, "Missing the root node");

	XML::processStatus(doc, "memberpasswordreminder");
}

void WebCoreClass::getInstalledItemList(TiXmlDocument &doc)
{
	HttpHandle hh(getInstalledWizardUrl().c_str());
	setWCCookies(hh);
	hh->getWeb();
	
	if (hh->getDataSize() == 0)
		throw gcException(ERR_BADRESPONSE);

	if (strncmp(hh->getData(), "BZh", 3)==0)
	{
		uint32 bufSize = hh->getDataSize() *25;

		UTIL::MISC::Buffer buff(bufSize);
		UTIL::BZIP::BZ2DBuff(buff, &bufSize, const_cast<char*>(hh->getData()), hh->getDataSize());

		XML::loadBuffer(doc, buff, bufSize);
	}
	else
	{
		XML::loadBuffer(doc, const_cast<char*>(hh->getData()), hh->getDataSize());
	}

	XML::processStatus(doc, "itemwizard");
}

void WebCoreClass::onHttpProg(volatile bool& stop, Prog_s& prog)
{
	prog.abort = stop;
}

void WebCoreClass::downloadImage(WebCore::Misc::DownloadImageInfo *dii, volatile bool &stop)
{
	if (!dii)
		throw gcException(ERR_BADITEM);

	const char* imageUrl = dii->url.c_str();
	uint32 hash = UTIL::MISC::RSHash_CSTR(imageUrl);

	std::string szPath = m_ImageCache.getImagePath(hash);

	if (UTIL::FS::isValidFile(szPath))
	{
		dii->outPath = szPath;
		return;
	}

	if (strncmp(imageUrl,"http://", 7) != 0)
		throw gcException(ERR_INVALIDDATA, gcString("The url {0} is not a valid http url", imageUrl));

	gcString appDataPath = UTIL::OS::getCachePath(L"images");
	gcString fileName = UTIL::FS::Path(imageUrl, "", true).getFile().getFile();

	if (dii->id.isOk())
		fileName = gcString("{0}_{1}", dii->id.toInt64(), fileName);
	else
		fileName = gcString("{0}_{1}", hash, fileName);

	HttpHandle hh(imageUrl);

	hh->setUserAgent(getUserAgent());
	hh->getProgressEvent() += extraDelegate<WebCoreClass, Prog_s, volatile bool&>(this, &WebCoreClass::onHttpProg, stop);

	hh->getWeb();
		
	if (hh->getDataSize() == 0)
		throw gcException(ERR_BADRESPONSE);

	if (UTIL::MISC::isValidImage((const unsigned char*)hh->getData()) == IMAGE_VOID)
		throw gcException(ERR_INVALIDDATA, gcString("The url {0} is not a valid image format", imageUrl));

	UTIL::FS::Path path(appDataPath, fileName, false);

	UTIL::FS::recMakeFolder(path);
	UTIL::FS::FileHandle fh(path, UTIL::FS::FILE_WRITE);

	fh.write(hh->getData(), hh->getDataSize());
			
	dii->outPath = path.getFullPath();
	m_ImageCache.updateImagePath(dii->outPath.c_str(), hash);
}

void WebCoreClass::downloadBanner(MCFCore::Misc::DownloadProvider* dlp, const char* saveFolder)
{
	if (!dlp)
		throw gcException(ERR_BADITEM);

	WebCore::Misc::DownloadImageInfo dii(dlp->getBanner());

	bool stop = false;
	downloadImage(&dii, stop);
	dlp->setBanner(dii.outPath.c_str());
}


}
