{
	'variables': {
		'third_party_dir': '../src/third_party',
		'third_party_lib_dir': 'src/third_party',
		'static_dir': '../src/static',
		'shared_dir': '../src/shared',
		'common_dir': '../src/common',
		'include_dir': '../src/include',

		'desura_nongpl%': 0,
		'desura_nongpl_dir%': 'NO_COMERCIAL_CODE_ALLOWED',
	},
	'target_defaults' : {
		'variables': {
			'third_party_dir%': '../src/third_party',
			'third_party_lib_dir%': 'src/third_party',
			'static_dir%': '../src/static',
			'shared_dir%': '../src/shared',
			'common_dir%': '../src/common',
			'include_dir%': '../src/include',
		},
		'default_configuration': 'Debug',
		'configurations' : {
			'Release' : {
				'cflags': [
					'-O2',						# full optimisations			(release only)
				],
				'defines' : [
					'NDEBUG',					# no debugging					(release only)
				],
			},
			'Debug' : {
				'cflags': [
					'-rdynamic',				# add all symbols				(debug only)
					'-fno-omit-frame-pointer', 	# don't omit frame pointer		(debug only)
					#'-Wl,-z,defs',				# resolve all symbols			(???)
				],
				'defines' : [
					'DEBUG',
					'_DEBUG',
				],
			},
		},
		'cflags': [
			'-pipe',							# use pipes not temp files for comms
			'-g3',								# max debugging
			'-fPIC',							# position independent code	
			'-fvisibility=hidden',				# hides symbols by default 		(use CEXPORT)
			'-Wall',							# warn on all possible errors
			'-Wno-unknown-pragmas',				# ignore unknown (VC) pragmas
			'-finline-functions',				# inline functions where appropriate (regardless of inline keyword)
			'-mtune=generic',					# tune code output for std IA32/AMD64 instructions
			'-march=native',					# build for current architecture
			'-mno-sse3',						# disable see3 for older systems
			'-mno-sse2',						# disable see2 for older systems
		],
		'cflags_cc': [
			'-std=c++0x',						# c++0x standards for lambda
			'-fpermissive',						# allow use before define etc	(debug only)
		],
		'defines': [
			'_LARGE_FILES',						# support large files
			'_FILE_OFFSET_BITS=64',				# support large files
		],
		'ldflags': [
			'-Wl,-Bsymbolic-functions',
		],
		'include_dirs': [
			'<(include_dir)',
			'<(common_dir)',
			'<(third_party_dir)/tinyxml/include',			
		],	
	},
}
