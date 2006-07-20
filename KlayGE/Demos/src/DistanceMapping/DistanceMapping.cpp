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
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Sampler.hpp>
#include <KlayGE/SceneObjectHelper.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>
#include <boost/bind.hpp>

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
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			RenderEffectPtr effect = rf.LoadEffect("DistanceMapping.fx");

			if (!effect->ValidateTechnique("DistanceMapping30"))
			{
				if (!effect->ValidateTechnique("DistanceMapping2a"))
				{
					technique_ = effect->Technique("DistanceMapping20");
				}
				else
				{
					technique_ = effect->Technique("DistanceMapping2a");
				}
			}
			else
			{
				technique_ = effect->Technique("DistanceMapping30");
			}

			SamplerPtr diffuse_sampler(new Sampler);
			diffuse_sampler->SetTexture(LoadTexture("diffuse.dds"));
			diffuse_sampler->Filtering(Sampler::TFO_Bilinear);
			diffuse_sampler->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Wrap);
			diffuse_sampler->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Wrap);
			*(technique_->Effect().ParameterByName("diffuseMap")) = diffuse_sampler;

			SamplerPtr normal_sampler(new Sampler);
			normal_sampler->SetTexture(LoadTexture("normal.dds"));
			normal_sampler->Filtering(Sampler::TFO_Bilinear);
			normal_sampler->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Wrap);
			normal_sampler->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Wrap);
			*(technique_->Effect().ParameterByName("normalMap")) = normal_sampler;

			SamplerPtr distance_sampler(new Sampler);
			distance_sampler->SetTexture(LoadTexture("distance.dds"));
			distance_sampler->Filtering(Sampler::TFO_Bilinear);
			distance_sampler->AddressingMode(Sampler::TAT_Addr_U, Sampler::TAM_Wrap);
			distance_sampler->AddressingMode(Sampler::TAT_Addr_V, Sampler::TAM_Wrap);
			distance_sampler->AddressingMode(Sampler::TAT_Addr_W, Sampler::TAM_Clamp);
			*(technique_->Effect().ParameterByName("distanceMap")) = distance_sampler;

			float3 xyzs[] =
			{
				float3(-1, 1,  0),
				float3(1,	1,	0),
				float3(1,	-1,	0),
				float3(-1, -1, 0),
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

			float3 t[4], b[4];
			MathLib::compute_tangent<float>(t, b,
				indices, indices + sizeof(indices) / sizeof(indices[0]),
				xyzs, xyzs + sizeof(xyzs) / sizeof(xyzs[0]), texs);

			rl_ = rf.MakeRenderLayout(RenderLayout::BT_TriangleList);

			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static);
			pos_vb->Resize(sizeof(xyzs));
			{
				GraphicsBuffer::Mapper mapper(*pos_vb, BA_Write_Only);
				std::copy(&xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]), mapper.Pointer<float3>());
			}
			GraphicsBufferPtr tex0_vb = rf.MakeVertexBuffer(BU_Static);
			tex0_vb->Resize(sizeof(texs));
			{
				GraphicsBuffer::Mapper mapper(*tex0_vb, BA_Write_Only);
				std::copy(&texs[0], &texs[0] + sizeof(texs) / sizeof(texs[0]), mapper.Pointer<float2>());
			}
			GraphicsBufferPtr tan_vb = rf.MakeVertexBuffer(BU_Static);
			tan_vb->Resize(sizeof(t));
			{
				GraphicsBuffer::Mapper mapper(*tan_vb, BA_Write_Only);
				std::copy(&t[0], &t[0] + sizeof(t) / sizeof(t[0]), mapper.Pointer<float3>());
			}
			GraphicsBufferPtr binormal_vb = rf.MakeVertexBuffer(BU_Static);
			binormal_vb->Resize(sizeof(b));
			{
				GraphicsBuffer::Mapper mapper(*binormal_vb, BA_Write_Only);
				std::copy(&b[0], &b[0] + sizeof(b) / sizeof(b[0]), mapper.Pointer<float3>());
			}

			rl_->BindVertexStream(pos_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));
			rl_->BindVertexStream(tex0_vb, boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_GR32F)));
			rl_->BindVertexStream(tan_vb, boost::make_tuple(vertex_element(VEU_Tangent, 0, EF_BGR32F)));
			rl_->BindVertexStream(binormal_vb, boost::make_tuple(vertex_element(VEU_Binormal, 0, EF_BGR32F)));

			GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static);
			ib->Resize(sizeof(indices));
			{
				GraphicsBuffer::Mapper mapper(*ib, BA_Write_Only);
				std::copy(indices, indices + sizeof(indices) / sizeof(uint16_t), mapper.Pointer<uint16_t>());
			}
			rl_->BindIndexStream(ib, EF_R16);

			box_ = MathLib::compute_bounding_box<float>(&xyzs[0], &xyzs[4]);
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();

			float4x4 model = MathLib::rotation_x(-0.5f);
			float4x4 const & view = app.ActiveCamera().ViewMatrix();
			float4x4 const & proj = app.ActiveCamera().ProjMatrix();

			*(technique_->Effect().ParameterByName("worldviewproj")) = model * view * proj;
			*(technique_->Effect().ParameterByName("eyePos")) = app.ActiveCamera().EyePos();

			float degree(std::clock() / 700.0f);
			float3 lightPos(2, 0, -2);
			float4x4 matRot(MathLib::rotation_z(degree));
			lightPos = MathLib::transform_coord(lightPos, matRot);
			*(technique_->Effect().ParameterByName("lightPos")) = lightPos;
		}
	};

	class PolygonObject : public SceneObjectHelper
	{
	public:
		PolygonObject()
			: SceneObjectHelper(RenderablePtr(new RenderPolygon), SOA_Cullable)
		{
		}
	};


	enum
	{
		Exit,
	};

	InputActionDefine actions[] = 
	{
		InputActionDefine(Exit, KS_Escape),
	};

	bool ConfirmDevice(RenderDeviceCaps const & caps)
	{
		if (caps.max_shader_model < 2)
		{
			return false;
		}
		return true;
	}
}

int main()
{
	OCTree sceneMgr(Box(float3(-20, -20, -20), float3(20, 20, 20)), 3);

	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
	Context::Instance().SceneManagerInstance(sceneMgr);

	Context::Instance().InputFactoryInstance(DInputFactoryInstance());

	RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.color_fmt = EF_ARGB8;
	settings.full_screen = false;
	settings.ConfirmDevice = ConfirmDevice;

	DistanceMapping app;
	app.Create("DistanceMapping", settings);
	app.Run();

	return 0;
}

DistanceMapping::DistanceMapping()
{
	ResLoader::Instance().AddPath("../media/Common");
	ResLoader::Instance().AddPath("../media/DistanceMapping");
}

void DistanceMapping::InitObjects()
{
	// ½¨Á¢×ÖÌå
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.ttf", 16);

	polygon_.reset(new PolygonObject);
	polygon_->AddToSceneManager();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	this->LookAt(float3(2, 0, -2), float3(0, 0, 0));
	this->Proj(0.1f, 100);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.05f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler(inputEngine);
	input_handler += boost::bind(&DistanceMapping::InputHandler, this, _1, _2);
	inputEngine.ActionMap(actionMap, input_handler, true);
}

void DistanceMapping::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void DistanceMapping::DoUpdate(uint32_t /*pass*/)
{
	fpcController_.Update();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	renderEngine.Clear(RenderEngine::CBM_Color | RenderEngine::CBM_Depth);

	std::wostringstream stream;
	stream << renderEngine.CurRenderTarget()->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Distance Mapping");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str());
}
