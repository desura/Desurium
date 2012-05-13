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

createToolMenu = function(item) {
	if (item == null || typeof(item) == "undefined" || !item.isValid()) {
		return null;
	}

	var tool = new desura.contextmenu();

	if(item.getStatus() & desura.items.status.DEVELOPER) {
		tool.appendItem(desura.utils.getLocalString("#PM_DESURAPAGE"), function() {
			desura.links.internalLink(item.getId(), desura.links.action.DEVPAGE);
		});
		
		tool.appendItem(desura.utils.getLocalString("#PM_CREATEMCF"), function() {
			desura.links.internalLink(item.getId(), desura.links.action.CREATE);
		});
		
		tool.appendItem(desura.utils.getLocalString("#PM_UPLOADMCF"), function() {
			desura.links.internalLink(item.getId(), desura.links.action.UPLOAD);
		});
	}

	return tool;
};

getBranchOsName = function(x, branches) {

	var osName = "";

	for (y in branches) {
		if (x == y)
			continue;

		if (branches[x].getName() == branches[y].getName()) {
			var os = "";
			var ver = "";

			if (branches[x].isLinux())					
				os = "Linux";
			else if (branches[x].isWindows())
				os = "Windows";

			if (branches[x].is64Bit())
				ver = "x64";
			else if (branches[x].is32Bit())
				ver = "x32";

			osName = " (" + os + " " + ver + ")";
			break;
		}
	}

	return osName;
}

createBranchMenu = function(item) {
	if (item == null)
		return null;

	var count = 0;
	var branch = new desura.contextmenu();	
	var isLink = (item.getStatus() &  desura.items.status.LINK)?true:false;

	if(isLink) {
		branch.appendCheckboxItem(desura.utils.getLocalString("#LINK"), function(){}, true, true);
		count++;
	}

	var cb = item.getCurrentBranch();

	var branches = [];

	for (var x=0; x<item.getBranchCount(); x++) {
		var bi = item.getBranchByIndex(x);

		if (bi == null)
			continue;
	
		var flags = bi.getFlags();
		var onAccount = (flags & desura.items.branch.flags.ONACCOUNT);

		if(!onAccount && cb != null && bi.getBranchId() != cb.getBranchId()) {
			var isFree 		= (flags & desura.items.branch.flags.FREE);
			var hasReleases = !(flags & desura.items.branch.flags.NORELEASES);
			var isLocked 	= (flags & (desura.items.branch.flags.MEMBERLOCK|desura.items.branch.flags.REGIONLOCK));

			if (isLocked || !isFree || !hasReleases)
				continue;
		}

		branches[x] = bi;
	}

	for (x in branches) {

		var bi = branches[x];
		var checked = (!isLink && cb != null && bi.getBranchId() == cb.getBranchId());

		var branchAction = (function(bi) {
			return function() {
				var branchId = bi.getBranchId().toString();

				var branchcount = 0;
				var barray = getItemMore(item);

				if (typeof(barray.length) != 'undefined')
					branchcount = barray.length;

				playCache[item.getId()] = new Array(branchId, branchcount);
				jsonStr = JSON.stringify(playCache);
				desura.utils.setCacheValue('LastActivePlay', jsonStr, jsonStr.length);
				desura.links.internalLink(item.getId(), desura.links.action.SWITCHBRANCH, "branch=" + branchId);
			}
		})(bi);
	
		var branchName = bi.getName() + getBranchOsName(x, branches);
		branch.appendCheckboxItem(branchName, branchAction, checked, checked);
	}

	return branches.length > 1 ? branch : new desura.contextmenu();
};

onItemContextMenu = function(event, itemId) {
	var item = desura.items.getItemFromId(itemId);

	if(item == null || typeof(item) == "undefined" || !item.isValid()) {
		return;
	}

	var cb = item.getCurrentBranch();
	
	var menu = new desura.contextmenu();
	var status = item.getStatus();
	var enableProps = (status & (desura.items.status.READY|desura.items.status.INSTALLED));
	var onAccount = (status & desura.items.status.ONACCOUNT);
	var isPausable = (status & desura.items.status.PAUSABLE);
	var isPaused = (status & desura.items.status.PAUSED);
	var isDownloadable = item.isDownloadable();
	var hasCDKey = cb != null && (cb.getFlags() & desura.items.branch.flags.CDKEY);
	var isInstalled = (item.getStatus() & (desura.items.status.INSTALLED|
										desura.items.status.DOWNLOADING|
										desura.items.status.INSTALLING|
										desura.items.status.VERIFING|
										desura.items.status.UPDATING))?true:false;
	var isLink = (status & desura.items.status.LINK);
	var isAppLink = item.isLink();
	
	var branchMenu = null;
	var toolMenu = createToolMenu(item);

	var exeList = item.getExeInfoList();
	var isBought = false;

	for (var x=0; x<item.getBranchCount(); x++) {
		var bi = item.getBranchByIndex(x);

		if (bi == null)
			continue;
	
		var flags = bi.getFlags();
		var onAccount = (flags & desura.items.branch.flags.ONACCOUNT);
		var isFree = (flags & desura.items.branch.flags.FREE);
		var isDemo = (flags & desura.items.branch.flags.DEMO);
		var isTest = (flags & desura.items.branch.flags.TEST);

		if (onAccount && !isFree && !isDemo && !isTest) {
			isBought = true;
			break;
		}
	}
	
	menu.appendItem(desura.utils.getLocalString("#LAUNCH"), function() {
		desura.links.internalLink(itemId, desura.links.action.LAUNCH);
	}, !item.isLaunchable());		

	if (exeList.length > 1) {
	
		for (var x=0; x<exeList.length; x++) {
		
			var exeAction = (function(x) {
				return function() {	
					var i = x;
					desura.links.internalLink(itemId, desura.links.action.LAUNCH, "exe=" + exeList[i]['name']);
				}
			})(x);
		
			menu.appendItem(exeList[x]['name'], exeAction, !item.isLaunchable());	
		}
	}
	
	if(desura.utils.isOffline()) {
		menu.show(event);
		return;
	}

	if(isDownloadable) {
		if(isPausable) {
			var label = desura.utils.getLocalString("#PAUSE");
			var action = desura.links.action.PAUSE;

			if(isPaused) {
				label = desura.utils.getLocalString("#RESUME");
				action = desura.links.action.UNPAUSE;
			}

			menu.appendItem(label, function() {
				desura.links.internalLink(itemId, action);
			});
		}
	}

	menu.appendSeparator();
	
	if(!onAccount && !isAppLink) {
		menu.appendItem(desura.utils.getLocalString("#PM_ADDACCOUNT"), function() {
			desura.links.internalLink(itemId, desura.links.action.ACCOUNTSTAT);
		});
	}

	if(!isAppLink && !isInstalled && !isDownloadable) {
		menu.appendItem(desura.utils.getLocalString("#PM_INSTALLCHECK"), function() {
			desura.links.internalLink(itemId, desura.links.action.INSCHECK);
		});	
	} else if(!isAppLink && !isInstalled) {
		menu.appendItem(desura.utils.getLocalString("#INSTALL"), function() {
			desura.links.internalLink(itemId, desura.links.action.INSTALL);
		});
	}

	if (!isBought || isInstalled) {
		var t = "#UNINSTALL";
	
		if (isAppLink)
			t = "#REMOVE";
		else if (!isInstalled && onAccount)
			t = "#PM_REMOVEFROMACCOUNT";
	
		menu.appendItem(desura.utils.getLocalString(t), function() {
			desura.links.internalLink(itemId, desura.links.action.UNINSTALL);
		});
	}

	if(isInstalled && isDownloadable) {
		menu.appendItem(desura.utils.getLocalString("#P_VERIFY"), function() {
			desura.links.internalLink(item.getId(), desura.links.action.VERIFY);
		});
	}

	installedMod = desura.items.getItemFromId(item.getInstalledModId());

	if(isInstalled && installedMod != null && installedMod.isValid() && item.getType() == desura.items.type.GAME) {
		menu.appendItem(desura.utils.getLocalString("#PM_REVERTCOMPLEX"), function() {
			desura.links.internalLink(item.getId(), desura.links.action.CLEANCOMPLEXMOD);
		});
	}

	menu.appendSeparator();		

	menu.appendItem(desura.utils.getLocalString("#PM_SHORTCUT"), function(){
		item.createDesktopShortcut();
	});

	menu.appendItem(desura.utils.getLocalString("#PM_MENU_ENTRY"), function(){
		item.createMenuEntry();
	});
	
	if(hasCDKey) {
		menu.appendItem(desura.utils.getLocalString("#PM_SHOWCDKEY"), function() {
			desura.links.internalLink(itemId, desura.links.action.DISPCDKEY);
		});
	}	
	
	if(isDownloadable) {
		menu.appendItem(desura.utils.getLocalString("#PM_UPDATELOG"), function() {
			desura.links.internalLink(itemId, desura.links.action.UPDATELOG);
		});

		branchMenu = createBranchMenu(item);
	}

	if (item.getProfile().length != 0) {
		menu.appendItem(desura.utils.getLocalString("#PM_VIEWPROFILE"), function() {
			desura.links.internalLink(itemId, desura.links.action.PROFILE);
		});
	}

	if(item.getType() == desura.items.type.GAME && (item.getStatus() & desura.items.status.DLC)) {
		menu.appendItem(desura.utils.getLocalString("#PM_MODLIST"), function() {
			desura.links.internalLink(itemId, desura.links.action.PROFILE, "/mods");
		});			
	}
	
	menu.appendSeparator();

	menu.appendCheckboxItem(desura.utils.getLocalString(item.isFavorite() ? "#PM_FAVORITE_REMOVE" : "#PM_FAVORITE"), function() {
		item.setFavorite(!item.isFavorite());
	}, item.isFavorite());

	menu.appendItem(desura.utils.getLocalString("#PM_PROPERTIES"), function() {
		showProperties(item);
	}, !enableProps);		

	menu.appendMenu(toolMenu, desura.utils.getLocalString("#MENU_TOOLS"));
	menu.appendMenu(branchMenu, desura.utils.getLocalString("#MENU_VERSION"));

	
	menu.show(event);
};

onUploadContextMenu = function(event, upload) {

	if (upload == null)
		return;

	var contextMenu = new desura.contextmenu();
	var resume = desura.utils.getLocalString("#RESUME");

	var started = upload.hasStarted()
	var paused = upload.isPaused();
	
	if (started && !paused)
		resume = desura.utils.getLocalString("#PAUSE");

	contextMenu.appendItem(resume, function() {
		if (started == false) {
			desura.links.internalLink(upload.getItemId(), desura.links.action.RESUPLOAD, upload.getuid());
		} 
		else {
		
			if (paused)
				upload.unpause();
			else
				upload.pause();
		}
	});

	contextMenu.appendItem(desura.utils.getLocalString("#CANCEL"), function() { 
		upload.cancel(); 
	});

	contextMenu.appendSeparator();

	contextMenu.appendItem(desura.utils.getLocalString("#PM_DEVPROFILE"), function() { 
		desura.links.internalLink(upload.getItemId(), desura.links.action.DEVPAGE); 
	});

	contextMenu.appendItem(desura.utils.getLocalString("#PM_SHOWMCF"), function() { 
		upload.exploreMcf(); 
	});	

	contextMenu.appendSeparator();

	contextMenu.appendCheckboxItem(desura.utils.getLocalString("#PM_DELMCF"), function() { 
		upload.setDeleteMcf(!upload.shouldDeleteMcf()); 
	}, upload.shouldDeleteMcf());

	contextMenu.show(event);
};
