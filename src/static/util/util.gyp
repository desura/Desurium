{
	'includes': [
		 '../../../build_lin/common.gypi',
	],
	'targets': [
	{
		'variables' : {
			'curl_lib': 1,
		},
		'target_name': 'util_web',
		'type': 'static_library',
		'include_dirs': [
			'./code',
		],
		'dependencies': [
			'<(third_party_dir)/libs.gyp:curl',
		],
		'sources': [
			'code/UtilWeb.cpp',
		],
	},
	{
		'target_name': 'util',
		'type': 'static_library',
		'include_dirs': [
			'./code',
		],
		'dependencies' : [
			'<(third_party_dir)/sqlite3x/sqlite3x.gyp:sqlite3x',
			'<(third_party_dir)/libs.gyp:boost',
			'<(third_party_dir)/bzip2/bzip2.gyp:bzip2'
		],		
		'sources': [
			'code/gcBuff.cpp',
			'code/third_party/GeneralHashFunctions.cpp',
			'code/third_party/md5.cpp',
			'code/MD5Progressive.cpp',
			'code/MD5Wrapper.cpp',
			'code/UtilBZip2.cpp',
			'code/UtilCrypto.cpp',
			'code/UtilFsPath.cpp',
			'code/UtilMisc.cpp',
			'code/UtilMisc_sha1.cpp',
			'code/UtilOs.cpp',
			'code/UtilString.cpp',
			'code/UtilLinux.cpp',
		],
	}],
}
