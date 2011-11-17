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
 * This is the native (c++) bindings to javascript for `Desura <http://www.desura.com>`_
 * @module desura
 * @author Mark Chandler
 * @version 1
 */

var desura;
if (!desura) {
    desura = {};
}
	
if (!desura.utils) {
    desura.utils = {};
}

if (!desura.crumbs) {
    desura.crumbs = {};
}

if (!desura.items) {
    desura.items = {};
}

if (!desura.internal) {
    desura.internal = {};
}
	
(function(){

	/**
	* Utility function for interacting with Desura
	* @class utils
	*/
    
	/**
	* Gets a string from the language file. Can be raw or have format parts.
	* i.e.: 
	* #abc is "This is string abc"
	* #dce is "This {0} string {1}"
	* 
	* desura.utils.getString("#abc") returns "This is string abc"
	* desura.utils.getString("#dce", "is", "spatar!!") returns "This is string spatar!!"
	* 
	* @method getLocalString
	* @param {String} str String id in language file
	* @param {String} Strings to use in format
	* @return {String} Translated string
	*/	

	desura.utils.getLocalString = function(str) {
		native function getLocalString();
		return getLocalString.apply(this, arguments);
	};

	/**
	* Gets color one from the theme file
	* @method getThemeColor
	* @param {String} controlName Control name in theme file
	* @param {String} colorId Color id in theme file
	* @return {String} Color in the form of #RRGGBB
	*/		
	
	desura.utils.getThemeColor = function(controlName, colorId) {
		native function getThemeColor();
		return getThemeColor(controlName, colorId);
	};
	
	/**
	* Gets a image location from the theme file
	* @method getThemeImage
	* @param {String} name Image name in theme file
	* @return {String} Image location
	*/	
	
	desura.utils.getThemeImage = function(name) {
		native function getThemeImage();
		return getThemeImage(name);	
	};

	/**
	* Base64 encodes a string
	* @method base64encode
	* @param {String} string string to encode
	* @return {String} encoded string
	*/		
	
   desura.utils.base64encode = function(string) {
        native function base64encode();
        return base64encode(string);
    };

	
	/**
	* Gets a cached value from desura
	* @method getCacheValue
	* @param {String} name Key name
	* @param {String} defaultValue Default value if cant be found
	* @return {String} Cached string or NULL if none
	*/	

	desura.utils.getCacheValue = function(name, defaultValue) {
	
		if (!defaultValue)
			defaultValue = "";

		native function getCacheValue();
		return getCacheValue(name, defaultValue);
	};	
	
	/**
	* Sets a cached value from desura
	* @method setCacheValue
	* @param {String} name Key name
	* @param {String} value Value to save
	* @param {Int} length Length of value (only required if length is greater than 255 chars)
	*/	

	desura.utils.setCacheValue = function(name, value, length) {
		native function setCacheValue();
		setCacheValue(name, value, length);
	};		
	
	
	/**
	* Gets the siteareaid from an id
	* @method getItemFromId
	* @param {ID} Item Id
	* @return {Int} Type
	*/	
	
	desura.utils.getItemFromId = function(id) {
		native function getItemFromId();
		return getItemFromId(id);
	};
	
	/**
	* Gets the sitearea from an id
	* @method getTypeFromId
	* @param {ID} Item Id
	* @return {String} Site area
	*/		
	
	desura.utils.getTypeFromId = function(id) {
		native function getTypeFromId();
		return getTypeFromId(id);
	};

	/**
	* Gets the cvar value from desura. Returns default if cvar doesnt exist
	* @method getCVarValue
	* @param {String} Cvar name
	* @return {Value} Default value to return if cvar is not found
	*/			
	
	desura.utils.getCVarValue = function(cvarName, defValue) {
		native function getCVarValue();
		
		var ret = getCVarValue(cvarName);
		
		if (ret == null)
			return defValue;
		
		return ret;
	};
	
	/**
	* Loads the template name and returns its contents as a string
	* @method getTemplate
	* @param {String} Template name
	* @return {String} Template contents
	*/			
	
	desura.utils.getTemplate = function(name) {
		native function getTemplate();
		return getTemplate(name);
	};
	
	/**
	* Is the icon pointed to by path a valid icon
	* @method isValidIcon
	* @param {String} Icon path
	* @return {Bool} Is valid
	*/	
	
	desura.utils.isValidIcon = function(icon) {
		native function isValidIcon();
		return isValidIcon(icon);
	};
	
	/**
	* Is desura in offline mode?
	* @method isOffline
	* @return {Bool} Is offline
	*/	
	
	desura.utils.isOffline = function() {
		native function isOffline();
		return isOffline();
	};

	/**
	* Runs the update poll
	* @method forceUpdatePoll
	*/	
	
	desura.utils.forceUpdatePoll = function() {
		native function forceUpdatePoll();
		return forceUpdatePoll();
	};
	
	
	/**
	* Is os windows version?
	* @method isWindows
	* @return {Bool} true/false
	*/		
	
	desura.utils.isWindows = function(){
		native function isWindows();
		return isWindows();
	};
	
	/**
	* Is os linux version?
	* @method isLinux
	* @return {Bool} true/false
	*/		
	
	desura.utils.isLinux = function(){
		native function isLinux();
		return isLinux();
	};
	
	/**
	* Is os 32 bit?
	* @method is32Bit
	* @return {Bool} true/false
	*/	
	
	desura.utils.is32Bit = function(){
		native function is32Bit();
		return is32Bit();
	};
	
	/**
	* Is os 64 bit?
	* @method is64Bit
	* @return {Bool} true/false
	*/		
	
	desura.utils.is64Bit = function(){
		native function is64Bit();
		return is64Bit();
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
	
	/**
	* Interface for Desura items	
	* @class items
	*/
	
	
	desura.items.getItemFromId = function(id){
		native function getItemInfoFromId();
		return desura.items.ItemInfo(getItemInfoFromId(id));
	};
	
	/**
	* Gets all the developer items from desura
	* @method getDevItems
	* @return {:api:`ItemInfo[] <desura.items.ItemInfo>`} Array of Developer items
	*/	
	
	desura.items.getDevItems = function() {
		native function getDevItems();
		var list = getDevItems();
		return desura.items.utils.convertList(list);
	};

	/**
	* Gets all the game items from desura
	* @method getGames
	* @return {:api:`ItemInfo[] <desura.items.ItemInfo>`} Array of games
	*/		
	
	desura.items.getGames = function() {
		native function getGames();
		var list = getGames();
		return desura.items.utils.convertList(list);
	};

	/**
	* Gets all the mod items of a game from desura
	* @method getMods
	* @param {ID} id Game id
	* @return {:api:`ItemInfo[] <desura.items.ItemInfo>`} Array of mods for game
	*/		
	
	desura.items.getMods = function(game) {
		native function getMods();
		var list = getMods(game);
		return desura.items.utils.convertList(list);
	};
	
	
	/**
	* Gets all the links items from desura
	* @method getLinks
	* @return {:api:`ItemInfo[] <desura.items.ItemInfo>`} Array of links
	*/		
	
	desura.items.getLinks = function() {
		native function getLinks();
		var list = getLinks();
		return desura.items.utils.convertList(list);
	};


	/**
	* Gets all the favorite items from desura
	* @method getFavorites
	* @return {:api:`ItemInfo[] <desura.items.ItemInfo>`} Array of favorites
	*/		
	
	desura.items.getFavorites = function() {
		native function getFavorites();
		var list = getFavorites();
		return desura.items.utils.convertList(list);
	};


	/**
	* Gets all the recent items from desura
	* @method getRecent
	* @return {:api:`ItemInfo[] <desura.items.ItemInfo>`} Array of recent items
	*/		
	
	desura.items.getRecent = function() {
		native function getRecent();
		var list = getRecent();
		return desura.items.utils.convertList(list);
	};	
	
	/**
	* Gets all the new items from desura
	* @method getNewItems
	* @return {:api:`ItemInfo[] <desura.items.ItemInfo>`} Array of new items
	*/		
	
	desura.items.getNewItems = function() {
		native function getNewItems();
		var list = getNewItems();
		return desura.items.utils.convertList(list);
	};		
	
	
	/**
	* Gets all the mcf uploads from desura
	* @method getUploads
	* @return {:api:`UploadInfo[] <desura.uploads.UploadInfo>`} Array of uploads
	*/		
	
	desura.items.getUploads = function() {
		native function getUploads();
		var list = getUploads();
		return desura.uploads.utils.convertList(list);
	};
	
	
	
	

	/**
	* Addes a new item to the list as a link to an exe
	* @param name Display name
	* @param exe Exe to launch
	* @param args Launch Args
	* @method addLink
	* @return {:api:`ItemInfo <desura.items.ItemInfo>`} Reference to added link
	*/			
	
	desura.items.addLink = function(name, exe, args){
		native function addLink();
		return addLink(name, exe, args);
	};
	
	/**
	* Deletes an item link
	* @param link Link to delete
	* @method delLink
	*/			
	
	desura.items.delLink = function(link){
		native function delLink();
		delLink(link);
	};
	
	/**
	* Updates a link
	* @param link Link to delete
	* @param args Launch Args	
	* @method updateLink
	*/			
	
	desura.items.updateLink = function(link, args){
		native function updateLink();
		updateLink(link, args);
	};	
	
	

	desura.contextmenu = function() {
		this._items = [];
		this._handlers = {};
	};

	desura.contextmenu.prototype = {
		show: function(event)
		{
			event.preventDefault();
		
			// Remove trailing separator.
			while (this._items.length > 0 && !("id" in this._items[this._items.length - 1]))
				this._items.splice(this._items.length - 1, 1);

			if (this._items.length) {
			
				desura._contextMenu = this;
				var res = desura.utils.showContextMenu(event.screenX, event.screenY, this._items, this, function(res){
					if (res != -1)
						this._itemSelected(res);
				});
			}
		},

		appendItem: function(label, handler, disabled)
		{
			var id = this._items.length;
			this._items.push({type: "item", id: id, label: label, enabled: !disabled});
			this._handlers[id] = handler;
		},

		appendCheckboxItem: function(label, handler, checked, disabled)
		{
			var id = this._items.length;
			this._items.push({type: "checkbox", id: id, label: label, checked: !!checked, enabled: !disabled});
			this._handlers[id] = handler;
		},

		appendSeparator: function()
		{
			// No separator dupes allowed.
			if (this._items.length === 0)
				return;
			if (!("id" in this._items[this._items.length - 1]))
				return;
			this._items.push({type: "separator"});
		},
		
		appendMenu: function(contextmenu, label) 
		{
			if (contextmenu == null || typeof(contextmenu) == "undefined")
				return;
	
			if (contextmenu._items.length == 0)
				return;
	
			this._items.push({type: "submenu", label: label, id: -1});
			
			for (var x=0; x<contextmenu._items.length; x++) {
			
				var i = contextmenu._items[x];
			
				var id = this._items.length;
				this._items.push({type: i.type, id: id, label: i.label, checked: i.checked, enabled: i.enabled});
				
				this._handlers[id] = contextmenu._handlers[i.id];
			}
			
			this._items.push({type: "endsubmenu", label: label, id: -1});
		},

		_itemSelected: function(id)
		{		
			if (this._handlers[id])
				this._handlers[id].call(this);
		}
	};
})();
