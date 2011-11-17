{
	'includes': [
		'../../common.gypi',
	],
	'targets': [
	{
		'target_name': 'crashuploader',
		'type': 'shared_library',
		'dependencies' : [
			'<(static_dir)/util_fs/util_fs.gyp:util_fs',
			'<(static_dir)/util/util.gyp:*',		
			'<(third_party_dir)/tinyxml/tinyxml.gyp:tinyxml',
			'<(third_party_dir)/libs.gyp:curl',
			'<(third_party_dir)/libs.gyp:gtk',
		],
		'include_dirs' : [
			'./RES',
			'./code',
		],
		'sources' : [
			'code/CrashuploaderMain.cpp',
		],
	}],
}
