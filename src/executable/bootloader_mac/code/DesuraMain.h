
#import <Foundation/NSObject.h>

@interface DESMainApp : NSObject {
	
}

+(DESMainApp*) app;
-(int) runWithArgc:(int)argc argv:(const char **)argv;

@end
