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

var os = os || {};

os.nix = os.nix || {};
os.win = os.win || {};
os.win.compmode = os.win.compmode || {};

(function(){

	os.IsLinux = function() {
		native function IsLinux();
		return IsLinux();
	};
	
	os.IsWindows = function() {
		native function IsWindows();
		return IsWindows();
	};
	
	os.win.SetRegistryKey = function(key, data) {
		native function SetRegistryKey();
		SetRegistryKey(key, data);
	};
	
	os.win.DelRegistryKey = function(key) {
		native function DelRegistryKey();
		DelRegistryKey(key);
	};

	os.win.GetRegistryKey = function(key) {
		native function GetRegistryKey();
		return GetRegistryKey(key);
	};	
	
	os.win.SetFirewallAllow = function(exePath, name) {
		native function SetFirewallAllow();
		return SetFirewallAllow(exePath, name);
	};

	os.win.DelFirewallAllow = function(exePath) {
		native function DelFirewallAllow();
		DelFirewallAllow(exePath);
	};		
	
	os.win.SetCompatiblityMode = function(path, osType, flags) {
		native function SetCompatiblityMode();
		
		if (typeof osType == 'undefined')
			osType = os.win.compmode.IGNORE;
			
		if (typeof flags == 'undefined')
			flags = os.win.compmode.IGNORE;
		
		SetCompatiblityMode(path, osType, flags);
	};
	
	os.win.compmode.IGNORE = -1;
	os.win.compmode.OS_WIN95 = 0;
	os.win.compmode.OS_WIN98 = 1;
	os.win.compmode.OS_NT4SP5 = 2;
	os.win.compmode.OS_WIN2000 = 3;
	
	os.win.compmode.FLAG_256COLOR 			= 1<<1;
	os.win.compmode.FLAG_640X480 			= 1<<2;
	os.win.compmode.FLAG_DISABLE_THEMES 	= 1<<3;
	os.win.compmode.FLAG_DISABLE_DWM 		= 1<<4;
	os.win.compmode.FLAG_RUN_AS_ADMIN 		= 1<<5;
	os.win.compmode.FLAG_HIGH_DPI_AWARE 	= 1<<6;

})();
