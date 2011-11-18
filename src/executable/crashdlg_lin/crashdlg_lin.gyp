{
	'includes': [
		'../../../build_lin/common.gypi',
	],
	'targets': [
	{
		'target_name': 'crashdlg',
		'type': 'executable',
		'dependencies' : [
			'<(static_dir)/util/util.gyp:util',
			'<(third_party_dir)/libs.gyp:gtk',
		],
		'sources' : [
			'code/main.cpp',
		],
		'ldflags' : [
			'-ldl',
		],
	}],
}
