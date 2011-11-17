{
	'includes': [
		'../../common.gypi',
	],
	'targets': [
	{
		'target_name': 'bittest',
		'type': 'executable',
		'cflags': [
			'-m32',
		],
		'ldflags': [
			'-m32',
			'-static-libgcc',
			'-static-libstdc++',
		],
		'sources': [
			'code/main.c',	
		],
	}],
}

