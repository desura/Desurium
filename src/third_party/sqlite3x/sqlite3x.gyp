{
	'includes' : [
		'../../common.gypi',
	],
	'targets': [
	{
		'target_name': 'sqlite3x',
		'type': 'static_library',
		'include_dirs': [
			'./code',
			'./include',			
		],
		'all_dependent_settings': {
			'include_dirs': [
				'./include',
			],
		},
		'cflags' : [
			'-fPIC',
		],
		'dependencies' : [
			'<(third_party_dir)/libs.gyp:boost',
			'../sqlite/sqlite.gyp:sqlite',
		],
		'sources': [
			'code/sqlite3x_command.cpp',
			'code/sqlite3x_connection.cpp',
			'code/sqlite3x_exception.cpp',
			'code/sqlite3x_reader.cpp',
			'code/sqlite3x_transaction.cpp',
		],
	}],
}
