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

if (!desura.events)
    desura.events = {};

if (!desura.events.main)
    desura.events.main = {};	

if (!desura.events.items)
    desura.events.items = {};		
	
if (!desura.events.internal)
    desura.events.internal = {};
	
if (!desura.events.uploads)
    desura.events.uploads = {};

if (!desura.events.chat)
    desura.events.chat = {};
	
if (!desura.events.cip)
	desura.events.cip = {};
	
(function(){
	
	function newDelegate(callback, flags, id){
		var delegate_ = {};
		
		delegate_.callback = callback;
		delegate_.flags = flags;
		delegate_.id = id;
		
		return delegate_;
	}	
	
	
	/**
	* Provideds basic event system
	* @class event
	*/		
	
	function newEvent(){
		var event_ = {};
		event_.delegates = [];
		event_.lastId = 0;
	
		function isUnsignedInteger(s) {
			return (s.toString().search(/^[0-9]+$/) === 0);
		}
	
		/**
		* Registers a delegate to an event
		* 
		* Example:
		*       desura.events.main.onTabClickedEvent.register(onTabClick, "all");
		* @param flags Filter flags, use "all" to register for all event triggers
		* @param callback	Function to call when event is triggered and flags match trigger flag
		* @method register
		* @return {DelegateID} Id of delegate. See: deregister
		*/		
	
		event_.register = function(callback, flags){

			event_.delegates.push(newDelegate(callback, flags, event_.lastId));
			event_.lastId += 1;
			
			return event_.lastId-1;
		};
		
		/**
		* Deregisters a delegate from an event
		* @param callback can be the original callback or the id returned when registering
		* @method deregister
		*/		
		
		event_.deregister = function(callback){
			var isInt = isUnsignedInteger(callback);
			for (var i=0; i<event_.delegates.length; i++){
				if (isInt && event_.delegates[i].id === callback)
				{
					event_.delegates.splice(i, 1);
					break;	
				}
				else if (event_.delegates[i].callback == callback){
					event_.delegates.splice(i, 1);
					break;
				}
			}
		};
	
		/**
		* Trigger an event
		* @param flags Filter flags for event, use "all" to trigger every delegate
		* @param value	Value to parse to delegates
		* @method trigger
		*/	
	
		event_.trigger = function(flags, value){
			for (var i=0; i<event_.delegates.length; i++){
				if (flags === "all" || event_.delegates[i].flags === flags || event_.delegates[i].flags === "all"){
					event_.delegates[i].callback(value);
				}
			}		
		};
		
		return event_;
	}
	
	/**
	* Events function for registering interest in desura callbacks
	* @class events.main
	*/		

	/**
	* Gets called when user clicks a tab. 
	* 
	* Flags is tabName 
	* 	
	* Value is tabName 
	* 
	* @property onTabClickedEvent
	* @final
	* @static
	* @type :api:`Event <desura.event>`
	*/
	
	desura.events.main.onTabClickedEvent = newEvent();	
	
	/**
	* Gets called when user enters text into search box. Value is search string
	* @property onSearchEvent
	* @final
	* @static
	* @type :api:`Event <desura.event>`
	*/		
	
	desura.events.main.onSearchEvent = newEvent();
	
	/**
	* Gets called when user clicks expand
	* @property onExpandEvent
	* @final
	* @static
	* @type :api:`Event <desura.event>`
	*/	
	
	desura.events.main.onExpandEvent = newEvent();
	
	/**
	* Gets called when user clicks contract
	* @property onContractEvent
	* @final
	* @static
	* @type :api:`Event <desura.event>`
	*/	
	
	desura.events.main.onContractEvent = newEvent();

	/**
	* Gets called when an update poll is occuring
	* @property onUpdatePollEvent
	* @final
	* @static
	* @type :api:`Event <desura.event>`
	*/		
	
	desura.events.main.onUpdatePollEvent = newEvent();
	
	
	/**
	* Events function for registering interest in item callbacks
	* @class events.items
	*/		
	
	/**
	* Gets called when an item has a update. Flag is itemId
	* 
	* Flags is item id
	* 	
	* Value is an object with the following attributes: 
	* 
	*  * id: Item id 
	*  * flags: Item update flags 
	* 	
	* @property onItemUpdateEvent
	* @final
	* @static
	* @type :api:`Event <desura.event>`
	*/		
	
	desura.events.items.onItemUpdateEvent = newEvent();
	
	/**
	* Gets called when item/s (game, mod, dev, tool) was added or removed
	* @property onItemListUpdatedEvent
	* @final
	* @static
	* @type :api:`Event <desura.event>`
	*/	
	
	desura.events.items.onItemListUpdatedEvent = newEvent();
	
	/**
	* Gets called when recent items was updated
	* @property onRecentUpdateEvent
	* @final
	* @static
	* @type :api:`Event <desura.event>`
	*/		
	
	desura.events.items.onRecentUpdateEvent = newEvent();
	
	/**
	* Gets called when favorite items was updated
	* @property onFavoriteUpdateEvent
	* @final
	* @static
	* @type :api:`Event <desura.event>`
	*/		
	
	desura.events.items.onFavoriteUpdateEvent = newEvent();
	
	/**
	* Gets called when a new item was added to the games list
	* @property onNewItemAdded
	* @final
	* @static
	* @type :api:`Event <desura.event>`
	*/	
	
	desura.events.items.onNewItemAddedEvent = newEvent();
	
	/**
	* Gets called when an alert box needs to be shown on the items page
	* 
	* Value is an object with the following attributes: 
	* 
	*  * text: Message to show 
	*  * time: Time to show message or 0 infinit 
	* 	
	* @property onShowAlertEvent
	* @final
	* @static
	* @type :api:`Event <desura.event>`
	*/		
	
	desura.events.items.onShowAlertEvent = newEvent();	
	
	
	/**
	* Events function for registering interest in item callbacks
	* @class events.uploads
	*/			
	
	/**
	* Gets called when upload items are updated
	* 
	* Flags is upload id
	* 		
	* Value is upload id
	* 
	* @property onUpdateEvent
	* @final
	* @static
	* @type :api:`Event <desura.event>`
	*/		
	
	desura.events.uploads.onUpdateEvent = newEvent();
	
	/**
	* Gets called when upload progress is updated
	* 	
	* Flags is upload id
	* 		
	* Value is upload id
	* 
	* @property onProgressEvent
	* @final
	* @static
	* @type :api:`Event <desura.event>`
	*/		
	
	desura.events.uploads.onProgressEvent = newEvent();
	
	
	
	

	desura.events.cip.onListUpdateEvent = newEvent();
	
	
	
	desura.events.items.onDelayLoadEvent = newEvent();		
	desura.events.items.onDelayLoadDoneEvent = newEvent();		
	
	
	
	
	
	
	
	
	
	
	
	desura.events.internal.onTabClicked = function(tabName){
		desura.events.main.onTabClickedEvent.trigger(tabName, tabName);
	};	
	
	desura.events.internal.onSearch = function(string){
		desura.events.main.onSearchEvent.trigger("all", string);
	};
	
	desura.events.internal.onExpand = function(){
		desura.events.main.onExpandEvent.trigger("all");
	};
	
	desura.events.internal.onContract = function(){
		desura.events.main.onContractEvent.trigger("all");
	};
	
	desura.events.internal.onItemUpdate = function(itemId, flags){
	
		var info = {
			id : itemId,
			flags : flags
		};
	
		desura.events.items.onItemUpdateEvent.trigger(itemId, info);
	};
	
	desura.events.internal.onItemListUpdated = function(){
		desura.events.items.onItemListUpdatedEvent.trigger("all");
	};
	
	desura.events.internal.onRecentUpdate = function(id){
		desura.events.items.onRecentUpdateEvent.trigger(id, id);
	};
	
	desura.events.internal.onFavoriteUpdate = function(id){
		desura.events.items.onFavoriteUpdateEvent.trigger(id, id);
	};
	
	desura.events.internal.onUploadUpdate = function(uid){
		desura.events.uploads.onUpdateEvent.trigger(uid, uid);
	};
	
	desura.events.internal.onUploadProgress = function(uid){
		desura.events.uploads.onProgressEvent.trigger(uid, uid);
	};	
	
	desura.events.internal.onUpdatePoll = function(){
		desura.events.main.onUpdatePollEvent.trigger("all");
	};

	desura.events.internal.onNewItemAdded = function(itemId, showAlert){
		desura.events.items.onNewItemAddedEvent.trigger(itemId, {itemId: itemId, alert: (showAlert == 'true')});
	};
	
	
	desura.events.internal.onChatInstantUpdate = function(){
		desura.events.chat.onInstantUpdate.trigger("all");
	};
	
	desura.events.internal.onChatProviderUpdate = function(){
		desura.events.chat.onProviderUpdate.trigger("all");
	};
	
	desura.events.internal.onChatBuddieListUpdate = function(){
		desura.events.chat.onBuddieListUpdate.trigger("all");
	};


	desura.events.internal.onChatInstaceConnect = function(id){
		desura.events.chat.onInstaceConnect.trigger(id, id);
	};
	
	desura.events.internal.onChatInstanceDisconnect = function(id){
		desura.events.chat.onInstanceDisconnect.trigger(id, id);
	};
	
	desura.events.internal.onChatEvent = function(id, type, time, info){
	
		var event = {
			fid: id, 
			type: type, 
			time: time, 
			msg: info
		};	
	
		desura.events.chat.onEvent.trigger(id, event);
	};

	desura.events.internal.onChatBuddieUpdate = function(id){
		desura.events.chat.onBuddieUpdate.trigger(id, id);
	};	
	
	desura.events.internal.onChatAddFriend = function(){
		desura.events.chat.onAddFriend.trigger("all");
	};
	
	desura.events.internal.onChatToggleOfflineContacts = function(state){
		desura.events.chat.onToggleOfflineContacts.trigger("all", state);
	};	
	
	desura.events.internal.onChatShowTab = function(id){
		desura.events.chat.onShowTab.trigger(id, id);
	};		
	
	desura.events.internal.onChatSearchComplete = function(id){
		desura.events.chat.onSearchComplete.trigger(id, id);
	};		
	
	desura.events.internal.onCIPListUpdate = function(id){
		desura.events.cip.onListUpdateEvent.trigger("all");
	};
	
	desura.events.internal.onShowAlert = function(text, time){
		desura.events.items.onShowAlertEvent.trigger("all", {text: text, time: parseInt(time)});
	};
	
	desura.events.internal.onDelayLoad = function(id){
		desura.events.items.onDelayLoadEvent.trigger("all");
	};
	
	desura.events.internal.onDelayLoadDone = function(id){
		desura.events.items.onDelayLoadDoneEvent.trigger("all");
	};
		
})();
