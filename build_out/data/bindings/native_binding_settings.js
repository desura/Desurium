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

if (!desura.settings)
    desura.settings = {};
	
if (!desura.cip)
	desura.cip = {};
	
desura.gamelink = desura.gamelink || {};
	
(function(){
	/**
    * @class settings
    */

	/**
	* Gets the value of a CVar
	* @method getValue
	* @param {String} Cvar name	
	* @return {String} CVar value
	*/		
	
	desura.settings.getValue = function(name){
		native function getValue();
		return getValue(name);
	};
		
		
	/**
	* Gets the max core count for this computer
	* @method getCoreCount
	* @return {Int} Max core count
	*/	
	
	desura.settings.getCoreCount = function(){
		native function getCoreCount();
		return getCoreCount();
	};

	/**
	* Gets an array of languages in the form of {file, name}
	* @method getLanguages
	* @return {Array} Language array
	*/		
	
	desura.settings.getLanguages = function(){
		native function getLanguages();
		return getLanguages();
	};
	
	/**
	* Gets an array of steam user names
	* @method getSteamNames
	* @return {Array} Steam user array
	*/		
	
	desura.settings.getSteamNames = function(){
		native function getSteamNames();
		return getSteamNames();
	};
	
	/**
	* Gets an array of themes. Each them is an object of the following: {name, image, tooltip, author, version}
	* @method getThemes
	* @return {Array} Themes array
	*/		
	
	desura.settings.getThemes = function(){
		native function getThemes();
		return getThemes();	
	};
	
	/**
	* Gets the game install path
	* @method getGameInstallPath
	* @return {String} game install path
	*/	
	
	desura.settings.getGameInstallPath = function(){
		native function getGameInstallPath();
		return getGameInstallPath();
	};
	
	/**
	* Gets the desura cache path
	* @method getCachePath
	* @return {String} Desura cache path
	*/	
	
	desura.settings.getCachePath = function(){
		native function getCachePath();
		return getCachePath();
	};
	
	/**
	* Browses for a new cache path
	* @method browseCachePath
	* @param {String} Current path
	* @return {String} New cache path
	*/	
	
	desura.settings.browseCachePath = function(curPath){
		native function browseCachePath();
		return browseCachePath(curPath);
	};
	
	/**
	* Checks to see if valid path
	* @method isValidCachePath
	* @param {String} Current path
	* @return {String} Error string or null if no error
	*/		
	
	desura.settings.isValidCachePath = function(path){
		native function isValidCachePath();
		return isValidCachePath(path);
	};
	
	/**
	* Sets the cache path. This will cause desura to restart strait away!!!
	* @method setCachePath
	* @param {String} new Cache path
	*/	
	
	desura.settings.setCachePath = function(path){
		native function setCachePath();
		return setCachePath(path);
	};	
	
	/**
	* Sets a cvar value
	* @method setValue
	* @param {String} Cvar Name
	* @param {String} new Cvar value
	*/	
	
	desura.settings.setValue = function(name, val){
		native function setValue();
		setValue(name, val);
	};
	
	

	desura.cip.getCurrentItems = function(){
		native function getCurrentCIPItems();
		return getCurrentCIPItems();
	};
	
	desura.cip.getAllItems = function(){
		native function getAllCIPItems();
		return getAllCIPItems();
	};
	
	desura.cip.updateList = function(){
		native function updateCIPList();
		updateCIPList();	
	};
	
	desura.cip.saveList = function(list){
		native function saveCIPList();
		saveCIPList(list);	
	};
	
	desura.cip.browse = function(name, path) {
		native function browseCIPPath();
		return browseCIPPath(name, path);
	};
	
	desura.cip.isValidPath = function(path){
		native function isValidCIPPath();
		return isValidCIPPath(path);
	};
	
	desura.gamelink.browse = function(name, path) {
		native function browseLinkBinary();
		return browseLinkBinary(name, path);
	};
	
	desura.gamelink.isValidBinary = function(bin){
		native function isValidLinkBinary();
		return isValidLinkBinary(bin);
	};

})();