#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderFactory.hpp>

#include "CartoonPP.hpp"

using namespace KlayGE;

CartoonPostProcess::CartoonPostProcess()
	: PostProcess(Context::Instance().RenderFactoryInstance().LoadEffect("CartoonPP.kfx")->TechniqueByName("Cartoon"))
{
}

void CartoonPostProcess::Source(TexturePtr const & tex, bool flipping)
{
	PostProcess::Source(tex, flipping);
	if (tex)
	{
		*(technique_->Effect().ParameterByName("inv_width_height")) = float2(1.0f / tex->Width(0), 1.0f / tex->Height(0));
	}
}

void CartoonPostProcess::ColorTex(TexturePtr const & tex)
{
	*(technique_->Effect().ParameterByName("color_tex")) = tex;
}

void CartoonPostProcess::OnRenderBegin()
{
	PostProcess::OnRenderBegin();

	Camera const & camera = Context::Instance().AppInstance().ActiveCamera();

	float depth_range = camera.FarPlane() - camera.NearPlane();
	*(technique_->Effect().ParameterByName("e_barrier")) = float2(0.8f, 0.1f / depth_range);
}
