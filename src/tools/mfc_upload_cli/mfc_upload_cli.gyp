{
	'includes': [
		'../../../build_lin/common.gypi',
	],
	'targets': [
	{
		'target_name': 'mfc_upload_cli',
		'type': 'executable',
		'dependencies' : [
			'<(shared_dir)/usercore/usercore.gyp:usercore',
			'<(static_dir)/managers/managers.gyp:*',
			'<(static_dir)/util_thread/util_thread.gyp:threads',
			'<(static_dir)/util_fs/util_fs.gyp:util_fs',
			'<(static_dir)/util/util.gyp:*',
		],
		'ldflags': [
			'-lboost_system-desura',
			'-lboost_date_time-desura',
		],
		'sources' : [
			'code/main.cpp',
		],
	}],
}
