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
var desura = desura || {}; 

desura.crumbs = desura.crumbs || {};
desura.internal = desura.internal || {};

(function(){

	desura.crumbs.addCrumb = function(name, url){
		native function addCrumb(); 
		addCrumb(name, url);
	};
	
	desura.crumbs.clearCrumbs = function(){
		native function clearCrumbs(); 
		clearCrumbs();
	};

	desura.internal.checkOldCrumbs = function(){
		if (typeof getBreadCrumbs == 'function')
		{
			desura.crumbs.clearCrumbs();
			var res = getBreadCrumbs().split("|=|");
			
			for (var i=0; i<res.length; i++){
				var crumb = res[i].split("|:|");
				
				if (crumb.length == 2) {
					desura.crumbs.addCrumb(crumb[0], crumb[1]);
				}
			}
		}
	};
	
	desura.internal.getUpdateCounts = function(){
		var msg = -1;
		var updates = -1;
		var threads = -1;
		var cart = -1;
	
		if (typeof getMessagesCount == 'function') {
			msg = getMessagesCount();
		}
		
		if (typeof getUpdatesCount == 'function') {
			updates = getUpdatesCount();
		}

		if (typeof getThreadwatchCount == 'function') {
			threads = getThreadwatchCount();
		}

		if (typeof getCartCount == 'function') {
			cart = getCartCount();
		}		
	
		native function updateCounts();
		updateCounts(msg, updates, threads, cart);
	};
	
})();
