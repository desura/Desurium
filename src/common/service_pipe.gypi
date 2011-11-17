{
	'target_defaults' : {
		'dependencies' : [
			'<(static_dir)/ipc_pipe/ipc_pipe.gyp:ipc_pipe',
		],
		'include_dirs' : [
			'<(static_dir)/ipc_pipe/code',
			'service_pipe',
		],		
		'sources': [
			'service_pipe/IPCComplexLaunch.cpp',
			'service_pipe/IPCInstallMcf.cpp',
			'service_pipe/IPCServiceMain.cpp',
			'service_pipe/IPCUninstallBranch.cpp',
			'service_pipe/IPCUninstallMcf.cpp',
			'service_pipe/IPCUpdateApp.cpp',
		],
	}
}
