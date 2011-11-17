{
	'includes': [
		'../../common.gypi',
	],
	'targets': [
	{
		'target_name': 'umcf',
		'type': 'static_library',
		'include_dirs': [
			'./code',
			'./code/bzip2',
		],
		'sources': [
			'./code/UMcf.cpp',
			'./code/UMcfFile.cpp',
			'./code/UMcfFile_utils.cpp',
			'./code/UMcfHeader.cpp', 
		],
	}],
}
