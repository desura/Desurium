{
	'includes': [
		 '../../../build_lin/common.gypi',
	],
	'targets': [
	{
		'target_name': 'webcore',
		'type': 'shared_library',
		'dependencies' : [
			'<(static_dir)/util_thread/util_thread.gyp:threads',
			'<(static_dir)/util_fs/util_fs.gyp:util_fs',
			'<(static_dir)/util/util.gyp:*',
			'<(third_party_dir)/tinyxml/tinyxml.gyp:tinyxml',
			'<(third_party_dir)/sqlite3x/sqlite3x.gyp:sqlite3x',
			'<(third_party_dir)/libs.gyp:curl_lib',
		],
		'include_dirs': [
			'./code',
			'./RES',
		],
		'sources': [
			'code/ImageCache.cpp',
			'code/WebCore.cpp',
			'code/WebCore_External.cpp',
			'code/WebCoreMain.cpp',
			'code/Log.cpp',
			'code/WebCore_Internal.cpp',
		],
		'conditions' : [
			['desura_nongpl==1',{
				'dependencies' : [
					'<(desura_nongpl_dir)/shared/webcore/webcore.gyp:webcore_nongpl',
				]
			}]
		],
	}],
}
