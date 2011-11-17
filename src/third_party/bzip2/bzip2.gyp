{
	'includes' : [
		'../../common.gypi',
	],
	'targets': [
	{
		'target_name': 'bzip2',
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
		'sources': [
			'code/blocksort.c',
			'code/bzlib.c',
			'code/compress.c',
			'code/crctable.c',
			'code/decompress.c',
			'code/huffman.c',
			'code/randtable.c',
		],
	}],
}
