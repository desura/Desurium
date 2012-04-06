{
	'includes': [
		 '../../../build_lin/common.gypi',
	],
	'targets': [
	{
		'target_name': 'mcf_util',
		'type': 'executable',
		'dependencies' : [
			'<(shared_dir)/mcfcore/mcfcore.gyp:mcfcore',
			'<(static_dir)/util/util.gyp:util',
			'<(static_dir)/umcf/umcf.gyp:umcf',
			'<(third_party_dir)/tinyxml/tinyxml.gyp:tinyxml',
			'<(third_party_dir)/libs.gyp:boost',
		],
		'ldflags': [
			'-ldl',
			'-lboost_system-desura',
			'-lboost_date_time-desura',
			'-lboost_filesystem-desura',
			'-lboost_thread-desura',
			'-lpthread',		
		],
		'include_dirs': [
			'./code',
		],
		'sources': [
			'./code/main.cpp',
			'./code/CheckMCF.cpp',
			'./code/CreateMCF.cpp',
			'./code/MCFSetup.cpp',
			'./code/PatchMCF.cpp',
			'./code/SaveMCF.cpp',
			'./code/TestMCF.cpp',
			'./code/UtilFunction.cpp',
		],
	}],
}
