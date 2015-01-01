#include <KlayGE/KlayGE.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Window.hpp>

#include "SampleCommon.hpp"

#ifdef KLAYGE_PLATFORM_IOS
#import <UIKit/UIKit.h>

@interface KlayGEAppDelegate : NSObject <UIApplicationDelegate>
{
	KlayGE::App3DFramework* app;
}
@end

int main(int argc, char *argv[])
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	int retVal = UIApplicationMain(argc, argv, nil, @"KlayGEAppDelegate");
	[pool release];
	return retVal;
}

@implementation KlayGEAppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	KlayGE::ResLoader::Instance().AddPath("../../Samples/media/Common");
	KlayGE::Context::Instance().LoadCfg("KlayGE.cfg");
	
	app = SampleApp();
	app->Create();
	// Empty app->Run()
	KlayGE::Context::Instance().AppInstance().MainWnd()->StartRunLoop();
	
	return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
	// TODO: active
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
	// TODO: active
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	// TODO: close
}

@end

#elif defined(KLAYGE_PLATFORM_DARWIN)
#import <Cocoa/Cocoa.h>

@interface KlayGEAppDelegate : NSObject<NSApplicationDelegate>
{
	KlayGE::App3DFramework *app;
}
@end

int main()
{
	NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
	
	NSApplication* application = [NSApplication sharedApplication];
	[application setActivationPolicy:NSApplicationActivationPolicyRegular];
	KlayGEAppDelegate *delegate = [[[KlayGEAppDelegate alloc] init] autorelease];
	
	[application setDelegate:delegate];
	[application run];
	
	[pool drain];
	
	return EXIT_SUCCESS;
}

@implementation KlayGEAppDelegate : NSObject
- (id)init {
	if (self = [super init]) {
	}
	return self;
}

- (void)applicationWillFinishLaunching:(NSNotification*)notification
{
	(void)notification;
	KlayGE::ResLoader::Instance().AddPath("../../Samples/media/Common");
	KlayGE::Context::Instance().LoadCfg("KlayGE.cfg");
	app = SampleApp();
	app->Create();
	// Empty app->Run()
	KlayGE::Context::Instance().AppInstance().MainWnd()->StartRunLoop();
}

- (void)applicationWillTerminate:(NSNotification*)notification
{
	delete app;    
}

- (BOOL) applicationShouldTerminateAfterLastWindowClosed: (NSApplication*)application
{
	(void)application;
	return YES;
}

- (void)dealloc
{
	[super dealloc];
}
@end
#endif