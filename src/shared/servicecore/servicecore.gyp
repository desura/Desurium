{
	'includes': [
		'../../../build_lin/common.gypi',
		'../../common/service_pipe.gypi',
	],
	'targets': [
	{
		'target_name': 'servicecore',
		'type': 'shared_library',
		'dependencies' : [
			'<(static_dir)/util_thread/util_thread.gyp:threads',
			'<(static_dir)/util_fs/util_fs.gyp:util_fs',
			'<(static_dir)/util/util.gyp:util',
			'<(static_dir)/umcf/umcf.gyp:umcf',
			'<(third_party_dir)/sqlite/sqlite.gyp:sqlite',
			'<(third_party_dir)/sqlite3x/sqlite3x.gyp:sqlite3x',
			'<(third_party_dir)/tinyxml/tinyxml.gyp:tinyxml',
			'<(common_dir)/gcJSBase.gyp:gcJSBase',
		],
		'include_dirs' : [
			'./code',
			'./RES',
		],
		'sources' : [
			'code/ComplexLaunchProcess.cpp',
			'code/InstallProcess.cpp',
			'code/InstallScriptRunTime.cpp',
			'code/Log.cpp',
			'code/ServiceCoreMain.cpp',
			'code/McfInit.cpp',
			'code/ServiceCore.cpp',
			'code/ServiceMainThread.cpp',
			'code/UnInstallBranchProcess.cpp',
			'code/UnInstallProcess.cpp',
			'code/UnInstallRegKey.cpp',
			'code/UpdateProcess.cpp',
			'code/UpdateShortcuts.cpp',
		],
	}],
}
