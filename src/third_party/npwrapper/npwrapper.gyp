{
	'target_defaults' : {
		'default_configuration': 'Build32',
		'configurations' : {
			'Build32' : {
				'defines' : [
					'NIX32',
				],
				'cflags': [
					'-m32',
				],
				'ldflags': [
					'-m32',
				],
			},
			'Build64' : {
				'defines' : [
					'NIX64',
				],
				'cflags': [
					'-m64',
				],
				'ldflags': [
					'-m64',
				],
			},
		},
	},
	'targets' : [
	{
		'target_name' : 'common',
		'type' : 'settings',
		'all_dependent_settings' : {
			'ldflags': [
				'-lpthread',
				'-ldl',
				'<!@(pkg-config --libs glib-2.0)',
				'<!@(pkg-config --libs gthread-2.0)',
				'<!@(pkg-config --libs gobject-2.0)',
			],
			'cflags': [
				'-fPIC',
				'-std=c99',
				'<!@(pkg-config --cflags glib-2.0)',
				'<!@(pkg-config --cflags gthread-2.0)',
				'<!@(pkg-config --cflags gobject-2.0)',
			],
			'include_dirs' : [
				'include',
				'src/common',
			],
		}
	},
	{
		'target_name' : 'flashwrapper',
		'type' : 'shared_library',
		'dependencies' : [
			'common',
		],
		'sources': [
			'src/wrapper/npw-wrapper.c',
			'src/common/debug.c',
			'src/common/npruntime.c',
			'src/common/npw-common.c',
			'src/common/npw-malloc.c',
			'src/common/npw-rpc.c',
			'src/common/rpc.c',
			'src/common/utils.c',
		],
		'defines': [
			'BUILD_WRAPPER',
		],
	},
	{
		'target_name' : 'flashhost',
		'type' : 'executable',
		'dependencies' : [
			'common',
		],
		'ldflags' : [
			'<!@(pkg-config gtk+-2.0 --libs)',
			'-lX11 -lXt',
		],
		'cflags' : [
			'<!@(pkg-config gtk+-2.0 --cflags)',
		],
		'sources': [
			'src/host/npw-viewer.c',
			'src/common/debug.c',
			'src/common/npruntime.c',
			'src/common/npw-common.c',
			'src/common/npw-malloc.c',
			'src/common/npw-rpc.c',
			'src/common/rpc.c',
			'src/common/utils.c',
		],
		'defines': [
			'BUILD_VIEWER',
		],
	},
	]
}
