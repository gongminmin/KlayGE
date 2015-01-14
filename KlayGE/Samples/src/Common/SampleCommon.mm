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

- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
	UNREF_PARAM(application);
	UNREF_PARAM(launchOptions);

	NSString* path = [[NSBundle mainBundle] resourcePath];
	KlayGE::ResLoader::Instance().AddPath(std::string([path UTF8String]));

	[self performSelector:@selector(postFinishLaunch) withObject:nil afterDelay:0.0];
	return YES;
}

- (void)applicationWillResignActive:(UIApplication*)application
{
	UNREF_PARAM(application);
	KlayGE::WindowPtr const & app_window = KlayGE::Context::Instance().AppInstance().MainWnd();
	app_window->Active(false);
	app_window->OnActive()(*app_window, false);
}

- (void)applicationDidBecomeActive:(UIApplication*)application
{
	UNREF_PARAM(application);

	if (!KlayGE::Context::Instance().AppValid())
	{
		[self performSelector:@selector(applicationDidBecomeActive:) withObject:nil afterDelay:1.0];
		return;
	}

	KlayGE::WindowPtr const & app_window = KlayGE::Context::Instance().AppInstance().MainWnd();
	app_window->BindDrawable(); // restore GL context after awake from background
	app_window->Active(true);
	app_window->Ready(true);
	app_window->OnSize()(*app_window, true);
}

- (void)applicationWillTerminate:(UIApplication*)application
{
	UNREF_PARAM(application);

	KlayGE::WindowPtr const & app_window = KlayGE::Context::Instance().AppInstance().MainWnd();
	app_window->OnClose()(*app_window);
	app_window->Active(false);
	app_window->Ready(false);
	app_window->Closed(true);
}

- (void)postFinishLaunch
{
	EntryFunc();
}
@end

#endif
