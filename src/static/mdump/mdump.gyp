{
	'includes': [
		'../../common.gypi',
	],
	'targets': [
	{
		'target_name': 'mdump',
		'type': 'static_library',
		'dependencies' : [
			'<(third_party_dir)/libs.gyp:breakpad',
		],
		'sources' : [
			'code/MiniDumpGenerator_lin.cpp',
		],
	},
	],
}
