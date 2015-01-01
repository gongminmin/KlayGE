#include <KlayGE/KlayGE.hpp>

#ifdef KLAYGE_PLATFORM_IOS

#include <KFL/PreDeclare.hpp>
#include <KFL/Math.hpp>
#include <KFL/Util.hpp>

#include <KlayGE/Window.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <UIKit/UIKit.h>
#include <GLKit/GLKit.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

@interface KlayGEView : GLKView<GLKViewDelegate>
{
	CADisplayLink *displayLink;
}
- (void) startDisplayLink;
- (void) stopDisplayLink;
- (void) render;
@end

namespace KlayGE
{
	Window::Window(std::string const & name, RenderSettings const & settings)
		: active_(false), ready_(false), closed_(false)
	{
		//http://www.raywenderlich.com/5223/beginning-opengl-es-2-0-with-glkit-part-1
		glk_view_ = [[KlayGEView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
		glk_view_.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
		glk_view_.delegate = glk_view_;
		glk_view_.enableSetNeedsDisplay = NO;

		UIWindow *window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
		[window addSubview:glk_view_];
		window.backgroundColor = [UIColor whiteColor];
		[window makeKeyAndVisible];

		CGRect rect = glk_view_.frame;
		left_ = 0;
		top_ = 0;
		width_ = rect.size.width;
		height_ = rect.size.height;
	}

	Window::Window(std::string const & name, RenderSettings const & settings, void* native_wnd)
		: active_(false), ready_(false), closed_(false)
	{
		LogWarn("Unimplemented Window::Window");
	}

	Window::~Window()
	{
		LogWarn("Unimplemented Window::~Window");
	}

	void Window::StartRunLoop()
	{
		[glk_view_ startDisplayLink];
	}

	void Window::StopRunLoop()
	{
	}

	void Window::FlushBuffer()
	{
	}

	uint2 Window::GetGLKViewSize()
	{
		CGRect rect = glk_view_.frame;
		return KlayGE::uint2(rect.size.width, rect.size.height);
	}
}

@implementation KlayGEView

- (void) startDisplayLink
{
	displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(render:)];
	[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}

- (void) stopDisplayLink
{
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
	KlayGE::Context::Instance().RenderFactoryInstance().RenderEngineInstance().Refresh();
}

- (void)render:(CADisplayLink*)displayLink
{
	[self display];
}
@end

#endif
