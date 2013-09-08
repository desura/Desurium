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


/**
 * @module desura
 */

var desura;
if (!desura) {
    desura = {};
}

if (!desura.uploads) {
    desura.uploads = {};
}

if (!desura.uploads.utils) {
    desura.uploads.utils = {};
}

if (!desura.uploads.UploadInfo) {
    desura.uploads.UploadInfo = {};
}

(function(){

	/**
    * @class uploads.utils
    */
	
	/**
	* Converts a list of upload ids to a list of upload info objects
	* @param list {Uid[]} List of upload ids
	* @method convertList
	* @return {:api:`UploadInfo[] <desura.uploads.UploadInfo>`} List of upload info objects
	*/		
	
	desura.uploads.utils.convertList = function(list){
		var rList = [];
		for (var i=0; i<list.length; i++) {
			rList.push( desura.uploads.UploadInfo(list[i]) );
		}
		return rList;
	};

	
	desura.uploads.getUploadFromId = function(uid){
		native function getUploadFromId();
		return desura.uploads.UploadInfo(getUploadFromId(uid));
	};

	/**
	* Creates a upload from an upload id
	* @param uid Upload id
	* @method UploadInfo
	* @return {:api:`UploadInfo <desura.uploads.UploadInfo>`} UploadInfo object
	*/		
	
	/**
    * @class uploads.UploadInfo
    */	
	
	desura.uploads.UploadInfo = function(self){
		
		if (self == null || typeof self == 'undefined')
			return null;
		
		self._uid = null;
		self._itemId = null;
		
		/**
		* Gets item uid
		* @method getuid
		* @return {String} uid 
		*/	
		
		self.getuid = function(){
			if (this._uid != null)
				return this._uid;
				
			native function getUploadId();
			this._uid = getUploadId(this);
		
			return this._uid;
		};
		
		/**
		* Gets the upload corrisponding item id
		* @method getItemId
		* @return {String} item id 
		*/	
		
		self.getItemId = function(){
			if (this._itemId != null)
				return this._itemId;
		
			native function getItemId();
			this._itemId = getItemId(this);
			
			return this._itemId;
		};	
		
		/**
		* Is the mcf getting deleted at the end of the upload
		* @method shouldDeleteMcf
		* @return {Bool }
		*/		
		
		self.shouldDeleteMcf = function(){
			native function shouldDeleteMcf();
			return shouldDeleteMcf(this);
		};
		
		/**
		* Sets if the mcf should be deleted on completion of the upload
		* @method setDeleteMcf
		*/		
	
		self.setDeleteMcf = function(state){
			native function setDeleteMcf();
			return setDeleteMcf(this, state);
		};
		
		/**
		* Gets the upload corrisponding item id
		* @method getItem
		* @return {ItemInfo} item 
		*/	
		
		self.getItem = function(){
			native function getItem();
			return desura.items.ItemInfo(getItem(this));
		};
		
		/**
		* Remove the upload (cancels it and stops it)
		* @method remove
		*/	
		
		self.remove = function(){
			native function remove();
			return remove(this);
		};
		
		/**
		* Pause the upload
		* @method pause
		*/	
		
		self.pause = function(){
			native function pause();
			return pause(this);
		};
		
		/**
		* Unpause the upload
		* @method unpause
		*/	
		
		self.unpause = function(){
			native function unpause();
			return unpause(this);
		};
		
		/**
		* Is the upload paused
		* @method isPaused
		* @return {Bool} Is paused 
		*/	
		
		self.isPaused = function(){
			native function isPaused();
			return isPaused(this);
		};
		
		/**
		* Is the upload complete
		* @method isComplete
		* @return {Bool} Is complete 
		*/		
		
		self.isComplete = function() {
			native function isComplete();
			return isComplete(this);
		};
		
		/**
		* Is the upload a valid item in desura
		* @method isValid
		* @return {Bool} Is valid 
		*/			
		
		self.isValid = function() {
			native function isValid();
			return isValid(this);
		};
		
		/**
		* Does the upload have an error
		* @method hasError
		* @return {Bool} Has error
		*/			
		
		self.hasError = function() {
			native function hasError();
			return hasError(this);	
		};
	
		/**
		* Has the upload started
		* @method hasStarted
		* @return {Bool} Has upload started
		*/			
		
		self.hasStarted = function() {
			native function hasStarted();
			return hasStarted(this);
		};
	
		/**
		* Open the mcf file location in explorer
		* @method exploreMcf
		*/			
		
		self.exploreMcf = function() {
			native function exploreMcf();
			exploreMcf(this);	
		};
		
		/**
		* Get the upload prgress
		* @method getProgress
		* @return {Int} Progress
		*/			
		
		self.getProgress = function() {
			native function getProgress();
			return getProgress(this);
		};
		
		/**
		* Cancels the upload
		* @method cancel
		*/			
		
		self.cancel = function() {
			native function cancel();
			cancel(this);
		};
		
		/**
		* Gets the name of the mcf being uploaded
		* @method getFileName
		* @return {String} File name
		*/		
		
		self.getFileName = function() {
			native function getFileName();
			return getFileName(this);
		};
		
		return self;
	};
})();
