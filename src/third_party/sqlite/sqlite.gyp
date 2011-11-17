{
	'includes' : [
		'../../common.gypi'
	],
	'targets': [
	{
		'target_name': 'sqlite',
		'type': 'static_library',
		'include_dirs': [
			'./code',
			'./include',
		],
		'all_dependent_settings': {
			'include_dirs': [
				'./include'
			],			
		},				
		'sources': [
			'code/sqlite3.c',
		],
	}],
}
