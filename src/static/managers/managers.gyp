{
	'includes': [
		 '../../../build_lin/common.gypi',
	],
	'targets': [
	{
		'target_name': 'managers',
		'type': 'static_library',
		'include_dirs': [
			'./code',
		],
		'dependencies': [
			'<(third_party_dir)/sqlite3x/sqlite3x.gyp:sqlite3x',
		],		
		'sources': [
			'code/ConCommand.cpp',
			'code/ConCommandManager.cpp',
			'code/CVar.cpp',
			'code/CVarManager.cpp',
			'code/LanguageManager.cpp',
			'code/Managers.cpp',
			'code/Theme.cpp',
			'code/ThemeManager.cpp',
			'code/WildcardManager.cpp',
		],
	},
	{
		'target_name' : 'managers_window',
		'type' : 'static_library',
		'dependencies' : [
			'<(third_party_dir)/libs.gyp:wxWidgets',
		],
		'include_dirs' : [
			'./code',
		],
		'defines' : [
			'USING_WX_WIDGETS',
		],
		'sources' : [
			'code/WindowManager.cpp',
		],
	}
	],
}
