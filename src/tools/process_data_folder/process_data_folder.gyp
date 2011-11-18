{
	'includes': [
		'../../../build_lin/common.gypi',
	],
	'targets': [
	{
		'target_name': 'process_data_folder',
		'type': 'executable',
		'dependencies' : [
			'<(static_dir)/util_fs/util_fs.gyp:util_fs',
			'<(static_dir)/util/util.gyp:util',
			'<(third_party_dir)/libs.gyp:boost',
		],
		'ldflags': [
			'-lboost_system-desura',
		],
		'sources' : [
			'code/main.cpp',
		],
	}],
}
