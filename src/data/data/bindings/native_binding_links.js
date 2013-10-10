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


/**
 * @module desura
 */

var desura;
if (!desura)
    desura = {};

if (!desura.links)
    desura.links = {};
	
if (!desura.links.action)
    desura.links.action = {};

if (!desura.links.tab)
    desura.links.tab = {};
	
(function(){

	/**
	* Class that deals with internal links to allow desura to perform tasks
    * @class links
    */
	
	/**
	* Switches the currently active tab
	* @param {:api:`Tab <desura.links.tab>`} Id of the tab to switch to
	* @method switchTab
	*/		
	
	desura.links.switchTab = function(TabID){
		native function switchTab();
		return switchTab(TabID);
	};
	
	/**
	* Forces the update poll to run
	* @method internalLink
	* @param ID Item id
	* @param {:api:`Action <desura.links.action>`} Action to perform
	* @param String extraOne Optional pramater to pass in
	* @param String extraTwo Optional pramater to pass in
	*/		
	
	desura.links.internalLink = function(ID, Action, extraOne, extraTwo){
		native function internalLink();
		
		if (typeof(extraOne) == 'undefined')
			internalLink(ID, Action);
		else if (typeof(extraTwo) == 'undefined')
			internalLink(ID, Action, extraOne);
		else
			internalLink(ID, Action, extraOne, extraTwo);
	};

	
	/**
	* Gets the desura user id
	* @method loadUrl
	* @param String Url to load
	* @param {:api:`Tab <desura.links.tab>`} Id of tab to load url in (Note: ITEMS is an invalid option)
	*/		
	
	desura.links.loadUrl = function(url, TabID){
		native function loadUrl();
		return loadUrl(TabID, url);
	};
	
	
	/**
	* Tab names
    * @class links.tab
    */	
	
	/**
	* TabID: Play list tab
	* @property ITEMS
	* @final
	* @static
	*  @type TabID
	*/		

	desura.links.tab.ITEMS	= 0;	
	
	/**
	* TabID: Game tab
	* @property GAMES
	* @final
	* @static
	*  @type TabID
	*/		

	desura.links.tab.GAMES	= 1;

	/**
	* TabID: Mod tab
	* @property MODS
	* @final
	* @static
	*  @type TabID
	*/		

	desura.links.tab.MODS	= 2;

	/**
	* TabID: Community tab
	* @property COMMUNITY
	* @final
	* @static
	*  @type TabID
	*/		

	desura.links.tab.COMMUNITY	= 3;

	/**
	* TabID: Development tab
	* @property DEVELOPMENT
	* @final
	* @static
	*  @type TabID
	*/		

	desura.links.tab.DEVELOPMENT = 4;

	/**
	* TabID: Support tab
	* @property SUPPORT
	* @final
	* @static
	*  @type TabID
	*/		

	desura.links.tab.SUPPORT	= 5;	
	
	/**
	* Internal link actions
    * @class links.action
    */		

	/**
	* Action: Install an item (extraOne: branchId)
	* @property INSTALL
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.INSTALL	= 0;	

	/**
	* Action: Test install an item (extraOne: branch, extraTwo: build)
	* @property TEST
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.TEST	= 1;	
	
	/**
	* Action: Remove item
	* @property UNINSTALL
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.UNINSTALL	= 2;	

	/**
	* Action: Verify item (extraOne: show form ("true"|"false"))
	* @property VERIFY
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.VERIFY	= 3;	

	/**
	* Action: Launch an item
	* @property LAUNCH
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.LAUNCH	= 4;		
	
	/**
	* Action: New mcf upload
	* @property UPLOAD
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.UPLOAD	= 5;	

	/**
	* Action: Resume mcf upload (extraOne: Upload key)
	* @property RESUPLOAD
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.RESUPLOAD	= 6;	
	
	/**
	* Action: Create mcf
	* @property CREATE
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.CREATE	= 7;	

	/**
	* Action: Show item properties
	* @property PROPERTIES
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.PROPERTIES	= 8;	

	/**
	* Action: Show item dev profile
	* @property DEVPROFILE
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.DEVPROFILE	= 9;	

	/**
	* Action: Show item profile
	* @property PROFILE
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.PROFILE	= 10;	

	/**
	* Action: Show item update log
	* @property UPDATELOG
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.UPDATELOG	= 11;	
	
	/**
	* Action: Show app update log
	* @property APPUPDATELOG
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.APPUPDATELOG	= 12;	

	/**
	* Action: Add or remove item from account
	* @property ACCOUNTSTAT
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.ACCOUNTSTAT	= 13;	

	/**
	* Action: Show items developer page
	* @property DEVPAGE
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.DEVPAGE	= 14;	

	/**
	* Action: Update item
	* @property UPDATE
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.UPDATE	= 15;	

	/**
	* Action: Show item update prompt (extraOne: show remider ("true"|"false")
	* @property SHOWUPDATE
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.SHOWUPDATE	= 16;	
	
	/**
	* Action: Check if item is installed
	* @property INSCHECK
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.INSCHECK	= 17;	

	/**
	* Action: Launch installed wizzard
	* @property INSTALLEDW
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.INSTALLEDW	= 18;	

	/**
	* Action: Pause item
	* @property PAUSE
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.PAUSE	= 19;	

	/**
	* Action: Unpause item
	* @property UNPAUSE
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.UNPAUSE	= 20;	

	/**
	* Action: Show play list tab
	* @property PLAY
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.PLAY	= 21;	
	
	/**
	* Action: Switch item branch (extraOne: branchId)
	* @property SWITCHBRANCH
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.SWITCHBRANCH	= 22;	

	/**
	* Action: Show item prompt (extraOne: ("update"|"launch"|"eula"|"preorder"))
	* @property PROMPT
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.PROMPT	= 23;	

	/**
	* Action: Remove complex mod from a game
	* @property CLEANCOMPLEXMOD
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.CLEANCOMPLEXMOD	= 24;		
	
	
	/**
	* Action: Display the items cd key
	* @property DISPCDKEY
	* @final
	* @static
	*  @type Action
	*/		

	desura.links.action.DISPCDKEY	= 25;			
	
	
	
})();
