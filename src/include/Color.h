/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>

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

#ifndef DESURA_COLOR_H
#define DESURA_COLOR_H

#include <string.h>

class Color
{
public:
	Color(uint8 r, uint8 g, uint8 b, uint8 a)
	{
		red = r;
		green = g;
		blue = b;
		alpha = a;
	}

	Color(uint8 r, uint8 g, uint8 b)
	{
		red = r;
		green = g;
		blue = b;
		alpha = 0;
	}

	Color(const Color *t)
	{
		if (t)
		{
			red = t->red;
			green = t->green;
			blue = t->blue;
			alpha = t->alpha;
		}
		else
		{
			red = 0;
			green = 0;
			blue = 0;
			alpha = 255;
		}
	}

	Color(const Color &t)
	{
		red = t.red;
		green = t.green;
		blue = t.blue;
		alpha = t.alpha;
	}

	Color(int val)
	{
		red		= (val>>24)&0xFFFFFF;
		green	= (val>>16)&0xFFFFFF;
		blue	= (val>> 8)&0xFFFFFF;
		alpha	= (val>> 0)&0xFFFFFF;
	}

	Color(const char* text)
	{
		red = 0;
		green = 0;
		blue = 0;
		alpha = 255;

		if (!text)
			return;

		uint32 textlen = (uint32) strlen(text);

		if (textlen == 0)
		{
		}
		else if (text[0] == '#')
		{
			uint32 round = 0;
			uint32 count = 1;
			char hex[3];

			while (count < textlen)
			{
				hex[0] = text[count];
				count++;
				hex[1] = text[count];
				count++;
				hex[2]='\0';

				uint8 val = UTIL::MISC::hextoDec(hex);

				switch (round)
				{
					case 0: red = val;
						break;
					case 1: green = val;
						break;
					case 2: blue = val;
						break;
					case 3: alpha = val;
						break;
					default: count = textlen;
						break;
				}
				round++;
			}
		}
		else
		{
			red = atoi(text);
	
			char textcpy[255] = {0};
#ifdef WIN32
			strcpy_s(textcpy, 255, text);
#else
			strcpy(textcpy, text);
#endif

			char* g = strstr(textcpy+1, " ");
			if (g)
			{
				green = atoi(g);

				char* b = strstr(g+1, " ");
				if (b)
				{
					blue = atoi(b);

					char* a = strstr(b+1, " ");
					if (a)
						alpha = atoi(a);
				}
			}
		}
	}

	Color()
	{
		Color(0,0,0,0);
	}

	uint8 red;
	uint8 green;
	uint8 blue;
	uint8 alpha;

	unsigned int getColor(){return (red<<24) + (green<<16) + (blue<<8) + alpha;}

	//wxWidgets color
	operator unsigned long()
	{
		return (blue<<16) + (green<<8) + red;
	}
};


#endif
