#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <iterator>
#include <sstream>
#include <vector>

#include "SampleCommon.hpp"
#include "DistanceMapping.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderPolygon : public Renderable
	{
	public:
		RenderPolygon()
			: Renderable(L"Polygon")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = SyncLoadRenderEffect("DistanceMapping.fxml");

			technique_ = effect_->TechniqueByName("DistanceMapping2a");
			if (!technique_->Validate())
			{
				technique_ = effect_->TechniqueByName("DistanceMapping20");
			}

			*(effect_->ParameterByName("diffuse_tex")) = ASyncLoadTexture("diffuse.dds", EAH_GPU_Read | EAH_Immutable);
			*(effect_->ParameterByName("normal_tex")) = ASyncLoadTexture("normal.dds", EAH_GPU_Read | EAH_Immutable);
			*(effect_->ParameterByName("distance_tex")) = ASyncLoadTexture("distance.dds", EAH_GPU_Read | EAH_Immutable);

			float3 xyzs[] =
			{
				float3(-1, +1, 0),
				float3(+1, +1, 0),
				float3(+1, -1, 0),
				float3(-1, -1, 0)
			};

			float2 texs[] =
			{
				float2(0, 0),
				float2(1, 0),
				float2(1, 1),
				float2(0, 1)
			};

			uint16_t indices[] =
			{
				0, 1, 2, 2, 3, 0
			};

			rls_[0] = rf.MakeRenderLayout();
			rls_[0]->TopologyType(RenderLayout::TT_TriangleList);

			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(xyzs), xyzs);
			GraphicsBufferPtr tex0_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(texs), texs);

			rls_[0]->BindVertexStream(pos_vb, VertexElement(VEU_Position, 0, EF_BGR32F));
			rls_[0]->BindVertexStream(tex0_vb, VertexElement(VEU_TextureCoord, 0, EF_GR32F));

			float3 normal_float3[std::size(xyzs)];
			MathLib::compute_normal(normal_float3,
				indices, indices + std::size(indices),
				xyzs, xyzs + std::size(xyzs));

			float3 tangent_float3[std::size(xyzs)];
			float3 binormal_float3[std::size(xyzs)];
			MathLib::compute_tangent(tangent_float3, binormal_float3,
				indices, indices + std::size(indices),
				xyzs, xyzs + std::size(xyzs), texs, normal_float3);

			Quaternion tangent_quat[std::size(xyzs)];
			for (uint32_t j = 0; j < std::size(xyzs); ++ j)
			{
				float3 const t = MathLib::normalize(tangent_float3[j]);
				float3 const b = MathLib::normalize(binormal_float3[j]);
				float3 const n = MathLib::normalize(normal_float3[j]);

				tangent_quat[j] = MathLib::to_quaternion(t, b, n, 8);
			}

			uint32_t tangent[std::size(xyzs)];
			ElementFormat fmt;
			if (rf.RenderEngineInstance().DeviceCaps().VertexFormatSupport(EF_ABGR8))
			{
				fmt = EF_ABGR8;

				for (uint32_t j = 0; j < std::size(xyzs); ++ j)
				{
					tangent[j] = (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quat[j].x() * 0.5f + 0.5f) * 255), 0, 255) << 0)
						| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quat[j].y() * 0.5f + 0.5f) * 255), 0, 255) << 8)
						| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quat[j].z() * 0.5f + 0.5f) * 255), 0, 255) << 16)
						| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quat[j].w() * 0.5f + 0.5f) * 255), 0, 255) << 24);
				}
			}
			else
			{
				BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().VertexFormatSupport(EF_ARGB8));

				fmt = EF_ARGB8;

				for (uint32_t j = 0; j < std::size(xyzs); ++ j)
				{
					tangent[j] = (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quat[j].x() * 0.5f + 0.5f) * 255), 0, 255) << 16)
						| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quat[j].y() * 0.5f + 0.5f) * 255), 0, 255) << 8)
						| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quat[j].z() * 0.5f + 0.5f) * 255), 0, 255) << 0)
						| (MathLib::clamp<uint32_t>(static_cast<uint32_t>((tangent_quat[j].w() * 0.5f + 0.5f) * 255), 0, 255) << 24);
				}
			}
			
			GraphicsBufferPtr tan_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(tangent), tangent);
			rls_[0]->BindVertexStream(tan_vb, VertexElement(VEU_Tangent, 0, fmt));

			GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(indices), indices);
			rls_[0]->BindIndexStream(ib, EF_R16UI);

			pos_aabb_ = MathLib::compute_aabbox(&xyzs[0], &xyzs[0] + std::size(xyzs));
			tc_aabb_ = AABBox(float3(0, 0, 0), float3(0, 0, 0));
		}

		void LightPos(float3 const & light_pos)
		{
			*(effect_->ParameterByName("light_pos")) = MathLib::transform_coord(light_pos, inv_model_mat_);
		}

		void LightColor(float3 const & light_color)
		{
			*(effect_->ParameterByName("light_color")) = light_color;			
		}

		void LightFalloff(float3 const & light_falloff)
		{
			*(effect_->ParameterByName("light_falloff")) = light_falloff;			
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			*(effect_->ParameterByName("worldviewproj")) = model_mat_ * app.ActiveCamera().ViewProjMatrix();
			*(effect_->ParameterByName("eye_pos")) = MathLib::transform_coord(app.ActiveCamera().EyePos(), inv_model_mat_);
		}
	};


	class PointLightNodeUpdate
	{
	public:
		void operator()(SceneNode& node, float app_time, float elapsed_time)
		{
			KFL_UNUSED(elapsed_time);

			float4x4 const mat_rot = MathLib::rotation_z(app_time);
			node.TransformToParent(MathLib::translation(MathLib::transform_coord(float3(1, 0, -1), mat_rot)));
		}
	};


	enum
	{
		Exit,
		FullScreen,
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape),
		InputActionDefine(FullScreen, KS_Enter),
	};
}

int SampleMain()
{
	DistanceMapping app;
	app.Create();
	app.Run();

	return 0;
}

DistanceMapping::DistanceMapping()
					: App3DFramework("DistanceMapping")
{
	ResLoader::Instance().AddPath("../../Tutorials/media/DistanceMapping");
}

void DistanceMapping::OnCreate()
{
	font_ = SyncLoadFont("gkai00mp.kfont");

	polygon_renderable_ = MakeSharedPtr<RenderPolygon>();
	polygon_ = MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(polygon_renderable_), SceneNode::SOA_Cullable);
	polygon_->TransformToParent(MathLib::rotation_x(-0.5f));
	Context::Instance().SceneManagerInstance().SceneRootNode().AddChild(polygon_);

	this->LookAt(float3(2, 0, -2), float3(0, 0, 0));
	this->Proj(0.1f, 100);

	fpcController_.RequiresLeftButtonDown(true);
	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.1f);

	auto& root_node = Context::Instance().SceneManagerInstance().SceneRootNode();

	light_ = MakeSharedPtr<PointLightSource>();
	light_->Attrib(0);
	light_->Color(float3(2, 2, 2));
	light_->Falloff(float3(1, 0, 1.0f));

	auto light_proxy = LoadLightSourceProxyModel(light_);
	light_proxy->RootNode()->TransformToParent(MathLib::scaling(0.05f, 0.05f, 0.05f) * light_proxy->RootNode()->TransformToParent());

	auto light_node = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable | SceneNode::SOA_Moveable);
	light_node->TransformToParent(MathLib::translation(1.0f, 0.0f, -1.0f));
	light_node->AddComponent(light_);
	light_node->AddChild(light_proxy->RootNode());
	light_node->OnMainThreadUpdate().Connect(PointLightNodeUpdate());
	root_node.AddChild(light_node);

	checked_pointer_cast<RenderPolygon>(polygon_renderable_)->LightFalloff(light_->Falloff());

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + std::size(actions));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->Connect(
		[this](InputEngine const & sender, InputAction const & action)
		{
			this->InputHandler(sender, action);
		});
	inputEngine.ActionMap(actionMap, input_handler);

	UIManager::Instance().Load(*ResLoader::Instance().Open("DistanceMapping.uiml"));
}

void DistanceMapping::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls();
}

void DistanceMapping::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case FullScreen:
		{
			RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			renderEngine.EndFrame();
			ContextCfg const & cfg = Context::Instance().Config();
			renderEngine.Resize(cfg.graphics_cfg.width, cfg.graphics_cfg.height);
			renderEngine.FullScreen(!renderEngine.FullScreen());
			renderEngine.BeginFrame();
		}
		break;

	case Exit:
		this->Quit();
		break;
	}
}

void DistanceMapping::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Distance Mapping", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
	font_->RenderText(0, 36, Color(1, 1, 0, 1), renderEngine.Name(), 16);
}

uint32_t DistanceMapping::DoUpdate(uint32_t /*pass*/)
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	Color clear_clr(0.2f, 0.4f, 0.6f, 1);
	if (Context::Instance().Config().graphics_cfg.gamma)
	{
		clear_clr.r() = 0.029f;
		clear_clr.g() = 0.133f;
		clear_clr.b() = 0.325f;
	}
	renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

	checked_pointer_cast<RenderPolygon>(polygon_renderable_)->LightPos(light_->Position());
	checked_pointer_cast<RenderPolygon>(polygon_renderable_)->LightColor(light_->Color());
	checked_pointer_cast<RenderPolygon>(polygon_renderable_)->LightFalloff(light_->Falloff());

	return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
}
