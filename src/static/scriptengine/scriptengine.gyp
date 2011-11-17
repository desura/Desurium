{
	'includes': [
		'../../common.gypi',
	],
	'targets': [
	{
		'target_name': 'scriptengine',
		'type': 'static_library',
		'dependencies' : [
			'<(static_dir)/util_thread/util_thread.gyp:threads',
			'<(static_dir)/util_fs/util_fs.gyp:util_fs',
			'<(static_dir)/util/util.gyp:util',		
			'<(common_dir)/gcJSBase.gyp:gcJSBase',
			'<(third_party_dir)/libs.gyp:v8',
		],
		'include_dirs' : [
			'./RES',
			'./code',
		],
		'sources' : [
			'code/jsExtensions.cpp',
			'code/jsFS.cpp',
			'code/jsItem.cpp',
			'code/jsWin.cpp',
			'code/ScriptCore.cpp',
			'code/ScriptCoreInternal.h',
			'code/ScriptCoreHelper.cpp',
		],
	}],
}
