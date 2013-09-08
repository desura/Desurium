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

var fs = fs || {};

(function(){

	fs.IsValidFile = function(file) {
		native function IsValidFile();
		return IsValidFile(file);
	};
	
	fs.IsValidFolder = function(folder) {
		native function IsValidFolder();
		return IsValidFolder(folder);
	};
	
	fs.CopyFile = function(src, dest) {
		native function CopyFile();
		return CopyFile(src, dest);
	};
	
	fs.DeleteFile = function(file) {
		native function DeleteFile();
		return DeleteFile(file);
	};
	
	fs.DeleteFolder = function(path) {
		native function DeleteFolder();
		return DeleteFolder(path);
	};
	
	fs.SetFolderPermissions = function(folder) {
		native function SetFolderPermissions();
		return SetFolderPermissions(folder);
	};
	
	fs.GetFileSize = function(file) {
		native function GetFileSize();
		return GetFileSize(file);
	};
	
	fs.OpenFileForWrite = function(file) {
		native function OpenFileForWrite();
		return OpenFileForWrite(file);
	};
	
	fs.WriteFile = function(handle, string) {
		native function WriteFile();
		return WriteFile(handle, string);
	};
	
	fs.CloseFile = function(handle) {
		native function CloseFile();
		return CloseFile(handle);
	};
})();
