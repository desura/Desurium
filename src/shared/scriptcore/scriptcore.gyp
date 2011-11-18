{
	'includes': [
		'../../../build_lin/common.gypi',
	],
	'targets': [
	{
		'target_name': 'scriptcore',
		'type': 'shared_library',
		'dependencies' : [
			'<(static_dir)/util_thread/util_thread.gyp:threads',
			'<(static_dir)/util_fs/util_fs.gyp:util_fs',
			'<(static_dir)/util/util.gyp:util',		
			'<(common_dir)/gcJSBase.gyp:gcJSBase',
			'<(third_party_dir)/libs.gyp:v8',
			'<(static_dir)/scriptengine/scriptengine.gyp:scriptengine',	
		],
		'include_dirs' : [
			'./RES',
			'./code',
		],
		'sources' : [
			'code/Log.cpp',
			'code/ScriptCoreMain.cpp',
		],
	}],
}
