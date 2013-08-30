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
#include "Theme.h"
#include "XMLMacros.h"

Theme::Theme(const char* name) : ControlList(true), ImageList(true), SpriteList(true), WebList(true),
	m_szName(name)
{}

const char* Theme::getImage(const char* id)
{
	ThemeImageInfo* img = ImageList::findItem(id);

	if (img)
		return img->path.c_str();

	return NULL;
}

const char* Theme::getWebPage(const char* id)
{
	ThemeWebInfo* web = WebList::findItem(id);

	if (web)
		return web->path.c_str();

	return NULL;
}

Color Theme::getColor(const char* name, const char* id)
{
	ThemeControlInfo* col = ControlList::findItem(name);

	if (col)
		return col->getColor(name, id);

	if (strcmp(name, "default")!=0)
	{
		Warning(gcString("Cant find color {0} for control {1}, getting default\n", id, name));
		return getColor("default", id);
	}

	Warning("Cant find default color.\n");
	return Color();
}


SpriteRect* Theme::getSpriteRect(const char* id, const char* rectId)
{
	ThemeSpriteInfo* sprite = SpriteList::findItem(id);

	if (!sprite)
		return NULL;

	return sprite->findItem(rectId);
}

void Theme::parseFile(const char* file)
{
	tinyxml2::XMLDocument doc;

	doc.LoadFile(file);

	tinyxml2::XMLElement *mcNode = doc.FirstChildElement("theme");

	if (!mcNode)
		throw gcException(ERR_XML_NOPRIMENODE);

	XML::GetChild("creator", m_szDev, mcNode);
	XML::GetChild("name", m_szPName, mcNode);

	UTIL::FS::Path path(file, "", true);

	tinyxml2::XMLElement *cNode = mcNode->FirstChildElement("images");
	if (cNode)
	{
		tinyxml2::XMLNode* pChild = cNode->FirstChild();

		while (pChild)
		{
			if (!XML::isValidElement(pChild))
			{
				pChild = pChild->NextSibling();
				continue;
			}

			const char* name = pChild->ToElement()->Attribute("name");
			const char* val = pChild->ToElement()->GetText();

			if (name && val)
			{
				std::string outVal = UTIL::STRING::sanitizeFileName(val);
				gcString fullPath("{0}{2}images{2}app{2}{1}", path.getFolderPath(), outVal, DIRS_STR);
				ThemeImageInfo* img = ImageList::findItem(name);

				if (!img)
				{
					img = new ThemeImageInfo(name);
					ImageList::addItem(img);
				}

				img->path = fullPath;
			}

			pChild = pChild->NextSibling();
		}
	}


	cNode = mcNode->FirstChildElement("web");
	if (cNode)
	{
		tinyxml2::XMLNode* pChild = cNode->FirstChild();

		gcString urlPath(path.getFolderPath());

		for (size_t x=0; x<urlPath.size(); x++)
		{
			if (urlPath[x] == '\\')
				urlPath[x] = '/';
		}

		while (pChild)
		{
			if (!XML::isValidElement(pChild))
			{
				pChild = pChild->NextSibling();
				continue;
			}

			const char* name = pChild->ToElement()->Attribute("name");
			const char* val= pChild->ToElement()->GetText();

			if (name && val)
			{
				std::string outVal = UTIL::STRING::sanitizeFileName(val);

#ifdef WIN32
				gcString fullPath("file:///{0}/html/{1}", urlPath, outVal);
#else
				gcString fullPath("file://{0}/html/{1}", urlPath, outVal);
#endif

				ThemeWebInfo* web = WebList::findItem(name);

				if (!web)
				{
					web = new ThemeWebInfo(name);
					WebList::addItem(web);
				}

				web->path = fullPath;
			}

			pChild = pChild->NextSibling();
		}
	}


	cNode = mcNode->FirstChildElement("sprites");
	if (cNode)
	{
		tinyxml2::XMLNode* pChild = cNode->FirstChild();

		while (pChild)
		{
			if (!XML::isValidElement(pChild))
			{
				pChild = pChild->NextSibling();
				continue;
			}

			const char* name = pChild->ToElement()->Attribute("name");
			
			if (!name)
			{
				pChild = pChild->NextSibling();
				continue;
			}

			ThemeSpriteInfo* sprite = SpriteList::findItem(name);

			if (!sprite)
			{
				sprite = new ThemeSpriteInfo(name);
				SpriteList::addItem( sprite );
			}

			tinyxml2::XMLNode* rNode = pChild->FirstChild();

			while (rNode)
			{
				if (!XML::isValidElement(rNode))
				{
					rNode = rNode->NextSibling();
					continue;
				}

				const char* rName = rNode->ToElement()->Attribute("name");
				tinyxml2::XMLElement* pos = rNode->FirstChildElement("pos");
				tinyxml2::XMLElement* size = rNode->FirstChildElement("size");

				if (!rName || !XML::isValidElement(pos) || !XML::isValidElement(size))
				{
					rNode = rNode->NextSibling();
					continue;
				}

				const char* x = pos->ToElement()->Attribute("x");
				const char* y = pos->ToElement()->Attribute("y");

				const char* w = size->ToElement()->Attribute("w");
				const char* h = size->ToElement()->Attribute("h");

				if (!(x && y && w && h))
				{
					rNode = rNode->NextSibling();
					continue;
				}

				SpriteRect* rect = sprite->findItem(rName);
		
				if (!rect)
				{
					rect = new SpriteRect(rName);
					sprite->addItem(rect);
				}

				rect->x = atoi(x);
				rect->y = atoi(y);
				rect->w = atoi(w);
				rect->h = atoi(h);

				rNode = rNode->NextSibling();
			}

			pChild = pChild->NextSibling();
		}
	}



	cNode = mcNode->FirstChildElement("controls");
	if (cNode)
	{
		tinyxml2::XMLNode* pChild = cNode->FirstChild();

		while (pChild)
		{
			if (!XML::isValidElement(pChild))
			{
				pChild = pChild->NextSibling();
				continue;
			}
		

			const char* name = pChild->ToElement()->Attribute("name");
			tinyxml2::XMLElement* colNode = pChild->FirstChildElement("color");

			if (name && colNode)
			{
				ThemeControlInfo* control = ControlList::findItem(name);

				if (!control)
				{
					control = new ThemeControlInfo(name);
					ControlList::addItem(control);
				}

				while (colNode)
				{
					if (!XML::isValidElement(colNode))
					{
						colNode = colNode->NextSiblingElement();
						continue;
					}

					const char* id = colNode->ToElement()->Attribute("id");
					const char* val = colNode->ToElement()->GetText();

					if (id && val)
					{
						ThemeColorInfo* col = control->findItem(id);

						if (!col)
						{
							col = new ThemeColorInfo(id);
							control->add(col);
						}

						col->color = Color(val);
					}

					colNode = colNode->NextSiblingElement();
				}
			}
			pChild = pChild->NextSibling();
		}
	}

	ThemeControlInfo* control = ControlList::findItem("default");

	if (!control)
	{
		control = new ThemeControlInfo("default");

		ThemeColorInfo* col1 = new ThemeColorInfo("bg");
		ThemeColorInfo* col2 = new ThemeColorInfo("fg");

		col1->color = Color(0);
		col2->color = Color(0xFFFFFF);

		control->add(col1);
		control->add(col2);

		ControlList::addItem( control );
	}
}



