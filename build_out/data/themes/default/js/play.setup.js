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

var jQ = $;

var listTimeout;
var searchCache = "";
var sortFieldCache = desura.utils.getCacheValue('LastSortField', 'getName');
var sortDirCache = desura.utils.getCacheValue('LastSortDir', 'asc');
var tabCache = desura.utils.getCacheValue('LastActiveTab', 'game');
var expandCache = JSON.parse(desura.utils.getCacheValue('LastActiveExpand', '{}'));
var playCache = JSON.parse(desura.utils.getCacheValue('LastActivePlay', '{}'));
var overlayPlay = false;
var overlayProperties = false;
var dateCache = 0;
var keepon = true;

expandCache = null !== expandCache && typeof(expandCache) == "object" ? expandCache : new Object;
playCache = null !== playCache && typeof(playCache) == "object" ? playCache : new Object;
