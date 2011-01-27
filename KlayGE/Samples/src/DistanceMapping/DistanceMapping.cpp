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
#include <KlayGE/Timer.hpp>
#include <KlayGE/Light.hpp>

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
			BOOST_AUTO(diffuse_loader, LoadTexture("diffuse.dds", EAH_GPU_Read));
			BOOST_AUTO(normal_loader, LoadTexture("normal.dds", EAH_GPU_Read));
			BOOST_AUTO(dist_loader, LoadTexture("distance.dds", EAH_GPU_Read));

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
			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);

			init_data.row_pitch = sizeof(texs);
			init_data.slice_pitch = 0;
			init_data.data = texs;
			GraphicsBufferPtr tex0_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);

			rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));
			rl_->BindVertexStream(tex0_vb, boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_GR32F)));

			float3 normal_float3[4];
			MathLib::compute_normal<float>(normal_float3,
				indices, indices + sizeof(indices) / sizeof(indices[0]),
				xyzs, xyzs + sizeof(xyzs) / sizeof(xyzs[0]));

			float3 tangent_float3[4], binormal_float3[4];
			MathLib::compute_tangent<float>(tangent_float3, binormal_float3,
				indices, indices + sizeof(indices) / sizeof(indices[0]),
				xyzs, xyzs + sizeof(xyzs) / sizeof(xyzs[0]), texs, normal_float3);

			for (uint32_t j = 0; j < 4; ++ j)
			{
				normal_float3[j] = MathLib::normalize(normal_float3[j]) * 0.5f + 0.5f;
				tangent_float3[j] = MathLib::normalize(tangent_float3[j]) * 0.5f + 0.5f;
			}

			if (rf.RenderEngineInstance().DeviceCaps().vertex_format_support(EF_A2BGR10))
			{
				uint32_t normal[4];
				for (uint32_t j = 0; j < 4; ++ j)
				{
					normal[j] = MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal_float3[j].x() * 1023), 0, 1023)
						| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal_float3[j].y() * 1023), 0, 1023) << 10)
						| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(normal_float3[j].z() * 1023), 0, 1023) << 20);
				}
				uint32_t tangent[4];
				for (uint32_t j = 0; j < 4; ++ j)
				{
					tangent[j] = MathLib::clamp<uint32_t>(static_cast<uint32_t>(tangent_float3[j].x() * 1023), 0, 1023)
						| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(tangent_float3[j].y() * 1023), 0, 1023) << 10)
						| (MathLib::clamp<uint32_t>(static_cast<uint32_t>(tangent_float3[j].z() * 1023), 0, 1023) << 20);
				}

				init_data.row_pitch = sizeof(normal);
				init_data.slice_pitch = 0;
				init_data.data = normal;
				GraphicsBufferPtr normal_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
			
				init_data.row_pitch = sizeof(tangent);
				init_data.slice_pitch = 0;
				init_data.data = tangent;
				GraphicsBufferPtr tan_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);

				rl_->BindVertexStream(normal_vb, boost::make_tuple(vertex_element(VEU_Normal, 0, EF_A2BGR10)));
				rl_->BindVertexStream(tan_vb, boost::make_tuple(vertex_element(VEU_Tangent, 0, EF_A2BGR10)));
			}
			else
			{
				init_data.row_pitch = sizeof(normal_float3);
				init_data.slice_pitch = 0;
				init_data.data = normal_float3;
				GraphicsBufferPtr normal_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
			
				init_data.row_pitch = sizeof(tangent_float3);
				init_data.slice_pitch = 0;
				init_data.data = tangent_float3;
				GraphicsBufferPtr tan_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);

				rl_->BindVertexStream(normal_vb, boost::make_tuple(vertex_element(VEU_Normal, 0, EF_BGR32F)));
				rl_->BindVertexStream(tan_vb, boost::make_tuple(vertex_element(VEU_Tangent, 0, EF_BGR32F)));
			}

			init_data.row_pitch = sizeof(indices);
			init_data.slice_pitch = 0;
			init_data.data = indices;
			GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read, &init_data);
			rl_->BindIndexStream(ib, EF_R16UI);

			box_ = MathLib::compute_bounding_box<float>(&xyzs[0], &xyzs[4]);
		}

		void LightPos(float3 const & light_pos)
		{
			*(technique_->Effect().ParameterByName("light_pos")) = light_pos;			
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

			float4x4 model = MathLib::rotation_x(-0.5f);
			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("worldviewproj")) = model * view * proj;
			*(technique_->Effect().ParameterByName("eye_pos")) = app.ActiveCamera().EyePos();
		}
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

	private:
		Timer timer_;
	};


	class PointLightSourceUpdate
	{
	public:
		void operator()(LightSource& light)
		{
			float degree = static_cast<float>(timer_.current_time());
			float4x4 matRot = MathLib::rotation_z(degree);

			float3 light_pos(1, 0, -1);
			light.Position(MathLib::transform_coord(light_pos, matRot));
		}

	private:
		Timer timer_;
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
	ResLoader::Instance().AddPath("../Samples/media/Common");

	Context::Instance().LoadCfg("KlayGE.cfg");

	DistanceMapping app;
	app.Create();
	app.Run();

	return 0;
}

DistanceMapping::DistanceMapping()
					: App3DFramework("DistanceMapping")
{
	ResLoader::Instance().AddPath("../Samples/media/DistanceMapping");
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
	polygon_->AddToSceneManager();

	this->LookAt(float3(2, 0, -2), float3(0, 0, 0));
	this->Proj(0.1f, 100);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.1f);

	light_ = MakeSharedPtr<PointLightSource>();
	light_->Attrib(0);
	light_->Color(float3(2, 2, 2));
	light_->Falloff(float3(0, 0, 1.0f));
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

	UIManager::Instance().Load(ResLoader::Instance().Load("DistanceMapping.uiml"));
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
	renderEngine.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, Color(0.2f, 0.4f, 0.6f, 1), 1.0f, 0);

	checked_pointer_cast<PolygonObject>(polygon_)->LightPos(light_->Position());
	checked_pointer_cast<PolygonObject>(polygon_)->LightColor(light_->Color());
	checked_pointer_cast<PolygonObject>(polygon_)->LightFalloff(light_->Falloff());

	return App3DFramework::URV_Need_Flush | App3DFramework::URV_Finished;
}
