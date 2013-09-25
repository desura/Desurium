/*
Desura is the leading indie game distribution platform
Copyright (C) 2013 Mark Chandler (Desura Net Pty Ltd)

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


//Boost is like a virus, used once, infects every where. Sqlite3x has no reason to depend on boost as it can do this simple function it self. :(

#ifndef __BOOST_FAKE_UTILITY_HPP__
#define __BOOST_FAKE_UTILITY_HPP__

namespace boost
{
	class noncopyable
	{
	public:
		noncopyable()
		{
		}
		
	private:
		noncopyable(const noncopyable& obj)
		{
		}
		
		noncopyable& operator=(const noncopyable& obj)
		{
			return *this;
		}
	};
}

#endif