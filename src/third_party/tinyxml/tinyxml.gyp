{
	'includes' : [
		'../../common.gypi'
	],
	'targets' : [
	{
		'target_name': 'tinyxml',
		'type': 'static_library',
		'defines' : [
			'_LIB',
		],
		'include_dirs': [
			'./code',
			'./include',		
		],		
		'direct_dependent_settings': {
			'include_dirs': [
				'./include',
			],			
		},		
		'cflags' : [
			'-fPIC',
		],		
		'sources': [
			'code/tinystr.cpp',
			'code/tinyxml.cpp',
			'code/tinyxmlerror.cpp',
			'code/tinyxmlparser.cpp',
		],
	}
	],
}
