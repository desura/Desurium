{
	'includes': [
		 '../common.gypi',
	],
	'targets': [
	{
		'target_name': 'gcJSBase',
		'type': 'static_library',
		'include_dirs' : [
			'../third_party/cef',
		],
		'sources': [
			'./gcJSBase.cpp',
		],
		'direct_dependent_settings': {
			'include_dirs': [
				'../third_party/cef',
			],
		},
	}],
}
