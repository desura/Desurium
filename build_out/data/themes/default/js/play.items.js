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

showProperties = function(item) {
	if(!isValidItem(item))
		return false;

	overlayProperties = item.getId();
	
	remove = function(){
		$("#overlayproperties,#overlay").remove();
		overlayProperties = false;
	};
	
	var exeList = item.getExeInfoList();
	var reminder = (item.getOptions() & desura.items.options.NOTREMINDUPDATE)?true:false;
	
	$("body").prepend(parseTemplate($("#gameproperties").html(), {exeList: exeList, iteminfo: item, reminder: reminder}));

	$("#overlay").click(remove);
	$("#overlayproperties").css("top", ($(window).height()-$("#overlayproperties").height())/2).css("left", ($(window).width()-$("#overlayproperties").width())/2).show();

	$("#overlayproperties .btnclose").click(remove);
	$("#overlayproperties .btnsave").click(function(){
		if ($("#overlayproperties .checkbox").is(":checked")) {
			item.addOptionFlags(desura.items.options.NOTREMINDUPDATE);
		} else {
			item.delOptionFlags(desura.items.options.NOTREMINDUPDATE);
		}

		for(x in exeList) {
			exeList[x]['userargs'] = $("#userargs" + x).val();
		}

		item.setExeUserArgs(exeList);

		remove();
	});	
};

launchItem = function(item) {
	if(!isValidItem(item)) {
		return false;
	}
	
	if (desura.utils.isOffline()) {
		var isInstalled = (item.getStatus() & desura.items.status.INSTALLED);

		if (isInstalled)
			desura.links.internalLink(item.getId(), desura.links.action.LAUNCH);
		
		return false;
	}

	if (item.isBusy()) {
		desura.links.internalLink(item.getId(), desura.links.action.LAUNCH);
		return false;
	}

	branchcount = 0;
	branches = getItemMore(item);

	for(x in branches) {
		branchcount++;
	}

	if(branchcount > 1 && isBranchUnknown(item, branchcount, playCache[item.getId()])) {
		overlayPlay = item.getId();

		$("body").prepend(parseTemplate($("#branchlist").html(), {branches: branches, iteminfo: item}));

		$("#overlay").click(function(){
			$("#overlay,#overlayplaylist").remove();
			overlayPlay = false;
		});

		$("#overlayplaylist").css("top", ($(window).height()-$("#overlayplaylist").height())/2).css("left", ($(window).width()-$("#overlayplaylist").width())/2).show();

		$("#overlayplaylist div.item").hover(function(){
			$("#overlayplaylist .on").removeClass("on");
			$(this).addClass("on");
		});

		$("#overlayplaylist div.item").click(function(){
			// launch or swap
			if($("#overlayplaylist div.content div.item").index(this) == 0) {
				desura.links.internalLink(item.getId(), desura.links.action.LAUNCH);
			} else {
				desura.links.internalLink(item.getId(), desura.links.action.SWITCHBRANCH, "branch=" + $(this).attr("rel"));
			}

			if($("#overlayplaylist input").is(":checked")) {
				playCache[item.getId()] = new Array($(this).attr("rel"), branchcount);
				jsonStr = JSON.stringify(playCache);
				desura.utils.setCacheValue('LastActivePlay', jsonStr, jsonStr.length);
			} else if(playCache[item.getId()]) {
				delete playCache[item.getId()];
				jsonStr = JSON.stringify(playCache);
				desura.utils.setCacheValue('LastActivePlay', jsonStr, jsonStr.length);
			}

			$("#overlay,#overlayplaylist").remove();
			overlayPlay = false;
		});
	} else {
		desura.links.internalLink(item.getId(), desura.links.action.LAUNCH);
	}

	return false;
};

isValidItem = function(item) {
	return (item == null || typeof(item) == "undefined" || !item.isValid()) == false;
};

isPreload = function(item) {
	if(!isValidItem(item))
		return false;

	var curBranch = item.getCurrentBranch();

	if(curBranch == null)
		return false;

	// means it maybe a preload without a release out
	if(curBranch.getBranchId() == null) {
		for(var x=0; x<item.getBranchCount(); x++) {
			var bi = item.getBranchByIndex(x);

			if (bi == null)
				continue;

			var flags = bi.getFlags();
			var isPreorder = (flags & desura.items.branch.flags.PREORDER);
			var onAccount = (flags & desura.items.branch.flags.ONACCOUNT);
			var noReleases = (flags & desura.items.branch.flags.NORELEASES);
			
			if (onAccount && isPreorder && noReleases)
				curBranch = bi;
		}
	}

	return (curBranch.getFlags() & desura.items.branch.flags.PREORDER) ? curBranch : false;
};

isPreloadAvailable = function(item) {
	curBranch = isPreload(item);

	return curBranch && !(curBranch.getFlags() & desura.items.branch.flags.NORELEASES);
};

isItemDemo = function(item) {
	if(!isValidItem(item))
		return false;

	var curBranch = item.getCurrentBranch();

	if(curBranch == null)
		return false;

	return curBranch.getFlags() & desura.items.branch.flags.DEMO;
};

isItemOwnFull = function(item) {
	if(!isValidItem(item)) {
		return false;
	}

	for(var x=0; x<item.getBranchCount(); x++) {
		var bi = item.getBranchByIndex(x);

		if(bi == null) 
			continue;

		var flags = bi.getFlags();
		var onAccount = (flags & desura.items.branch.flags.ONACCOUNT);
		var isDemo = (flags & desura.items.branch.flags.DEMO);

		if(onAccount && !isDemo)
			return true;
	}

	return false;
};

isBranchUnknown = function(item, branchcount, cache) {
	if(!isValidItem(item))
		return false;

	if(!cache) {
		return true;
	} else if(cache[0] != item.getInstalledBranch()) {
		return true;
	} else if(cache[1] != branchcount) {
		return true;
	} else {
		return false;
	}
};

getDefaultIcon = function() {
	return "../images/html/icons/icon_default.png";
};

getItemMore = function(item) {
	if(!isValidItem(item))
		return [];

	var branches = [];
	var cb = item.getCurrentBranch();
	var o=0;
	
	if (cb && cb.isValid()) {
		branches[0] = cb;
		o = 1;
	}

	for(var x=0; x<item.getBranchCount(); x++) {
		var bi = item.getBranchByIndex(x);

		if(bi == null)
			continue;

		if (bi.getFlags() & desura.items.branch.flags.NORELEASES)
			continue;
	
		if ((bi.getFlags() & desura.items.branch.flags.ONACCOUNT) && cb && bi.getBranchId() != cb.getBranchId())
			branches[x+o] = bi;
	}
	
	var realBranches = [];
	var o=0;

	for (var x1 in branches){
		var shouldAdd = true;

		for (var x2 in branches){

			if (x1 == x2)
				continue;

			var b1 = branches[x1];
			var b2 = branches[x2];

			if (b1.getName() != b2.getName())
				continue;

			//if this branch is 32 bit and the other is not use the other
			if (b1.is32Bit() && !b2.is32Bit())
				shouldAdd = false;

			break;
		}

		if (shouldAdd) {
			realBranches[o] = branches[x1];
			o += 1;
		}
	}
	
	return realBranches;
};

getItemIcon = function(item) {
	if(!isValidItem(item))
		return getDefaultIcon();

	var icon = item.getIcon();

	if(!desura.utils.isValidIcon(icon))
		return getDefaultIcon();

	return icon;	
};

getUploadIcon = function(upload) {
	if (upload == null)
		return getDefaultIcon();

	return getItemIcon(upload.getItem());
};

getUploadStatus = function(upload) {
	if (upload == null)
		return "";
		
	if(upload.isPaused())
		return desura.utils.getLocalString("#IS_PAUSED");
	else if(upload.isComplete())
		return desura.utils.getLocalString("#UDF_COMPLETE");
	else if(upload.hasError())
		return desura.utils.getLocalString("#UDF_ERROR_LABEL");

	return upload.getProgress() + "%";
};

getUploadName = function(upload) {
	if (upload == null)
		return desura.utils.getLocalString("#PLAY_INVALID_UPLOAD");

	var item = upload.getItem();
	var file = upload.getFileName();

	if (item == null) {
		var type = desura.utils.getTypeFromId(upload.getItemId());
		var id = desura.utils.getItemFromId(upload.getItemId());

		return "[Admin] " + type + " " + id + " [" + file + "]";
	} else {
		return item.getName() + " [" + file + "]";
	}
};

month_names = new Array("January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December");

getReleaseString = function(time){
	var now = new Date();
	var diff = (time.getTime()-now.getTime())/1000; //need to convert back to seconds
	var coming = desura.utils.getLocalString("#IS_PREORDER");

	if (diff > 31104000) {
		return coming + ' ' + time.getFullYear();
	} else if (diff > (2592000*3)) {
		return coming + ' Q' + Math.ceil(time.getMonth()/3) + ' ' + time.getFullYear();
	} else if (diff > 2592000) {
		return coming + ' ' + month_names[time.getMonth()] + ' ' + time.getFullYear();
	} else if (diff > 86400) {
		return coming + ' ' + Math.ceil(diff/86400) + ' ' + desura.utils.getLocalString(Math.ceil(diff/86400)==1 ? "#IS_PREORDER_DAY" : "#IS_PREORDER_DAYS");
	} else if (diff > 3600) {
		return coming + ' ' + Math.ceil(diff/3600) + ' ' + desura.utils.getLocalString(Math.ceil(diff/3600)==1 ? "#IS_PREORDER_HOUR" : "#IS_PREORDER_HOURS");
	} else {
		return coming + ' ' + Math.ceil(diff/60) + ' ' + desura.utils.getLocalString(Math.ceil(diff/60)==1 ? "#IS_PREORDER_MIN" : "#IS_PREORDER_MINS");
	}
}

getItemName = function(item, hideNameOn) {
	if(!isValidItem(item)) {
		return desura.utils.getLocalString("#PLAY_INVALID_ITEM");
	}

	var preload = isPreload(item);
	var curBranch = preload ? preload : item.getCurrentBranch();
	var curBranchValid = curBranch != null;	
	var preloadDate = preload ? curBranch.getPreorderDate() : null;

	if(hideNameOn) {
		var label = item.getName();
	} else {
		var label = item.getName();
		var branchName = curBranchValid && (curBranch.getFlags() & desura.items.branch.flags.DISPLAY_NAME) ? curBranch.getName() : "";

		if(branchName.length > 0) {
			label += ' ' + branchName;
		}
	}

	if(preloadDate != null) {
	
		var month = preloadDate.substr(4,2);
		
		if (month.charAt(0) == '0')
			month = month.charAt(1);
	
		var dto = new Date(0,0,0,0,0,0);
		dto.setUTCFullYear(preloadDate.substr(0,4));
		dto.setUTCMonth(parseInt(month)-1);
		dto.setUTCDate(preloadDate.substr(6,2));
		dto.setUTCHours(preloadDate.substr(8,2));
		dto.setUTCMinutes(preloadDate.substr(10, 2));
		
		label += ' - ' + getReleaseString(dto);
	}
	
	return label;
};

getItemVersion = function(item){
	if(!isValidItem(item)) {
		return "Invalid Item";
	}

	if (item.isDownloadable()) {
		var branch = item.getCurrentBranch().getName();
		var ver = branch + "(" + item.getInstalledBranch() + "),  Build " + item.getInstalledBuild();
	
		if (item.getStatus() & desura.items.status.UNAUTHED)
			ver += " [UnAuthed]";

		return ver;
	}

	return desura.utils.getLocalString("#P_RETAIL");
};
