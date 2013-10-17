/*
Desura is the leading indie game distribution platform
Copyright (C) Karol Herbst <git@karolherbst.de>

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

template <typename T>
bool isInVector(const T&, const std::vector<T>&);

#include <iostream>

template <typename T>
bool isInVector(const T& t, const std::vector<T>& vector)
{
	bool b = false;
	std::for_each(vector.begin(), vector.end(), [&b, &t](const T& i)
	{
		if (i == t)
		{
			b = true;
			return;
		}
	});
	return b;
}
