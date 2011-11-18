{
	'includes': [
		'../../../build_lin/common.gypi',
	],
	'targets': [
	{
		'variables' : {
			'curl_static': 1,
			'breakpad_static': 1,
		},
		'target_name': 'desura',
		'type': 'executable',
		'ldflags': [
			'-lrt',										# needed for 64 bit build
			'-lpthread',
			'-static-libgcc',
			'-static-libstdc++',
			'-ldl',
			'-lX11',
			'-lXpm',
			'-Wl,-Bsymbolic-functions',
			'-Wl,--print-gc-sections',					# garbage collect unused section and show whats being removed
		],
		'include_dirs': [
			'./code',
		],
		'dependencies': [
			'<(third_party_dir)/libs.gyp:curl',
			'<(third_party_dir)/libs.gyp:breakpad',
			'<(third_party_dir)/tinyxml/tinyxml.gyp:tinyxml',
			'<(static_dir)/util/util.gyp:*',
			'<(static_dir)/umcf/umcf.gyp:umcf',
			'<(static_dir)/mdump/mdump.gyp:mdump',
		],
		'sources': [
			'code/main.cpp',
			'code/update.cpp',
			'code/UpdateForm.cpp',
			'code/files.cpp',
			'code/UMcfEx.cpp',
			'code/XLibWindow.cpp',		
		],
	}],
}

