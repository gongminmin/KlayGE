#include <KlayGE/KlayGE.hpp>
#include <KFL/ThrowErr.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/TexCompressionBC.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>

#include "SampleCommon.hpp"
#include "EnvLighting.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class SphereRenderable : public StaticMesh
	{
	public:
		SphereRenderable(RenderModelPtr const & model, std::wstring const & /*name*/)
			: StaticMesh(model, L"Sphere")
		{
			RenderEffectPtr effect = SyncLoadRenderEffect("EnvLighting.fxml");
			techs_[0] = effect->TechniqueByName("PBFittingPrefiltered"); 
			techs_[1] = effect->TechniqueByName("PBPrefiltered");
			techs_[2] = effect->TechniqueByName("Prefiltered");
			techs_[3] = effect->TechniqueByName("Approximate");
			techs_[4] = effect->TechniqueByName("GroundTruth");
			this->RenderingType(0);

			SceneManager& sm = Context::Instance().SceneManagerInstance();
			for (uint32_t i = 0; i < sm.NumLights(); ++ i)
			{
				LightSourcePtr const & light = sm.GetLight(i);
				if (LightSource::LT_Ambient == light->Type())
				{
					*(technique_->Effect().ParameterByName("skybox_Ycube_tex")) = light->SkylightTexY();
					*(technique_->Effect().ParameterByName("skybox_Ccube_tex")) = light->SkylightTexC();

					uint32_t const mip = light->SkylightTexY()->NumMipMaps();
					*(technique_->Effect().ParameterByName("diff_spec_mip")) = int2(mip - 1, mip - 2);
					*(technique_->Effect().ParameterByName("mip_bias")) = mip / -2.0f;
					break;
				}
			}
		}

		virtual void DoBuildMeshInfo() override
		{
			AABBox const & pos_bb = this->PosBound();
			*(technique_->Effect().ParameterByName("pos_center")) = pos_bb.Center();
			*(technique_->Effect().ParameterByName("pos_extent")) = pos_bb.HalfSize();

			AABBox const & tc_bb = this->TexcoordBound();
			*(technique_->Effect().ParameterByName("tc_center")) = float2(tc_bb.Center().x(), tc_bb.Center().y());
			*(technique_->Effect().ParameterByName("tc_extent")) = float2(tc_bb.HalfSize().x(), tc_bb.HalfSize().y());
		}

		void Material(float4 const &  diff_spec, float roughness)
		{
			*(technique_->Effect().ParameterByName("diff_spec")) = diff_spec;
			*(technique_->Effect().ParameterByName("roughness")) = roughness;
		}

		void RenderingType(int type)
		{
			technique_ = techs_[type];
		}

		void IntegrateBRDFTex(TexturePtr const & tex)
		{
			*(technique_->Effect().ParameterByName("integrated_brdf_tex")) = tex;
		}

		void OnRenderBegin()
		{
			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			float4x4 const mvp = model_mat_ * camera.ViewProjMatrix();

			*(technique_->Effect().ParameterByName("model")) = model_mat_;
			*(technique_->Effect().ParameterByName("mvp")) = mvp;
			*(technique_->Effect().ParameterByName("eye_pos")) = camera.EyePos();
		}

	private:
		array<RenderTechniquePtr, 5> techs_;
	};

	class SphereObject : public SceneObjectHelper
	{
	public:
		SphereObject(float4 const & diff_spec, float roughness)
			: SceneObjectHelper(SOA_Cullable)
		{
			renderable_ = SyncLoadModel("sphere_high.7z//sphere_high.meshml", EAH_GPU_Read | EAH_Immutable, CreateModelFactory<RenderModel>(), CreateMeshFactory<SphereRenderable>())->Subrenderable(0);
			checked_pointer_cast<SphereRenderable>(renderable_)->Material(diff_spec, roughness);
		}

		void RenderingType(int type)
		{
			checked_pointer_cast<SphereRenderable>(renderable_)->RenderingType(type);
		}

		void IntegrateBRDFTex(TexturePtr const & tex)
		{
			checked_pointer_cast<SphereRenderable>(renderable_)->IntegrateBRDFTex(tex);
		}
	};

	float4 diff_spec_parametes[] =
	{
		float4(0.0147f,  0.0332f,  0.064f,   0.00121383f),
		float4(0.0183f,  0.0657f,  0.0248f,  0.001264066f),
		float4(0.0712f,  0.0263f,  0.03f,    0.000666173f),
		float4(0.314f,   0.259f,   0.156f,   0.000782239f),
		float4(0.226f,   0.233f,   0.0489f,  0.00103061f),
		float4(0.0864f,  0.0597f,  0.0302f,  0.009314418f),
		float4(0.31f,    0.248f,   0.151f,   0.001512634f),
		float4(0.0335f,  0.028f,   0.00244f, 0.019185166f),
		float4(0.0633f,  0.0603f,  0.0372f,  0.004406822f),
		float4(0.00289f, 0.00263f, 0.00227f, 0.001797238f),
		float4(0.000547f,0.00133f, 0.0013f,  0.001568741f),
		float4(0.0192f,  0.0156f,  0.0104f,  0.00573269f),
		float4(0.00272f, 0.0022f,  0.00169f, 0.005357428f),
		float4(0.0173f,  0.0142f,  0.00972f, 0.00116849f),
		float4(0.00793f, 0.0134f,  0.0198f,  0.000659981f),
		float4(0.0425f,  0.0698f,  0.095f,   0.004742176f),
		float4(0.0382f,  0.0272f,  0.0119f,  0.018918134f),
		float4(0.0173f,  0.0221f,  0.0302f,  0.011137844f),
		float4(0.1f,     0.0613f,  0.0223f,  0.022533376f),
		float4(0.0633f,  0.0452f,  0.0226f,  0.03561164f),
		float4(0.0119f,  0.025f,   0.00997f, 0.003614732f),
		float4(0.0837f,  0.0128f,  0.00775f, 0.003173843f),
		float4(0.0432f,  0.0167f,  0.00699f, 0.00796655f),
		float4(0.00817f, 0.0063f,  0.00474f, 0.015880952f),
		float4(0.0299f,  0.0273f,  0.0196f,  0.017748664f),
		float4(0.0916f,  0.027f,   0.00942f, 0.00776415f),
		float4(0.0749f,  0.0414f,  0.027f,   0.04878524f),
		float4(0.012f,   0.0143f,  0.0267f,  0.005505024f),
		float4(0.256f,   0.0341f,  0.0102f,  0.003083858f),
		float4(0.299f,   0.249f,   0.15f,    0.00661538f),
		float4(0.314f,   0.183f,   0.0874f,  0.000900249f),
		float4(0.25f,    0.148f,   0.088f,   0.000239042f),
		float4(0.0122f,  0.0058f,  0.00354f, 0.0000679821f),
		float4(0.0232f,  0.0216f,  0.0349f,  0.000260137f),
		float4(0.0474f,  0.0375f,  0.0302f,  0.002095382f),
		float4(0.0731f,  0.0894f,  0.0271f,  0.000378756f),
		float4(0.247f,   0.0676f,  0.0414f,  0.011542062f),
		float4(0.296f,   0.182f,   0.12f,    0.008505336f),
		float4(0.191f,   0.0204f,  0.00426f, 0.002339532f),
		float4(0.0515f,  0.0327f,  0.0141f,  0.008953678f),
		float4(0.164f,   0.0796f,  0.0205f,  0.02805926f),
		float4(0.102f,   0.0887f,  0.0573f,  0.005794026f),
		float4(0.00727f, 0.0219f,  0.0132f,  0.001732392f),
		float4(0.00479f, 0.0318f,  0.0267f,  0.01726392f),
		float4(0.016f,   0.0701f,  0.0538f,  0.000728329f),
		float4(0.0307f,  0.0267f,  0.0186f,  0.02363148f),
		float4(0.0591f,  0.0204f,  0.0062f,  0.00889345f),
		float4(0.152f,   0.023f,   0.00514f, 0.001304914f),
		float4(0.191f,   0.0302f,  0.0187f,  0.00192188f),
		float4(0.0152f,  0.00973f, 0.0177f,  0.01935222f),
		float4(0.069f,   0.0323f,  0.00638f, 0.04748044f),
		float4(0.0695f,  0.0628f,  0.0446f,  0.06273436f),
		float4(0.102f,   0.036f,   0.00995f, 0.010106036f),
		float4(0.252f,   0.186f,   0.106f,   0.006244098f),
		float4(0.0301f,  0.0257f,  0.0173f,  0.05095432f),
		float4(0.236f,   0.204f,   0.127f,   0.0048657f),
		float4(0.325f,   0.0469f,  0.00486f, 0.000790428f),
		float4(0.096f,   0.0534f,  0.0184f,  0.004913678f),
		float4(0.41f,    0.124f,   0.00683f, 0.005298618f),
		float4(0.00198f, 0.0022f,  0.00203f, 0.010628338f),
		float4(0.418f,   0.0415f,  0.00847f, 0.005128198f),
		float4(0.181f,   0.129f,   0.0776f,  0.03923218f),
		float4(0.29f,    0.161f,   0.0139f,  0.011645952f),
		float4(0.189f,   0.146f,   0.0861f,  0.03621944f),
		float4(0.288f,   0.18f,    0.0597f,  0.00396387f),
		float4(0.146f,   0.0968f,  0.0559f,  0.008632844f),
		float4(0.201f,   0.109f,   0.0599f,  0.004287808f),
		float4(0.388f,   0.0835f,  0.043f,   0.001855682f),
		float4(0.267f,   0.236f,   0.161f,   0.00399621f),
		float4(0.0555f,  0.0578f,  0.0432f,  0.00786954f),
		float4(0.0194f,  0.0152f,  0.0105f,  0.000269286f),
		float4(0.0876f,  0.0322f,  0.0165f,  0.0f),
		float4(0.00498f, 0.00255f, 0.00151f, 0.00452736f),
		float4(0.289f,   0.22f,    0.13f,    0.004219234f),
		float4(0.0275f,  0.00723f, 0.00123f, 0.011270334f),
		float4(0.273f,   0.0276f,  0.0186f,  0.007131702f),
		float4(0.0316f,  0.0308f,  0.0238f,  0.006442842f),
		float4(0.302f,   0.0316f,  0.00636f, 0.000794115f),
		float4(0.132f,   0.0182f,  0.00668f, 0.006105202f),
		float4(0.00568f, 0.00249f, 0.00118f, 0.010215618f),
		float4(0.167f,   0.0245f,  0.00789f, 0.001915708f),
		float4(0.276f,   0.0456f,  0.0109f,  0.006771836f),
		float4(0.242f,   0.0316f,  0.00946f, 0.001691485f),
		float4(0.161f,   0.0841f,  0.0537f,  0.001886298f),
		float4(0.0146f,  0.011f,   0.00606f, 0.00320641f),
		float4(0.021f,   0.0162f,  0.0106f,  0.009570082f),
		float4(0.0303f,  0.0187f,  0.0122f,  0.017150268f),
		float4(0.0156f,  0.0162f,  0.0112f,  0.011789742f),
		float4(0.345f,   0.291f,   0.196f,   0.0f),
		float4(0.303f,   0.261f,   0.178f,   0.009031378f),
		float4(0.026f,   0.0172f,  0.00442f, 0.01779272f),
		float4(0.0708f,  0.0167f,  0.013f,   0.000694197f),
		float4(0.245f,   0.053f,   0.0749f,  0.004880858f),
		float4(0.00321f, 0.00218f, 0.00141f, 0.01029147f),
		float4(0.284f,   0.196f,   0.075f,   0.000683875f),
		float4(0.317f,   0.234f,   0.107f,   0.006194334f),
		float4(0.312f,   0.265f,   0.178f,   0.005380284f),
		float4(0.307f,   0.118f,   0.0101f,  0.000367343f),
		float4(0.293f,   0.104f,   0.0162f,  0.003435382f),
		float4(0.253f,   0.187f,   0.0263f,  0.00373529f)
	};

	float shininess_parametes[] =
	{
		1,
		1,
		1,
		1,
		1,
		1,
		1,
		1,
		1,
		1,
		0.945208645f,
		0.458015101f,
		0.910586764f,
		0.583458654f,
		1,
		0.418942787f,
		1,
		1,
		0.943868871f,
		0.48518681f,
		1,
		1,
		0.489177717f,
		1,
		1,
		0.459261041f,
		1,
		0.382160827f,
		0.391669218f,
		0.490921191f,
		0.49850679f,
		0.562529458f,
		0.490521275f,
		0.525606924f,
		0.332456007f,
		0.610056718f,
		0.257730557f,
		0.284649209f,
		0.358103987f,
		0.541032539f,
		0.400125682f,
		0.77095137f,
		1,
		0.474609615f,
		1,
		1,
		0.493160556f,
		1,
		1,
		0.407419801f,
		0.414529103f,
		0.479139899f,
		0.502892822f,
		0.490387649f,
		0.77095137f,
		0.596014835f,
		1,
		1,
		0.353610396f,
		0.695722625f,
		0.380012827f,
		0.409101295f,
		0.244086726f,
		0.368601082f,
		0.930769633f,
		0.495355626f,
		0.828703016f,
		0.388366101f,
		0.346997071f,
		0.612307841f,
		0.508142297f,
		0.041234838f,
		0.581122219f,
		0.404559422f,
		0.541876471f,
		0.596014835f,
		0.65685837f,
		1,
		0.472901056f,
		0.514346194f,
		1,
		0.409932584f,
		1,
		0.94454078f,
		1,
		0.90351341f,
		1,
		1,
		0.001104253f,
		0.459966777f,
		1,
		1,
		0.419956278f,
		0.631496413f,
		1,
		0.487817693f,
		0.689453539f,
		1,
		0.791362491f,
		0.423187627f
	};


	enum
	{
		Exit,
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape),
	};


	uint32_t ReverseBits(uint32_t bits)
	{
		bits = (bits << 16) | (bits >> 16);
		bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1);
		bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2);
		bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4);
		bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8);
		return bits;
	}

	float RadicalInverseVdC(uint32_t bits)
	{
		return ReverseBits(bits) * 2.3283064365386963e-10f; // / 0x100000000
	}

	// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
	float2 Hammersley2D(uint32_t i, uint32_t N)
	{
		return float2(static_cast<float>(i) / N, RadicalInverseVdC(i));
	}

	float3 ImportanceSampleBP(float2 const & xi, float roughness)
	{
		float phi = 2 * PI * xi.x();
		float cos_theta = pow(1 - xi.y() * (roughness + 1) / (roughness + 2), 1 / (roughness + 1));
		float sin_theta = sqrt(1 - cos_theta * cos_theta);
		return float3(sin_theta * cos(phi), sin_theta * sin(phi), cos_theta);
	}

	float GImplicit(float n_dot_v, float n_dot_l)
	{
		return n_dot_v * n_dot_l;
	}

	float2 IntegrateBRDFBP(float roughness, float n_dot_v)
	{
		float3 view(sqrt(1.0f - n_dot_v * n_dot_v), 0, n_dot_v);
		float2 rg(0, 0);

		uint32_t const NUM_SAMPLES = 1024;
		for (uint32_t i = 0; i < NUM_SAMPLES; ++ i)
		{
			float2 xi = Hammersley2D(i, NUM_SAMPLES);
			float3 h = ImportanceSampleBP(xi, roughness);
			float3 l = -MathLib::reflect(view, h);
			float n_dot_l = MathLib::clamp(l.z(), 0.0f, 1.0f);
			float n_dot_h = MathLib::clamp(h.z(), 0.0f, 1.0f);
			float v_dot_h = MathLib::clamp(MathLib::dot(view, h), 0.0f, 1.0f);
			if (n_dot_l > 0)
			{
				float g = GImplicit(n_dot_v, n_dot_l);
				float g_vis = g * v_dot_h / std::max(1e-6f, n_dot_h * n_dot_v);
				float fc = pow(1 - v_dot_h, 5);
				rg += float2(1 - fc, fc) * g_vis;
			}
		}

		return rg / static_cast<float>(NUM_SAMPLES);
	}

	TexturePtr GenIntegrateBRDF()
	{
		uint32_t const WIDTH = 128;
		uint32_t const HEIGHT = 128;

		std::vector<uint8_t> integrate_brdf_gr(WIDTH * HEIGHT * 2, 0);
		for (uint32_t y = 0; y < HEIGHT; ++ y)
		{
			float roughness = (y + 0.5f) / HEIGHT;
			roughness = pow(8192.0f, roughness);
			for (uint32_t x = 0; x < WIDTH; ++ x)
			{
				float cos_theta = (x + 0.5f) / WIDTH;

				float2 lut = IntegrateBRDFBP(roughness, cos_theta);
				integrate_brdf_gr[(y * WIDTH + x) * 2 + 0]
					= static_cast<uint8_t>(MathLib::clamp(static_cast<int>(lut.x() * 255 + 0.5f), 0, 255));
				integrate_brdf_gr[(y * WIDTH + x) * 2 + 1]
					= static_cast<uint8_t>(MathLib::clamp(static_cast<int>(lut.y() * 100 * 255 + 0.5f), 0, 255));
			}
		}

		std::vector<uint8_t> integrated_brdf_bc5(WIDTH * HEIGHT);
		TexCompressionBC5 bc5_codec;
		bc5_codec.EncodeMem(WIDTH, HEIGHT, &integrated_brdf_bc5[0], WIDTH * 4, WIDTH * HEIGHT,
			&integrate_brdf_gr[0], WIDTH * 2, WIDTH * HEIGHT * 2, TCM_Quality);

		std::vector<ElementInitData> init_data(1);
		init_data[0].data = &integrated_brdf_bc5[0];
		init_data[0].row_pitch = WIDTH * 4;
		init_data[0].slice_pitch = WIDTH * HEIGHT;
		
		RenderFactory& rf = Context::Instance().RenderFactoryInstance(); 
		return rf.MakeTexture2D(WIDTH, HEIGHT, 1, 1, EF_BC5, 1, 0, EAH_GPU_Read | EAH_Immutable, &init_data[0]);
	}
}


int SampleMain()
{
	EnvLightingApp app;
	app.Create();
	app.Run();

	return 0;
}

EnvLightingApp::EnvLightingApp()
		: App3DFramework("EnvLighting"),
			obj_controller_(true, MB_Left, MB_Middle, 0)
{
	ResLoader::Instance().AddPath("../../Samples/media/EnvLighting");
}

bool EnvLightingApp::ConfirmDevice() const
{
	return true;
}

void EnvLightingApp::OnCreate()
{
	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();

	font_ = SyncLoadFont("gkai00mp.kfont");

	TexturePtr y_cube_map = ASyncLoadTexture("uffizi_cross_filtered_y.dds", EAH_GPU_Read | EAH_Immutable);
	TexturePtr c_cube_map = ASyncLoadTexture("uffizi_cross_filtered_c.dds", EAH_GPU_Read | EAH_Immutable);
	if (ResLoader::Instance().Locate("IntegratedBRDF.dds").empty())
	{
		SaveTexture(GenIntegrateBRDF(), "../../Samples/media/EnvLighting/IntegratedBRDF.dds");
	}
	integrate_brdf_tex_ = ASyncLoadTexture("IntegratedBRDF.dds", EAH_GPU_Read | EAH_Immutable);

	AmbientLightSourcePtr ambient_light = MakeSharedPtr<AmbientLightSource>();
	ambient_light->SkylightTex(y_cube_map, c_cube_map);
	ambient_light->AddToSceneManager();

	uint32_t spheres_row = 10;
	uint32_t spheres_column = 10; 
	spheres_.resize(spheres_row * spheres_column);
	for (uint32_t i = 0; i < spheres_row; ++ i)
	{
		for (uint32_t j = 0; j < spheres_column; ++ j)
		{
			spheres_[i * spheres_column + j] = MakeSharedPtr<SphereObject>(diff_spec_parametes[i * spheres_column + j],
				shininess_parametes[i * spheres_column + j]);
			spheres_[i * spheres_column + j]->ModelMatrix(MathLib::scaling(1.3f, 1.3f, 1.3f)
				* MathLib::translation((-static_cast<float>(spheres_column / 2) + j + 0.5f) * 0.08f,
										0.0f, 
									   (-static_cast<float>(spheres_row / 2) + i + 0.5f) * 0.08f));
			checked_pointer_cast<SphereObject>(spheres_[i * spheres_column + j])->IntegrateBRDFTex(integrate_brdf_tex_);
			spheres_[i * spheres_column + j]->AddToSceneManager();
		}
	}

	sky_box_ = MakeSharedPtr<SceneObjectSkyBox>(0);
	checked_pointer_cast<SceneObjectSkyBox>(sky_box_)->CompressedCubeMap(y_cube_map, c_cube_map);
	sky_box_->AddToSceneManager();

	this->LookAt(float3(0.0f, 0.2f, -0.6f), float3(0, 0, 0));
	this->Proj(0.05f, 100);

	obj_controller_.AttachCamera(this->ActiveCamera());
	obj_controller_.Scalers(0.003f, 0.003f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(std::bind(&EnvLightingApp::InputHandler, this, std::placeholders::_1, std::placeholders::_2));
	inputEngine.ActionMap(actionMap, input_handler);

	UIManager::Instance().Load(ResLoader::Instance().Open("EnvLighting.uiml"));

	dialog_ = UIManager::Instance().GetDialog("Method");
	id_type_combo_ = dialog_->IDFromName("TypeCombo");

	dialog_->Control<UIComboBox>(id_type_combo_)->OnSelectionChangedEvent().connect(std::bind(&EnvLightingApp::TypeChangedHandler, this, std::placeholders::_1));
	this->TypeChangedHandler(*dialog_->Control<UIComboBox>(id_type_combo_));

	if (caps.max_shader_model < ShaderModel(4, 0))
	{
		dialog_->Control<UIComboBox>(id_type_combo_)->RemoveItem(4);
		dialog_->Control<UIComboBox>(id_type_combo_)->RemoveItem(3);
	}
}

void EnvLightingApp::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls();
}

void EnvLightingApp::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;
	}
}

void EnvLightingApp::TypeChangedHandler(KlayGE::UIComboBox const & sender)
{
	rendering_type_ = sender.GetSelectedIndex();
	for (size_t i = 0; i < spheres_.size(); ++ i)
	{
		checked_pointer_cast<SphereObject>(spheres_[i])->RenderingType(rendering_type_);
	}
}

void EnvLightingApp::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Env Lighting", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
}

uint32_t EnvLightingApp::DoUpdate(uint32_t /*pass*/)
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	re.CurFrameBuffer()->Attached(FrameBuffer::ATT_DepthStencil)->ClearDepthStencil(1.0f, 0);

	return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
}
