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

if (!desura.items)
    desura.items = {};

if (!desura.items.utils)
    desura.items.utils = {};

if (!desura.items.ItemInfo)
    desura.items.ItemInfo = {};

if (!desura.items.status)
    desura.items.status = {};

if (!desura.items.type)
    desura.items.type = {};

if (!desura.items.changed)
    desura.items.changed = {};		
	
if (!desura.items.options)
    desura.items.options = {};		
	
(function(){

	
	/**
    * @class items.utils
    */
	
	/**
	* Converts a list of ids to a list of item info objects
	* @param list {Int[]} List of item ids
	* @method convertList
	* @return {:api:`ItemInfo[] <desura.items.ItemInfo>`} Array of ItemInfo objects
	*/		
	
	desura.items.utils.convertList = function(list){
		var rList = [];
		for (var i=0; i<list.length; i++) {
			rList.push( desura.items.ItemInfo(list[i]) );
		}
		return rList;
	};
	
	/**
	* Creates a iteminfo from an item id
	* @param id Item id
	* @method ItemInfo
	* @return {:api:`ItemInfo <desura.items.ItemInfo>`} ItemInfo object
	*/	
	
	/**
    * @class items.ItemInfo
    */	
	
	desura.items.ItemInfo = function(self){
	
		if (self == null || typeof self == 'undefined')
			return null;
	
		/**
		* Gets item id
		* @method getId
		* @return {String} id 
		*/	
		
		self._id = null;
		self.getId = function(){
		
			if (this._id != null)
				return this._id;
			
			native function getItemId();
			this._id = getItemId(this);
			
			return this._id;
		};
		
		/**
		* Gets item name
		* @method getName
		* @return {String} item name 
		*/		
		
		self._name = null;
		self.getName = function(){
		
			if (this._name)
				return this._name;		
		
			native function getItemName();
			this._name = getItemName(this);
			
			return this._name;
		};
		
		/**
		* Gets parents item id
		* @method getParentId
		* @return {String} id 
		*/		
	
		self._parentId = null;
		self.getParentId = function(){
			
			if (this._parentId)
				return this._parentId;
		
			native function getItemParent();
			this._parentId = getItemParent(this);
			
			return this._parentId;
		};
	
		/**
		* Gets installed mod id
		* @method getInstalledModId
		* @return {String} installed mod id 
		*/	
		
		self.getInstalledModId = function(){
			native function getItemInstalledModId();
			return getItemInstalledModId(this);
		};
		
		/**
		* Gets changed flags
		* @method getChangedFlags
		* @return {Int} changed flags 
		*/	
		
		self.getChangedFlags = function(){
			native function getItemChangedFlags();
			return getItemChangedFlags(this);
		};
		
		/**
		* Gets item status
		* @method getStatus
		* @return {String} Item status 
		*/	
		
		self.getStatus = function(){
			native function getItemStatus();
			return getItemStatus(this);
		};
		
		/**
		* Gets item current percent
		* @method getPercent
		* @return {Int} Item percent 
		*/	
		
		self.getPercent = function(){
			native function getItemPercent();
			return getItemPercent(this);
		};
		
		/**
		* Gets item options
		* @method getOptions
		* @return {Int} Item options 
		*/	
		
		self.getOptions = function(){
			native function getItemOptions();
			return getItemOptions(this);	
		};
		
		/**
		* Sets item options
		* @method addOptionFlags
		* @param {Int} options Options to set
		*/	
		
		self.addOptionFlags = function(options){
			native function addItemOptionFlags();
			return addItemOptionFlags(this, options);
		};
	
		/**
		* Deletes item options
		* @method delOptionFlags
		* @param {Int} options Options to set
		*/	
	
		self.delOptionFlags = function(options){
			native function delItemOptionFlags();
			return delItemOptionFlags(this, options);
		};	
		
		/**
		* Can the item be launched
		* @method isLaunchable
		* @return {Bool} Is launchable 
		*/	
		
		self.isLaunchable = function(){
			native function isItemLaunchable();
			return isItemLaunchable(this);
		};
		
		/**
		* Is the item updating
		* @method isUpdating
		* @return {Bool} Is updating 
		*/	
		
		self.isUpdating = function(){
			native function isItemUpdating();
			return isItemUpdating(this);
		};
		
		/**
		* Is the item installed
		* @method isInstalled
		* @return {Bool} Is installed 
		*/	
		
		self.isInstalled = function(){
			native function isItemInstalled();
			return isItemInstalled(this);
		};
		
		/**
		* Is the item downloadable
		* @method isDownloadable
		* @return {Bool} Is downloadable 
		*/	
		
		self.isDownloadable = function(){
			native function isItemDownloadable();
			return isItemDownloadable(this);
		};
		
		/**
		* Is the item only a link to an exe
		* @method isLink
		* @return {Bool} Is a link 
		*/	
		
		self.isLink = function(){
			native function isLink();
			return isLink(this);
		};	
	
		/**
		* Get item genre
		* @method getGenre
		* @return {String} Item genre 
		*/	
		
		self.getGenre = function(){
			native function getItemGenre();
			return getItemGenre(this);
		};
	
		/**
		* Get item rating
		* @method getRating
		* @return {String} Item rating 
		*/	
		
		self.getRating = function(){
			native function getItemRating();
			return getItemRating(this);
		};
		
		/**
		* Gets item developer name
		* @method getDev
		* @return {String} Item developer
		*/	
		
		self._dev = null;
		self.getDev = function(){
		
			if (this._dev)
				return this._dev;
		
			native function getItemDev();
			this._dev = getItemDev(this);
			
			return this._dev;
		};
		
		/**
		* Gets item short name as used by Desura.com
		* @method getShortName
		* @return {String} Short name 
		*/	
		
		self._shortName = null;
		self.getShortName = function(){
		
			if (this._shortName)
				return this._shortName;
		
			native function getItemShortName();
			this._shortName = getItemShortName(this);
			
			return this._shortName;
		};
		
		/**
		* Gets item status string
		* @method getStatusStr
		* @return {String} Status string 
		*/	
		
		self.getStatusStr = function(){
			native function getItemStatusStr();
			return getItemStatusStr(this);
		};
		
		/**
		* Gets item install path
		* @method getPath
		* @return {String} Install path 
		*/	
		
		self.getPath = function(){
			native function getItemPath();
			return getItemPath(this);
		};
		
		/**
		* Gets item primary install path
		* @method getInsPrimary
		* @return {String} Primary install path 
		*/	
		
		self.getInsPrimary = function(){
			native function getItemInsPrimary();
			return getItemInsPrimary(this);
		};
		
		/**
		* Gets item icon as local file path
		* @method getIcon
		* @return {String} Item icon 
		*/	
		
		self.getIcon = function(){
			native function getItemIcon();
			return getItemIcon(this);
		};
		
		/**
		* Gets item logo as local file path
		* @method getLogo
		* @return {String} Item logo 
		*/	
		
		self.getLogo = function(){
			native function getItemLogo();
			return getItemLogo(this);
		};
		
		/**
		* Gets item icon url
		* @method getIconUrl
		* @return {String} Icon url 
		*/	
		
		self.getIconUrl = function(){
			native function getItemIconUrl();
			return getItemIconUrl(this);
		};
		
		/**
		* Gets item logo url
		* @method getLogoUrl
		* @return {String} Logo url 
		*/	
		
		self.getLogoUrl = function(){
			native function getItemLogoUrl();
			return getItemLogoUrl(this);
		};
		
		/**
		* Gets item profile url
		* @method getProfile
		* @return {String} Profile url 
		*/	
		
		self._profile = null;
		self.getProfile = function(){
		
			if (this._profile)
				return this._profile;
		
			native function getItemProfile();
			this._profile = getItemProfile(this);
			
			return this._profile;
		};
		
		/**
		* Gets item developer profile url
		* @method getDevProfile
		* @return {String} Developer profile url 
		*/	
		
		self._devProfile = null;
		self.getDevProfile = function(){
			
			if (this._devProfile)
				return this._devProfile;
		
			native function getItemDevProfile();
			this._devProfile = getItemDevProfile(this);
			
			return this._devProfile;
		};
		
		/**
		* Gets the items exe info as an array
		* Each element of the array has a sub array of following structure:
		*   name -> exe print name
		* 	exe -> exe path
		*   exeargs -> exe args set by desura
		*   userargs -> exe args set by user
		*   
		* @method getExeInfoList
		* @return {String} Exe path 
		*/	
		
		self.getExeInfoList = function(){
			native function getItemExeInfo();
			return getItemExeInfo(this);
		};
		
		/**
		* Sets the items exe user arg as an array
		* Each element of the array has a sub array of following structure:
		*   name -> exe print name
		* 	userargs -> new userarg value
		*   
		* @method setExeUserArgs
		* @param {Array} values Values array (described above)
		*/	
		
		self.setExeUserArgs = function(values){
			native function setItemUserArgs();
			return setItemUserArgs(this, values);
		};
			
		/**
		* Gets item branch count
		* @method getBranchCount
		* @return {Int} Count 
		*/	
		
		self.getBranchCount = function(){
			native function getItemBranchCount();
			return getItemBranchCount(this);
		};
		
		/**
		* Gets item branch by index
		* @method getBranchByIndex
		* @param {Int} index Index in branch array
		* @return {:api:`BranchInfo <desura.items.branch.BranchInfo>`} Branch
		*/	
		
		self.getBranchByIndex = function(index){
			native function getItemBranchByIndex();
			var branch = getItemBranchByIndex(this, index);
			
			return desura.items.branch.BranchInfo(branch);
		};
		
		/**
		* Gets item branch by id
		* @method getBranchById
		* @param {BID} branchId Branch id
		* @return {:api:`BranchInfo <desura.items.branch.BranchInfo>`} Branch 
		*/	
		
		self.getBranchById = function(branchId){
			native function getItemBranchById();
			var branch = getItemBranchById(this, branchId);
			
			return desura.items.branch.BranchInfo(branch);
		};		
		
		/**
		* Gets item currently installed branch
		* @method getCurrentBranch
		* @return {:api:`BranchInfo <desura.items.branch.BranchInfo>`} Branch 
		*/	
		
		self.getCurrentBranch = function(){
			native function getItemCurrentBranch();
			var branch = getItemCurrentBranch(this);
			
			return desura.items.branch.BranchInfo(branch);	
		};

		/**
		* Gets item install size
		* @method getInstallSize
		* @return {Int} Install size 
		*/	
		
		self.getInstallSize = function(){
			native function getItemInstallSize();
			return getItemInstallSize(this);
		};
		
		/**
		* Gets item download size
		* @method getDownloadSize
		* @return {Int} Download size 
		*/			
		
		self.getDownloadSize = function(){
			native function getItemDownloadSize();
			return getItemDownloadSize(this);
		};
		
		/**
		* Gets item installed build number
		* @method getInstalledBuild
		* @return {Int} Installed build
		*/	
		
		self.getInstalledBuild = function(){
			native function getItemInstalledBuild();
			return getItemInstalledBuild(this);
		};
		
		/**
		* Gets item installed branch number
		* @method getInstalledBranch
		* @return {Int} Installed branch 
		*/	
		
		self.getInstalledBranch = function(){
			native function getItemInstalledBranch();
			return getItemInstalledBranch(this);
		};
		
		/**
		* Gets item installed version string
		* @method getInstalledVersion
		* @return {String} Installed version
		*/	
		
		self.getInstalledVersion = function(){
			native function getItemInstalledVersion();
			return getItemInstalledVersion(this);
		};
		
		/**
		* Gets item type (see.type.XXXX )
		* @method getType
		* @return {Int} Item Type
		*/		
		
		self._type = null;
		self.getType = function(){
		
			if (this._type)
				return this._type;
		
			native function getItemType();
			this._type = getItemType(this);
			
			return this._type;
		};
		
		
		/**
		* Is this item a favorite for the current user?
		* @method isFavorite
		* @return {Bool} Is favorite
		*/		
		
		self.isFavorite = function(){
			native function isItemFavorite();
			return isItemFavorite(this);
		};
		
		
		/**
		* Sets this item as favorite for user
		* @method setFavorite
		* @param {Bool} isFavorite true for is, false for not. Default is true
		* @return {Int} Item Type
		*/		
		
		self.setFavorite = function(isFavorite){
			native function setItemFavorite();
			return setItemFavorite(this, isFavorite);
		};
		
		
		/**
		* Does this item exist in the system
		* @method isValid
		* @return {Bool} True if valid item
		*/		
		
		self.isValid = function(){
			native function isItemValid();
			return isItemValid(this);
		};		
		
		
		/**
		* Does this item exist in the system
		* @method isItemBusy
		* @return {Bool} If the item is in a stage (i.e. doing something) this returns true
		*/		
		
		self.isBusy = function(){
			native function isItemBusy();
			return isItemBusy(this);
		};				
		
		/**
		* Creates a desktop shortcut
		* @method createDesktopShortcut
		* @return {Bool} Ceates a shortcut and returns true if succeceded, false if failed
		*/		
		
		self.createDesktopShortcut = function(){
			native function createItemDesktopShortcut();
			return createItemDesktopShortcut(this);
		};					

		/**
		* Creates a menu entry
		* @method createMenuEntry
		* @return {Bool} Ceates a menu entry and returns true if succeceded, false if failed
		*/		
		
		self.createMenuEntry = function(){
			native function createItemMenuEntry();
			return createItemMenuEntry(this);
		};					
		
		return self;
	};
	
	
	/**
	* ItemInfo status flags
	* @class items.status
	*/
	
	/**
	* Status Flag: unknown status
	* @property UNKNOWN
	* @final
	* @static
	*  @type Int
	*/		

	desura.items.status.UNKNOWN		= 0;
	
	/**
	* Status Flag: item is installed on the computer
	* @property INSTALLED
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.status.INSTALLED	= 1<<1;
	
	/**
	* Status Flag: item is on the users account
	* @property ONACCOUNT
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.status.ONACCOUNT	= 1<<2;
	
	/**
	* Status Flag: item is local to the user
	* @property ONCOMPUTER
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.status.ONCOMPUTER	= 1<<3;
	
	/**
	* Status Flag: item is ready to go
	* @property READY
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.status.READY		= 1<<4;
	
	/**
	* Status Flag: item is updating atm
	* @property UPDATING
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.status.UPDATING		= 1<<5;
	
	/**
	* Status Flag: item is downloading atm
	* @property DOWNLOADING
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.status.DOWNLOADING	= 1<<6;
	
	/**
	* Status Flag: item is installing atm
	* @property INSTALLING
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.status.INSTALLING	= 1<<7;
	
	/**
	* Status Flag: item is uploading atm
	* @property UPLOADING
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.status.UPLOADING	= 1<<8;
	
	/**
	* Status Flag: item is verifing atm
	* @property VERIFING
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.status.VERIFING		= 1<<9;
	
	/**
	* Status Flag: user is a dev for this item
	* @property DEVELOPER
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.status.DEVELOPER	= 1<<10;
	
	/**
	* Status Flag: means it has no info
	* @property STUB
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.status.STUB			= 1<<11;
	
	/**
	* Status Flag: means it has full info
	* @property FULL
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.status.FULL			= 1<<12;
	
	/**
	* Status Flag: means this item has an update for it
	* @property UPDATEAVAL
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.status.UPDATEAVAL	= 1<<13;
	
	/**
	* Status Flag: means the item is deleted (safe delete)
	* @property DELETED
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.status.DELETED		= 1<<14;
	
	/**
	* Status Flag: means this item cant be downloaded
	* @property NONDOWNLOADABLE
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.status.NONDOWNLOADABLE = 1<<15;
	
	/**
	* Status Flag: means the item needs a clean up for its MCF files
	* @property NEEDCLEANUP
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.status.NEEDCLEANUP	= 1<<16;
	
	/**
	* Status Flag: means the item is paused in its current process
	* @property PAUSED
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.status.PAUSED		= 1<<17;
	
	/**
	* Status Flag:  means the current install is not authed yet
	* @property UNAUTHED
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.status.UNAUTHED		= 1<<18;
	
	/**
	* Status Flag: item can be paused at the current stage
	* @property PAUSABLE
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.status.PAUSABLE		= 1<<19;
	
	/**
	* Status Flag: item has complex install
	* @property INSTALLCOMPLEX
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.status.INSTALLCOMPLEX = 1<<20;
	
	/**
	* Status Flag: item is a link and is not managed by Desura
	* @property LINK
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.status.LINK			= 1<<21;

	/**
	* Status Flag: item has a cd key
	* @property CDKEY
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.status.CDKEY			= 1<<24;	
	

	/**
	* Status Flag: item has a downloadable content
	* @property DLC
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.status.DLC				= 1<<25;		
	
	
	/**
	* Status Flag: Item has been preordered and downloaded
	* @property PREORDER
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.status.PREORDER		= 1<<26;			
	
	/**
	* ItemInfo changed flags (for changed callback event)
	* @class items.changed
	*/	
	
	/**
	* Changed Flag: No changes
	* @property NOCHANGE
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.changed.NOCHANGE = 0;
	
	/**
	* Changed Flag: Icon changed
	* @property ICON
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.changed.ICON	= 1<<1;
	
	/**
	* Changed Flag: Logo changed
	* @property LOGO
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.changed.LOGO	= 1<<2;
	
	/**
	* Changed Flag: Status changed
	* @property STATUS
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.changed.STATUS	= 1<<3;
	
	/**
	* Changed Flag: Percent changed
	* @property PERCENT
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.changed.PERCENT	= 1<<4;
	
	/**
	* Changed Flag: Information changed
	* @property INFO
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.changed.INFO	= 1<<5;

	/**
	* ItemInfo type flags
	* @class items.type
	*/		
	
	
	/**
	* Type Flag: Item is a game
	* @property GAME
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.type.GAME	= 2;
	
	/**
	* Type Flag: Item is a mod
	* @property MOD
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.type.MOD	= 1;

	/**
	* ItemInfo type flags
	* @class items.options
	*/			
	
	/**
	* Type Flag: Item is a mod
	* @property NOTREMINDUPDATE
	* @final
	* @static
	* @type Int
	*/		
	
	desura.items.options.NOTREMINDUPDATE = 1<<3;
	
})();
