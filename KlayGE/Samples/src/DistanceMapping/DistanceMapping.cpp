#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
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
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/Camera.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>
#include <boost/bind.hpp>
#include <boost/typeof/typeof.hpp>

#include "DistanceMapping.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderPolygon : public RenderableHelper
	{
	public:
		RenderPolygon()
			: RenderableHelper(L"Polygon")
		{
			BOOST_AUTO(diffuse_loader, ASyncLoadTexture("diffuse.dds", EAH_GPU_Read | EAH_Immutable));
			BOOST_AUTO(normal_loader, ASyncLoadTexture("normal.dds", EAH_GPU_Read | EAH_Immutable));
			BOOST_AUTO(dist_loader, ASyncLoadTexture("distance.dds", EAH_GPU_Read | EAH_Immutable));

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			RenderEffectPtr effect = rf.LoadEffect("DistanceMapping.fxml");

			technique_ = effect->TechniqueByName("DistanceMapping2a");
			if (!technique_->Validate())
			{
				technique_ = effect->TechniqueByName("DistanceMapping20");
			}

			*(technique_->Effect().ParameterByName("diffuse_tex")) = diffuse_loader();
			*(technique_->Effect().ParameterByName("normal_tex")) = normal_loader();
			*(technique_->Effect().ParameterByName("distance_tex")) = dist_loader();

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

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_TriangleList);

			ElementInitData init_data;
			init_data.row_pitch = sizeof(xyzs);
			init_data.slice_pitch = 0;
			init_data.data = xyzs;
			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);

			init_data.row_pitch = sizeof(texs);
			init_data.slice_pitch = 0;
			init_data.data = texs;
			GraphicsBufferPtr tex0_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);

			rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));
			rl_->BindVertexStream(tex0_vb, boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_GR32F)));

			float3 normal_float3[sizeof(xyzs) / sizeof(xyzs[0])];
			MathLib::compute_normal(normal_float3,
				indices, indices + sizeof(indices) / sizeof(indices[0]),
				xyzs, xyzs + sizeof(xyzs) / sizeof(xyzs[0]));

			float4 tangent_float4[sizeof(xyzs) / sizeof(xyzs[0])];
			float3 binormal_float3[sizeof(xyzs) / sizeof(xyzs[0])];
			MathLib::compute_tangent(tangent_float4, binormal_float3,
				indices, indices + sizeof(indices) / sizeof(indices[0]),
				xyzs, xyzs + sizeof(xyzs) / sizeof(xyzs[0]), texs, normal_float3);

			for (uint32_t j = 0; j < sizeof(xyzs) / sizeof(xyzs[0]); ++ j)
			{
				normal_float3[j] = MathLib::normalize(normal_float3[j]) * 0.5f + 0.5f;

				float3 t(tangent_float4[j].x(), tangent_float4[j].y(), tangent_float4[j].z());
				t = MathLib::normalize(t) * 0.5f + 0.5f;
				tangent_float4[j] = float4(t.x(), t.y(), t.z(), tangent_float4[j].w() * 0.5f + 0.5f);
			}

			uint32_t normal[sizeof(xyzs) / sizeof(xyzs[0])];
			uint32_t tangent[sizeof(xyzs) / sizeof(xyzs[0])];
			ElementFormat fmt;
			if (rf.RenderEngineInstance().DeviceCaps().vertex_format_support(EF_A2BGR10))
			{
				fmt = EF_A2BGR10;

				for (uint32_t j = 0; j < sizeof(xyzs) / sizeof(xyzs[0]); ++ j)
				{
					normal[j] = MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal_float3[j].x() * 1023), 0, 1023)
						| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal_float3[j].y() * 1023), 0, 1023) << 10)
						| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal_float3[j].z() * 1023), 0, 1023) << 20);
				}
				for (uint32_t j = 0; j < sizeof(xyzs) / sizeof(xyzs[0]); ++ j)
				{
					tangent[j] = MathLib::clamp<uint32_t>(static_cast<uint32_t>(tangent_float4[j].x() * 1023), 0, 1023)
						| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(tangent_float4[j].y() * 1023), 0, 1023) << 10)
						| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(tangent_float4[j].z() * 1023), 0, 1023) << 20)
						| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(tangent_float4[j].w() * 3), 0, 3) << 30);
				}
			}
			else
			{
				if (rf.RenderEngineInstance().DeviceCaps().vertex_format_support(EF_ARGB8))
				{
					fmt = EF_ARGB8;

					for (uint32_t j = 0; j < sizeof(xyzs) / sizeof(xyzs[0]); ++ j)
					{
						normal[j] = (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal_float3[j].x() * 255), 0, 255) << 16)
							| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal_float3[j].y() * 255), 0, 255) << 8)
							| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal_float3[j].z() * 255), 0, 255) << 0);
					}
					for (uint32_t j = 0; j < sizeof(xyzs) / sizeof(xyzs[0]); ++ j)
					{
						tangent[j] = (MathLib::clamp<uint32_t>(static_cast<uint32_t>(tangent_float4[j].x() * 255), 0, 255) << 16)
							| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(tangent_float4[j].y() * 255), 0, 255) << 8)
							| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(tangent_float4[j].z() * 255), 0, 255) << 0)
							| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(tangent_float4[j].w() * 255), 0, 255) << 24);
					}
				}
				else
				{
					BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().vertex_format_support(EF_ABGR8));

					fmt = EF_ABGR8;

					for (uint32_t j = 0; j < sizeof(xyzs) / sizeof(xyzs[0]); ++ j)
					{
						normal[j] = (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal_float3[j].x() * 255), 0, 255) << 0)
							| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal_float3[j].y() * 255), 0, 255) << 8)
							| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal_float3[j].z() * 255), 0, 255) << 16);
					}
					for (uint32_t j = 0; j < sizeof(xyzs) / sizeof(xyzs[0]); ++ j)
					{
						tangent[j] = (MathLib::clamp<uint32_t>(static_cast<uint32_t>(tangent_float4[j].x() * 255), 0, 255) << 0)
							| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(tangent_float4[j].y() * 255), 0, 255) << 8)
							| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(tangent_float4[j].z() * 255), 0, 255) << 16)
							| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(tangent_float4[j].w() * 255), 0, 255) << 24);
					}
				}
			}
			
			init_data.row_pitch = sizeof(normal);
			init_data.slice_pitch = 0;
			init_data.data = normal;
			GraphicsBufferPtr normal_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
			rl_->BindVertexStream(normal_vb, boost::make_tuple(vertex_element(VEU_Normal, 0, fmt)));
			
			init_data.row_pitch = sizeof(tangent);
			init_data.slice_pitch = 0;
			init_data.data = tangent;
			GraphicsBufferPtr tan_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
			rl_->BindVertexStream(tan_vb, boost::make_tuple(vertex_element(VEU_Tangent, 0, fmt)));

			init_data.row_pitch = sizeof(indices);
			init_data.slice_pitch = 0;
			init_data.data = indices;
			GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
			rl_->BindIndexStream(ib, EF_R16UI);

			aabb_ = MathLib::compute_bounding_box<float>(&xyzs[0], &xyzs[sizeof(xyzs) / sizeof(xyzs[0])]);
		}

		void ModelMatrix(float4x4 const & mat)
		{
			RenderableHelper::ModelMatrix(mat);
			inv_model_mat_ = MathLib::inverse(model_mat_);
		}

		void LightPos(float3 const & light_pos)
		{
			*(technique_->Effect().ParameterByName("light_pos")) = MathLib::transform_coord(light_pos, inv_model_mat_);
		}

		void LightColor(float3 const & light_color)
		{
			*(technique_->Effect().ParameterByName("light_color")) = light_color;			
		}

		void LightFalloff(float3 const & light_falloff)
		{
			*(technique_->Effect().ParameterByName("light_falloff")) = light_falloff;			
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("worldviewproj")) = model_mat_ * view * proj;
			*(technique_->Effect().ParameterByName("eye_pos")) = MathLib::transform_coord(app.ActiveCamera().EyePos(), inv_model_mat_);
		}

	private:
		float4x4 inv_model_mat_;
	};

	class PolygonObject : public SceneObjectHelper
	{
	public:
		PolygonObject()
			: SceneObjectHelper(MakeSharedPtr<RenderPolygon>(), SOA_Cullable)
		{
		}

		void LightPos(float3 const & light_pos)
		{
			checked_pointer_cast<RenderPolygon>(renderable_)->LightPos(light_pos);
		}

		void LightColor(float3 const & light_color)
		{
			checked_pointer_cast<RenderPolygon>(renderable_)->LightColor(light_color);
		}

		void LightFalloff(float3 const & light_falloff)
		{
			checked_pointer_cast<RenderPolygon>(renderable_)->LightFalloff(light_falloff);
		}
	};


	class PointLightSourceUpdate
	{
	public:
		void operator()(LightSource& light, float app_time, float /*elapsed_time*/)
		{
			float4x4 matRot = MathLib::rotation_z(app_time);

			float3 light_pos(1, 0, -1);
			light.Position(MathLib::transform_coord(light_pos, matRot));
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

int main()
{
	ResLoader::Instance().AddPath("../../Samples/media/Common");

	Context::Instance().LoadCfg("KlayGE.cfg");

	DistanceMapping app;
	app.Create();
	app.Run();

	return 0;
}

DistanceMapping::DistanceMapping()
					: App3DFramework("DistanceMapping")
{
	ResLoader::Instance().AddPath("../../Samples/media/DistanceMapping");
}

bool DistanceMapping::ConfirmDevice() const
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}
	return true;
}

void DistanceMapping::InitObjects()
{
	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.kfont");

	polygon_ = MakeSharedPtr<PolygonObject>();
	polygon_->ModelMatrix(MathLib::rotation_x(-0.5f));
	polygon_->AddToSceneManager();

	this->LookAt(float3(2, 0, -2), float3(0, 0, 0));
	this->Proj(0.1f, 100);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.1f);

	light_ = MakeSharedPtr<PointLightSource>();
	light_->Attrib(0);
	light_->Color(float3(2, 2, 2));
	light_->Falloff(float3(1, 0, 1.0f));
	light_->Position(float3(1, 0, -1));
	light_->BindUpdateFunc(PointLightSourceUpdate());
	light_->AddToSceneManager();

	light_proxy_ = MakeSharedPtr<SceneObjectLightSourceProxy>(light_);
	checked_pointer_cast<SceneObjectLightSourceProxy>(light_proxy_)->Scaling(0.05f, 0.05f, 0.05f);
	light_proxy_->AddToSceneManager();

	checked_pointer_cast<PolygonObject>(polygon_)->LightFalloff(light_->Falloff());

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&DistanceMapping::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	UIManager::Instance().Load(ResLoader::Instance().Open("DistanceMapping.uiml"));
}

void DistanceMapping::OnResize(uint32_t width, uint32_t height)
{
	UIManager::Instance().SettleCtrls(width, height);
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

	checked_pointer_cast<PolygonObject>(polygon_)->LightPos(light_->Position());
	checked_pointer_cast<PolygonObject>(polygon_)->LightColor(light_->Color());
	checked_pointer_cast<PolygonObject>(polygon_)->LightFalloff(light_->Falloff());

	return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
}
