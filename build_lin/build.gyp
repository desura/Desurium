{
	'includes' : [
		'../src/common.gypi'
	],
	'targets' : [
	{
		'target_name' : 'ipc_pipe',
		'type' : 'none',
		'dependencies' : [
			'../src/static/ipc_pipe/ipc_pipe.gyp:*',
		]
	},
	{
		'target_name' : 'managers',
		'type' : 'none',
		'dependencies' : [
			'../src/static/managers/managers.gyp:*',
		]
	},
	{
		'target_name' : 'mdump',
		'type' : 'none',
		'dependencies' : [
			'../src/static/mdump/mdump.gyp:*',
		]
	},
	{
		'target_name' : 'util_thread',
		'type' : 'none',
		'dependencies' : [
			'../src/static/util_thread/util_thread.gyp:*',
		]
	},
	{
		'target_name' : 'umcf',
		'type' : 'none',
		'dependencies' : [
			'../src/static/umcf/umcf.gyp:*',
		]
	},
	{
		'target_name' : 'util',
		'type' : 'none',
		'dependencies' : [
			'../src/static/util/util.gyp:*',
		]
	},
	{
		'target_name' : 'util_fs',
		'type' : 'none',
		'dependencies' : [
			'../src/static/util_fs/util_fs.gyp:*',
		]
	},
	{
		'target_name' : 'wx_controls',
		'type' : 'none',
		'dependencies' : [
			'../src/static/wx_controls/wx_controls.gyp:*',
		]
	},
	
	{
		'target_name' : 'mcfcore',
		'type' : 'none',
		'dependencies' : [
			'../src/shared/mcfcore/mcfcore.gyp:*',
		]
	},
	{
		'target_name' : 'servicecore',
		'type' : 'none',
		'dependencies' : [
			'../src/shared/servicecore/servicecore.gyp:*',
		]
	},	
	{
		'target_name' : 'scriptcore',
		'type' : 'none',
		'dependencies' : [
			'../src/shared/scriptcore/scriptcore.gyp:*',
		]
	},		
	{
		'target_name' : 'uicore',
		'type' : 'none',
		'dependencies' : [
			'../src/shared/uicore/uicore.gyp:*',
		]
	},
	{
		'target_name' : 'usercore',
		'type' : 'none',
		'dependencies' : [
			'../src/shared/usercore/usercore.gyp:*',
		]
	},
	{
		'target_name' : 'webcore',
		'type' : 'none',
		'dependencies' : [
			'../src/shared/webcore/webcore.gyp:*',
		]
	},
	{
		'target_name' : 'crashuploader',
		'type' : 'none',
		'dependencies' : [
			'../src/shared/crashuploader/crashuploader.gyp:*',
		]
	},
	{
		'target_name' : 'bootloader_lin',
		'type' : 'none',
		'dependencies' : [
			'../src/executable/bootloader_lin/bootloader_lin.gyp:*',
		]
	},	
	{
		'target_name' : 'crashdlg_lin',
		'type' : 'none',
		'dependencies' : [
			'../src/executable/crashdlg_lin/crashdlg_lin.gyp:*',
		]
	},
	{
		'target_name' : 'scriptengine',
		'type' : 'none',
		'dependencies' : [
			'../src/static/scriptengine/scriptengine.gyp:*',
		]
	},
	{
		'target_name' : 'bittest_lin',
		'type' : 'none',
		'dependencies' : [
			'../src/executable/bittest_lin/bittest_lin.gyp:*',
		]
	},
	{
		'target_name' : 'process_data_folder',
		'type' : 'none',
		'dependencies' : [
			'../src/tools/process_data_folder/process_data_folder.gyp:*',
		]
	},	
	{
		'target_name' : 'all',
		'type' : 'none',
		'dependencies' : [
			'ipc_pipe',
			'managers',
			'mdump',
			'util_thread',
			'umcf',
			'util_fs',
			'util',
			'wx_controls',
			'mcfcore',
			'webcore',
			'usercore',
			'uicore',
			'servicecore',
			'scriptcore',
			'scriptengine',
			'crashdlg_lin',
			'process_data_folder',
		]
	},
	]
}
