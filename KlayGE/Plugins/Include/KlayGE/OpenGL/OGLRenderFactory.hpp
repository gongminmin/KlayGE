#ifndef _OGLRENDERFACTORY_HPP
#define _OGLRENDERFACTORY_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderFactory.hpp>

#define NOMINMAX
#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>

#pragma comment(lib, "KlayGE_RenderEngine_OpenGL.lib")

namespace KlayGE
{
	RenderFactory& OGLRenderFactoryInstance();
}

#endif			// _OGLRENDERFACTORY_HPP
