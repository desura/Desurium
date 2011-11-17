{
	'includes': [
		'../../common.gypi',
	],
	'targets': [
	{
		'target_name': 'threads',
		'type': 'static_library',
		'include_dirs': [
			'./code',
		],
		'dependencies': [
			'<(third_party_dir)/libs.gyp:boost',
		],
		'direct_dependent_settings' : {
			'ldflags': [
				'-lboost_thread-desura',
				'-lpthread',
			],
		},		
		'sources': [
			'./code/BaseThread.cpp',
			'./code/ThreadPool.cpp',
			'./code/ThreadPoolThread.cpp',				 
		],		
	}],
}
