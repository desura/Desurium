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

#ifndef DESURA_DESURAID_H
#define DESURA_DESURAID_H
#ifdef _WIN32
#pragma once
#endif

// old id: 28 bits id, 4 bits type
// New id: 32 bits id, 12 bits reserved, 8 bits branch, 8 bits type, 4 bits (old id)

enum OLD_TYPES
{
	O_TYPE_VOID=0,
	O_TYPE_MOD,
	O_TYPE_GAME,
	O_TYPE_ADDON,
	O_TYPE_TOOL,
	O_TYPE_LINK,
	O_TYPE_GROUP,
	O_TYPE_MEMBER,
	O_TYPE_APP, //must stay as 0x08
	O_TYPE_MADDON,
	O_TYPE_THEME,
	O_TYPE_UNKNOWN=0xF,
};


class DesuraId
{
public:
	DesuraId()
	{
		m_uiItemId = 0;
		m_uiType = TYPE_NONE;
	}

	DesuraId(uint64 id)
	{
		if ((id&0xF) != 0)
		{
			m_uiItemId = (id>>4)&0xFFFFFFFF;

			switch (id&0xF)
			{
				case O_TYPE_MOD:	m_uiType = TYPE_MOD; break;
				case O_TYPE_GAME:	m_uiType = TYPE_GAME; break;
				case O_TYPE_APP:	m_uiType = TYPE_APP; break;
				default: m_uiType = TYPE_NONE;
			};
		}
		else
		{
			m_uiItemId = (id>>32)&0xFFFFFFFF;
			m_uiType = (id>>4)&0xFF;
		}
	}

	DesuraId(uint32 id, uint8 type)
	{
		m_uiItemId = id;
		m_uiType = type;
	}

	DesuraId(const char* id, const char* type)
	{
		m_uiItemId = 0;
		m_uiType = TYPE_NONE;

		if (id)
			m_uiItemId = atoi(id);


		if (type && strcmp(type, "mods")==0)
		{
			m_uiType = TYPE_MOD;
		}
		else if (type && strcmp(type, "games")==0)
		{
			m_uiType = TYPE_GAME;
		}
		else if (type && strcmp(type, "tools")==0)
		{
			m_uiType = TYPE_TOOL;
		}
		else if (type && strcmp(type, "links")==0)
		{
			m_uiType = TYPE_LINK;
		}
	}

	uint32 getItem() const 
	{
		return m_uiItemId;
	}

	uint8 getType() const 
	{
		return m_uiType;
	}

	static gcString getTypeString(uint8 type)
	{
		switch (type)
		{
		case TYPE_MOD: return gcString("mods");
		case TYPE_GAME: return gcString("games");
		case TYPE_TOOL: return gcString("tools");
		case TYPE_LINK: return gcString("links");
		};

		return gcString("void");
	}

	gcString getTypeString() const
	{
		return getTypeString(m_uiType);
	}

	uint64 toInt64() const
	{
		return (((uint64)m_uiItemId)<<32) + (m_uiType<<4);
	}

	bool isOk() const
	{
		return (m_uiItemId != 0 && m_uiType != TYPE_NONE);
	}

	gcString getFolderPathExtension(const char* extra = NULL)
	{
		gcString res;
		gcString type = getTypeString();

		if (extra)
			res = gcString("{0}{3}{1}{3}{2}", type, m_uiItemId, extra, DIRS_STR);
		else
			res = gcString("{0}{2}{1}", type, m_uiItemId, DIRS_STR);

		return res;
	}

	DesuraId& operator=(const DesuraId &rhs)
	{
		m_uiItemId = rhs.getItem();
		m_uiType = rhs.getType();

		return *this;
	}

	bool operator==(const DesuraId &rhs) const 
	{
		return (m_uiItemId == rhs.getItem() && m_uiType == rhs.getType());
	}

	bool operator!=(const DesuraId &rhs) const 
	{
		return !(*this == rhs);
	}

	enum NEW_TYPES
	{
		TYPE_NONE = 0,
		TYPE_MOD,
		TYPE_GAME,
		TYPE_APP,
		TYPE_TOOL,
		TYPE_LINK,
	};

	gcString toString()
	{
		return gcString("{0}", toInt64());
	}

private:
	uint32 m_uiItemId;
	uint8 m_uiType;
};

#endif //DESURA_DESURAID_H
