{
	'includes': [
		 '../../common.gypi',
	],
	'targets': [
	{
		'target_name': 'util_fs',
		'type': 'static_library',
		'include_dirs': [
			'./code',
		],
		'dependencies': [
			'<(third_party_dir)/libs.gyp:boost',
		],
		'direct_dependent_settings' : {
			'ldflags': [
				'-lboost_filesystem-desura',
			],			
		},
		'sources': [
			'./code/UtilFs.cpp',
			'./code/UtilFs_nix.cpp',
		],
	}],
}
