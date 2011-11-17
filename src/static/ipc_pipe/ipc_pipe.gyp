{
	'includes': [
		 '../../common.gypi',
	],
	'targets': [
	{
		'target_name': 'ipc_pipe',
		'type': 'static_library',
		'include_dirs': [
			'./code',
		],
		'sources': [
			'code/IPCClass.cpp',
			'code/IPCManager.cpp',
			'code/IPCParameter.cpp',
			'code/IPCPipeBase_Nix.cpp',
			'code/IPCPipeClient_Nix.cpp',
			'code/IPCPipeServer_Nix.cpp',
		],
		'dependencies': [
			'<(static_dir)/util_thread/util_thread.gyp:threads',
			'<(static_dir)/util/util.gyp:util',
		],
		'direct_dependent_settings': {
			'include_dirs': [
				'./code',
			],
		},		
	}],
}
