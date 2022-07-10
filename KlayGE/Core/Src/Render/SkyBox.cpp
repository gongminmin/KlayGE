/**
 * @file SkyBox.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <KlayGE/KlayGE.hpp>
#include <KFL/Math.hpp>
#include <KFL/Util.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/SkyBox.hpp>

namespace KlayGE
{
	RenderableSkyBox::RenderableSkyBox()
		: Renderable(L"SkyBox")
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		RenderEffectPtr effect = SyncLoadRenderEffect("SkyBox.fxml");
		if (Context::Instance().DeferredRenderingLayerInstance())
		{
			effect_attrs_ |= EA_SpecialShading;

			this->BindDeferredEffect(effect);
			gbuffer_tech_ = effect->TechniqueByName("GBufferSkyBoxTech");
			special_shading_tech_ = effect->TechniqueByName("SkyBoxTech");
			this->Technique(effect, gbuffer_tech_);
		}
		else
		{
			this->Technique(effect, effect->TechniqueByName("SkyBoxTech"));
		}

		float3 xyzs[] =
		{
			float3(1.0f, 1.0f, 1.0f),
			float3(1.0f, -1.0f, 1.0f),
			float3(-1.0f, 1.0f, 1.0f),
			float3(-1.0f, -1.0f, 1.0f),
		};

		rls_[0] = rf.MakeRenderLayout();
		rls_[0]->TopologyType(RenderLayout::TT_TriangleStrip);

		GraphicsBufferPtr vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(xyzs), xyzs);
		rls_[0]->BindVertexStream(vb, VertexElement(VEU_Position, 0, EF_BGR32F));

		pos_aabb_ = MathLib::compute_aabbox(&xyzs[0], &xyzs[4]);
		tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));
	}

	void RenderableSkyBox::Technique(RenderEffectPtr const & effect, RenderTechnique* tech)
	{
		effect_ = effect;
		technique_ = tech;

		skybox_cube_tex_ep_ = effect_->ParameterByName("skybox_tex");
		skybox_Ccube_tex_ep_ = effect_->ParameterByName("skybox_C_tex");
		skybox_compressed_ep_ = effect_->ParameterByName("skybox_compressed");
		depth_far_ep_ = effect_->ParameterByName("depth_far");
		inv_mvp_ep_ = effect_->ParameterByName("inv_mvp");
	}

	void RenderableSkyBox::CubeMap(TexturePtr const & cube)
	{
		*skybox_cube_tex_ep_ = cube;
		*skybox_compressed_ep_ = static_cast<int32_t>(0);
	}

	void RenderableSkyBox::CompressedCubeMap(TexturePtr const & y_cube, TexturePtr const & c_cube)
	{
		*skybox_cube_tex_ep_ = y_cube;
		*skybox_Ccube_tex_ep_ = c_cube;
		*skybox_compressed_ep_ = static_cast<int32_t>(1);
	}

	void RenderableSkyBox::Pass(PassType type)
	{
		switch (type)
		{
		case PT_OpaqueGBuffer:
			technique_ = gbuffer_tech_;
			break;

		case PT_OpaqueSpecialShading:
			technique_ = special_shading_tech_;
			break;

		default:
			break;
		}
	}

	void RenderableSkyBox::OnRenderBegin()
	{
		App3DFramework const & app = Context::Instance().AppInstance();
		Camera const & camera = app.ActiveCamera();

		*depth_far_ep_ = camera.FarPlane();

		float4x4 rot_view = camera.ViewMatrix();
		rot_view(3, 0) = 0;
		rot_view(3, 1) = 0;
		rot_view(3, 2) = 0;
		*inv_mvp_ep_ = MathLib::inverse(rot_view * camera.ProjMatrix());
	}
}
