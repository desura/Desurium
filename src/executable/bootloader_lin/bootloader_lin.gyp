{
	'includes': [
		'../../../build_lin/common.gypi',
	],
	'targets': [
	{
		'variables' : {
			'breakpad_static': 1,
		},
		'target_name': 'desura',
		'type': 'executable',
		'ldflags': [
			'-lrt',										# needed for 64 bit build
			'-static-libgcc',
			'-static-libstdc++',
			'-ldl',
			'-Wl,-Bsymbolic-functions',
			'-Wl,--print-gc-sections',					# garbage collect unused section and show whats being removed
		],
		'include_dirs': [
			'./code',
		],
		'dependencies': [
			'<(third_party_dir)/libs.gyp:breakpad',
			'<(static_dir)/util/util.gyp:*',
			'<(static_dir)/mdump/mdump.gyp:mdump',
		],
		'sources': [
			'code/DesuraMain.cpp',
			'code/UtilFile.cpp',
		],
		'conditions' : [
			['desura_nongpl==1',{
				'dependencies' : [
					'<(desura_nongpl_dir)/executable/bootloader_lin/bootloader_lin.gyp:bootloader_lin_nongpl',
				]
			}]
		],		
	}],
}

