#import "DesuraMain.h"

#import "Common.h"
#import "SharedObjectLoader.h"
#import "UICoreI.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_BUILDNO 0
#define VERSION_EXTEND 0

@interface DESMainAppImpl : DESMainApp {
	SharedObjectLoader g_UICoreDll;
	UICoreI* m_pUICore;
}

-(BOOL) loadUICore;

@end

static DESMainAppImpl *instance = nil;

@implementation DESMainApp

+(DESMainApp*) app
{
	if (instance == nil)
		instance = [[DESMainAppImpl alloc] init];
	return instance;
}

-(int) runWithArgc:(int)argc argv:(const char **)argv
{
	return [instance runWithArgc:argc argv:argv];
}

@end

@implementation DESMainAppImpl

-(int) runWithArgc:(int)argc argv:(const char **)argv
{
	if([self loadUICore])
	{
		return m_pUICore->initWxWidgets(argc, (char **)argv);
	} else {
		ERROR_OUTPUT(dlerror())
		return -1;
	}
}

// private methods

-(BOOL) loadUICore
{
	if(!g_UICoreDll.load("libuicore.dylib"))
		return NO;
	
	dlerror();
	
	UICoreFP UICoreInterface = g_UICoreDll.getFunction<UICoreFP>("GetInterface");
	
	if (!UICoreInterface || dlerror())
		return NO;
	
	char version[100] = {0};
	snprintf(version, 100, "%d.%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILDNO, VERSION_EXTEND);
	
	m_pUICore = UICoreInterface();
	
	if (!m_pUICore)
		return NO;
	
	m_pUICore->setDesuraVersion(version);
	//m_pUICore->setRestartFunction(&MainApp::restartFromUICore);
	//m_pUICore->setCrashDumpSettings(&MainApp::setCrashSettings);
	
	return YES;
}

@end
