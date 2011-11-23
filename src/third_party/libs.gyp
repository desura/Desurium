{
	'includes' : [
		'../../build_lin/common.gypi'
	],
	'targets': [
	{
		'target_name': 'breakpad',
		'type': 'settings',
		'variables': {
			'breakpad_static%': 0,
		},
		
		'direct_dependent_settings':
		{
			'include_dirs': [
				'breakpad/src'
			],
			'libraries': [
				'>(third_party_lib_dir)/breakpad/src/client/linux/.libs/libbreakpad_client.a',
			],			
			'conditions' : [
				['breakpad_static==1', {		
					'cflags': [
						'-static',								# build a static executable
					],
				}]
			]
		},
	},	
	{
		'target_name': 'curl',
		'type': 'settings',
		'variables': {
			'curl_static%': 0,
		},
		'all_dependent_settings':
		{	
			'variables': {
				'curl_static%': 0,
			},
			'target_conditions' : [		
				['curl_static==1', {
					'cflags': [
						'-static',								# build a static executable
					],
					'ldflags': [
						'-lz',
					],
					'include_dirs': [
						'./curl-static/include'
					],				
				},{
					'include_dirs': [
						'./curl-shared/include'
					],
					'ldflags': [
						'-lcares_desura',
						'-lcurl_desura',
						'-L>(third_party_lib_dir)/curl-shared/lib/.libs',
						'-L>(third_party_lib_dir)/c-ares/.libs',
					],	
				}]
			],
		},
		'link_settings':
		{		
			'target_conditions' : [
				['curl_static==1', {
					'libraries': [
						'>(third_party_lib_dir)/curl-static/lib/.libs/libcurl.a',
						'/usr/lib/librt.a',
					],
				}]
			],
		}
	},
	{
		'target_name': 'curl_lib',
		'type': 'settings',	
		'all_dependent_settings':
		{	
			'ldflags': [
				'-lcares_desura',
				'-lcurl_desura',
				'-L>(third_party_lib_dir)/curl-shared/lib/.libs',
				'-L>(third_party_lib_dir)/c-ares/.libs',
			],	
		},
	},
	{
		'target_name' : 'wxWidgets',
		'type': 'settings',
		'all_dependent_settings' : {
			'cflags' : [
				'<!@(pkg-config gtk+-2.0 --cflags)',	# GTK+2.0 include directories and defines
			],
			'configurations' : {
				'Release' : {
					'cflags': [
						'<!(<(third_party_dir)/wxWidgets/wx-config --debug=no --cppflags)',
					],
					'ldflags' : [
						'<!(<(third_party_dir)/wxWidgets/wx-config --debug=no --libs)',
					],
					'include_dirs': [
						'./wxWidgets/include',
					],					
				},
				'Debug' : {
					'cflags': [
						'<!(<(third_party_dir)/wxWidgets_debug/wx-config --debug=yes --cppflags)',
					],
					'ldflags' : [
						'<!(<(third_party_dir)/wxWidgets_debug/wx-config --debug=yes --libs)',
					],
					'include_dirs': [
						'./wxWidgets_debug/include',
					],					
				}
			},
		}
	},
	{
		'target_name': 'gtk',
		'type': 'settings',
		'all_dependent_settings' : {
			'cflags' : [
				'<!@(pkg-config gtk+-2.0 --cflags)',	# GTK+2.0 include directories and defines
			],
			'ldflags' : [
				'<!@(pkg-config gtk+-2.0 --libs)',
			],			
		}
	},
	{
		'target_name' : 'boost',
		'type': 'settings',
		'all_dependent_settings' : {
			'ldflags': [
				'-L>(third_party_lib_dir)/boost/stage/lib',
			],
			'include_dirs': [
				'./boost',
			],
			'defines' : [
				'BOOST_FILESYSTEM_VERSION=2'
			],
		}
	},
	{
		'target_name' : 'v8',
		'type': 'settings',
		'all_dependent_settings' : {
			'ldflags': [
				'-L>(third_party_lib_dir)/v8',
			],
			'include_dirs': [
				'./v8/include',
			],
			'defines' : [
				'USING_V8_SHARED',
			],
			'configurations' : {
				'Release' : {
					'ldflags' : [
						'-lv8-s',
					],
				},
				'Debug' : {
					'ldflags' : [
						'-lv8_g',
					],
				}
			},			
		}
	},		
	],
}
