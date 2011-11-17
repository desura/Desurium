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

if (!desura.user)
    desura.user = {};
	
(function(){


	/**
    * @class user
    */
	
	/**
	* Checks to see if the current logged in user is an admin
	* @method isAdmin
	* @return {Bool} is user an admin
	*/		
	
	desura.user.isAdmin = function(){
		native function isUserAdmin();
		return isUserAdmin();
	};
	
	
	/**
	* Forces the update poll to run
	* @method forceUpdatePoll	
	*/		
	
	desura.user.forceUpdatePoll = function(){
		native function forceUpdatePoll();
		forceUpdatePoll();
	};

	
	/**
	* Gets the desura user id
	* @method getUserId
	* @return {Int} User id
	*/		
	
	desura.user.getUserId = function(){
		native function getUserUserId();
		return getUserUserId();
	};
	
	
	/**
	* Gets the users avatar url (can be local file)
	* @method getAvatar
	* @return {String} avatar url
	*/		
	
	desura.user.getAvatar = function(){
		native function getUserAvatar();
		return getUserAvatar();
	};
	
	
	/**
	* Gets the users profile url on desura.com
	* @method getProfileUrl
	* @return {String} Profile url
	*/		
	
	desura.user.getProfileUrl = function(){
		native function getUserProfileUrl();
		return getUserProfileUrl();
	};

	
	/**
	* Gets the users edit profile url on desura.com
	* @method getProfileEditUrl
	* @return {String} Edit profile url
	*/		
	
	desura.user.getProfileEditUrl = function(){
		native function getUserProfileEditUrl();
		return getUserProfileEditUrl();
	};	
	
	
	/**
	* Gets the users user name id used on desura.com
	* @method getUserNameId
	* @return {String} User name id
	*/		
	
	desura.user.getUserNameId = function(){
		native function getUserUserNameId();
		return getUserUserNameId();
	};
	
	
	/**
	* Gets the users user name used on desura.com
	* @method getUserName
	* @return {String} User name
	*/		
	
	desura.user.getUserName = function(){
		native function getUserUserName();
		return getUserUserName();
	};
	
	
	/**
	* Gets the users private message count
	* @method getPmCount
	* @return {Int} Pm count
	*/		
	
	desura.user.getPmCount = function(){
		native function getUserPmCount();
		return getUserPmCount();
	};

	
	/**
	* Gets the users update count
	* @method getUpCount
	* @return {Int} Update count
	*/		
	
	desura.user.getUpCount = function(){
		native function getUserUpCount();
		return getUserUpCount();
	};

	
	/**
	* Gets the users cart items count
	* @method getCartCount
	* @return {Int} Cart count
	*/		
	
	desura.user.getCartCount = function(){
		native function getUserCartCount();
		return getUserCartCount();
	};	


})();
