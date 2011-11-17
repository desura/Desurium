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

if (!desura.items.branch)
    desura.items.branch = {};
	
if (!desura.items.branch.BranchInfo)
    desura.items.branch.BranchInfo = {};
	
if (!desura.items.branch.flags)
    desura.items.branch.flags = {};
	
(function(){


	/**
    * @class items.branch
    */
	
	/**
	* Converts a branchId into a BranchInfo object
	* @method getBranch	
	* @param ID itemId Id of item the branch belongs to
	* @param BID branchId Id of branch
	* @return {:api:`BranchInfo <desura.items.branch.BranchInfo>`} Branch object
	*/		
	
	desura.items.branch.getBranch = function(itemId, branchId){
		return desura.items.branch.BranchInfo(this);
	};
	
	
	/**
	* Creates a Branch Info from an branch id
	* @param bid Branch id
	* @method BranchInfo
	* @return {:api:`BranchInfo <desura.items.branch.BranchInfo>`} BranchInfo object
	*/		
	
	
	/**
    * @class items.branch.BranchInfo
    */	
	
	desura.items.branch.BranchInfo = function(self){
		
		if (self == null || typeof self == 'undefined')
			return null;	
		

		self.isValid = function(){
			native function isValidBranch();
			return isValidBranch(this);
		};	
		
		self._branchId = null;
		self._itemId = null;
		
		/**
		* Gets the branch id
		* @method getBranchId
		* @return {BID} Branch id 
		*/	
		
		self.getBranchId = function(){
			if (this._branchId != null)
				return this._branchId;
				
			native function getBranchId();
			this._branchId = getBranchId(this);
		
			return this._branchId;
		};
		
		/**
		* Gets the branch item id
		* @method getItemId
		* @return {ID} Item id 
		*/
		
		self.getItemId = function(){
			if (this._itemId != null)
				return this._itemId;
				
			native function getItemId();
			this._itemId = getItemId(this);
		
			return this._itemId;
		};
		
		/**
		* Gets the branch flags
		* @method getFlags
		* @return {Int} Flags
		*/
		
		self.getFlags = function(){
			native function getBranchFlags();
			return getBranchFlags(this);
		};
		
		/**
		* Gets the branch name
		* @method getName
		* @return {String} Branch name 
		*/
		
		self.getName = function(){
			native function getBranchName();
			return getBranchName(this);
		};
		
		/**
		* Gets the branch cost
		* @method getCost
		* @return {String} Branch cost
		*/
		
		self.getCost = function(){
			native function getBranchCost();
			return getBranchCost(this);
		};
		
		/**
		* Gets the branch pre order date.
		* @method getPreorderDate
		* @return {String} Branch preorder date in form   or NULL if none
		*/	
		
		self.getPreorderDate = function(){
			native function getBranchPreorderDate();
			return getBranchPreorderDate(this);	
		}
		
		/**
		* Is this branch the windows version?
		* @method isWindows
		* @return {Bool} true/false
		*/		
		
		self.isWindows = function(){
			native function isWindows();
			return isWindows(this);
		};
		
		/**
		* Is this branch the linux version?
		* @method isLinux
		* @return {Bool} true/false
		*/		
		
		self.isLinux = function(){
			native function isLinux();
			return isLinux(this);
		};
		
		/**
		* Is this branch 32 bit?
		* @method is32Bit
		* @return {Bool} true/false
		*/	
		
		self.is32Bit = function(){
			native function is32Bit();
			return is32Bit(this);
		};
		
		/**
		* Is this branch 64 bit?
		* @method is64Bit
		* @return {Bool} true/false
		*/		
		
		self.is64Bit = function(){
			native function is64Bit();
			return is64Bit(this);
		};

		return self;
	};

	/**
	* Branch status flags
    * @class items.branch.flags
    */	
	
	/**
	* Flag: Should display name
	* @property DISPLAY_NAME
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.branch.flags.DISPLAY_NAME = 2;
	
	/**
	* Flag: Free branch
	* @property FREE
	* @final
	* @static
	* @type Int
	*/		
	
	desura.items.branch.flags.FREE		= 4;
	
	/**
	* Flag: Branch is on account
	* @property ONACCOUNT
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.branch.flags.ONACCOUNT	= 8;
	
	/**
	* Flag: Branch is region locked
	* @property REGIONLOCK
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.branch.flags.REGIONLOCK	= 16;
	
	/**
	* Flag: Branch is member locked
	* @property MEMBERLOCK
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.branch.flags.MEMBERLOCK	= 32;
	
	/**
	* Flag: Branch has no releases
	* @property NORELEASES
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.branch.flags.NORELEASES	= 64;
	
	/**
	* Flag: Branch is a demo
	* @property DEMO
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.branch.flags.DEMO		= 128;
	
	
	/**
	* Flag: Eula for branch has been accepted
	* @property ACCEPTED_EULA
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.branch.flags.ACCEPTED_EULA	= 256;
	
	/**
	* Flag: Branch is a preoder
	* @property PREORDER
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.branch.flags.PREORDER		= 512;
	
	
	/**
	* Flag: Branch is a test branch
	* @property TEST
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.branch.flags.TEST			= 1024;
	
	
	/**
	* Flag: Branch has a cd key assoicated with it
	* @property CDKEY
	* @final
	* @static
	* @type Int
	*/	
	
	desura.items.branch.flags.CDKEY			= 2048;	
})();
