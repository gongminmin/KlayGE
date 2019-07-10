#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderView.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/SkyBox.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/Light.hpp>
#include <KlayGE/TexCompressionBC.hpp>
#include <KlayGE/RenderMaterial.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

//#define CALC_FITTING_TABLE

#include <vector>
#include <sstream>
#ifdef CALC_FITTING_TABLE
#include <iostream>
#include <iomanip>
#include <fstream>
#endif

#include "SampleCommon.hpp"
#include "EnvLighting.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class SphereRenderable : public StaticMesh
	{
	public:
		explicit SphereRenderable(std::wstring_view name)
			: StaticMesh(name)
		{
			effect_ = SyncLoadRenderEffect("EnvLighting.fxml");
			techs_[0] = effect_->TechniqueByName("PBFittingPrefiltered");
			techs_[1] = effect_->TechniqueByName("PBPrefiltered");
			techs_[2] = effect_->TechniqueByName("PBFittingError");
			techs_[3] = effect_->TechniqueByName("Prefiltered");
			techs_[4] = effect_->TechniqueByName("Approximate");
			techs_[5] = effect_->TechniqueByName("GroundTruth");
			this->RenderingType(0);

			{
				auto* ambient_light = Context::Instance().SceneManagerInstance().SceneRootNode().FirstComponentOfType<AmbientLightSource>();

				*(effect_->ParameterByName("skybox_Ycube_tex")) = ambient_light->SkylightTexY();
				*(effect_->ParameterByName("skybox_Ccube_tex")) = ambient_light->SkylightTexC();

				uint32_t const mip = ambient_light->SkylightTexY()->NumMipMaps();
				*(effect_->ParameterByName("diff_spec_mip")) = int2(mip - 1, mip - 2);
			}

			// From https://github.com/BIDS/colormap/blob/master/parula.py
			uint32_t const color_map[] =
			{
				Color(0.2081f, 0.1663f, 0.5292f, 1).ABGR(),
				Color(0.2116238095f, 0.1897809524f, 0.5776761905f, 1).ABGR(),
				Color(0.212252381f, 0.2137714286f, 0.6269714286f, 1).ABGR(),
				Color(0.2081f, 0.2386f, 0.6770857143f, 1).ABGR(),
				Color(0.1959047619f, 0.2644571429f, 0.7279f, 1).ABGR(),
				Color(0.1707285714f, 0.2919380952f, 0.779247619f, 1).ABGR(),
				Color(0.1252714286f, 0.3242428571f, 0.8302714286f, 1).ABGR(),
				Color(0.0591333333f, 0.3598333333f, 0.8683333333f, 1).ABGR(),
				Color(0.0116952381f, 0.3875095238f, 0.8819571429f, 1).ABGR(),
				Color(0.0059571429f, 0.4086142857f, 0.8828428571f, 1).ABGR(),
				Color(0.0165142857f, 0.4266f, 0.8786333333f, 1).ABGR(),
				Color(0.032852381f, 0.4430428571f, 0.8719571429f, 1).ABGR(),
				Color(0.0498142857f, 0.4585714286f, 0.8640571429f, 1).ABGR(),
				Color(0.0629333333f, 0.4736904762f, 0.8554380952f, 1).ABGR(),
				Color(0.0722666667f, 0.4886666667f, 0.8467f, 1).ABGR(),
				Color(0.0779428571f, 0.5039857143f, 0.8383714286f, 1).ABGR(),
				Color(0.079347619f, 0.5200238095f, 0.8311809524f, 1).ABGR(),
				Color(0.0749428571f, 0.5375428571f, 0.8262714286f, 1).ABGR(),
				Color(0.0640571429f, 0.5569857143f, 0.8239571429f, 1).ABGR(),
				Color(0.0487714286f, 0.5772238095f, 0.8228285714f, 1).ABGR(),
				Color(0.0343428571f, 0.5965809524f, 0.819852381f, 1).ABGR(),
				Color(0.0265f, 0.6137f, 0.8135f, 1).ABGR(),
				Color(0.0238904762f, 0.6286619048f, 0.8037619048f, 1).ABGR(),
				Color(0.0230904762f, 0.6417857143f, 0.7912666667f, 1).ABGR(),
				Color(0.0227714286f, 0.6534857143f, 0.7767571429f, 1).ABGR(),
				Color(0.0266619048f, 0.6641952381f, 0.7607190476f, 1).ABGR(),
				Color(0.0383714286f, 0.6742714286f, 0.743552381f, 1).ABGR(),
				Color(0.0589714286f, 0.6837571429f, 0.7253857143f, 1).ABGR(),
				Color(0.0843f, 0.6928333333f, 0.7061666667f, 1).ABGR(),
				Color(0.1132952381f, 0.7015f, 0.6858571429f, 1).ABGR(),
				Color(0.1452714286f, 0.7097571429f, 0.6646285714f, 1).ABGR(),
				Color(0.1801333333f, 0.7176571429f, 0.6424333333f, 1).ABGR(),
				Color(0.2178285714f, 0.7250428571f, 0.6192619048f, 1).ABGR(),
				Color(0.2586428571f, 0.7317142857f, 0.5954285714f, 1).ABGR(),
				Color(0.3021714286f, 0.7376047619f, 0.5711857143f, 1).ABGR(),
				Color(0.3481666667f, 0.7424333333f, 0.5472666667f, 1).ABGR(),
				Color(0.3952571429f, 0.7459f, 0.5244428571f, 1).ABGR(),
				Color(0.4420095238f, 0.7480809524f, 0.5033142857f, 1).ABGR(),
				Color(0.4871238095f, 0.7490619048f, 0.4839761905f, 1).ABGR(),
				Color(0.5300285714f, 0.7491142857f, 0.4661142857f, 1).ABGR(),
				Color(0.5708571429f, 0.7485190476f, 0.4493904762f, 1).ABGR(),
				Color(0.609852381f, 0.7473142857f, 0.4336857143f, 1).ABGR(),
				Color(0.6473f, 0.7456f, 0.4188f, 1).ABGR(),
				Color(0.6834190476f, 0.7434761905f, 0.4044333333f, 1).ABGR(),
				Color(0.7184095238f, 0.7411333333f, 0.3904761905f, 1).ABGR(),
				Color(0.7524857143f, 0.7384f, 0.3768142857f, 1).ABGR(),
				Color(0.7858428571f, 0.7355666667f, 0.3632714286f, 1).ABGR(),
				Color(0.8185047619f, 0.7327333333f, 0.3497904762f, 1).ABGR(),
				Color(0.8506571429f, 0.7299f, 0.3360285714f, 1).ABGR(),
				Color(0.8824333333f, 0.7274333333f, 0.3217f, 1).ABGR(),
				Color(0.9139333333f, 0.7257857143f, 0.3062761905f, 1).ABGR(),
				Color(0.9449571429f, 0.7261142857f, 0.2886428571f, 1).ABGR(),
				Color(0.9738952381f, 0.7313952381f, 0.266647619f, 1).ABGR(),
				Color(0.9937714286f, 0.7454571429f, 0.240347619f, 1).ABGR(),
				Color(0.9990428571f, 0.7653142857f, 0.2164142857f, 1).ABGR(),
				Color(0.9955333333f, 0.7860571429f, 0.196652381f, 1).ABGR(),
				Color(0.988f, 0.8066f, 0.1793666667f, 1).ABGR(),
				Color(0.9788571429f, 0.8271428571f, 0.1633142857f, 1).ABGR(),
				Color(0.9697f, 0.8481380952f, 0.147452381f, 1).ABGR(),
				Color(0.9625857143f, 0.8705142857f, 0.1309f, 1).ABGR(),
				Color(0.9588714286f, 0.8949f, 0.1132428571f, 1).ABGR(),
				Color(0.9598238095f, 0.9218333333f, 0.0948380952f, 1).ABGR(),
				Color(0.9661f, 0.9514428571f, 0.0755333333f, 1).ABGR(),
				Color(0.9763f, 0.9831f, 0.0538f, 1).ABGR()
			};

			ElementInitData init_data;
			init_data.data = color_map;
			init_data.row_pitch = sizeof(color_map);
			init_data.slice_pitch = init_data.row_pitch * 1;
			auto& rf = Context::Instance().RenderFactoryInstance();
			TexturePtr color_map_tex = rf.MakeTexture2D(static_cast<uint32_t>(std::size(color_map)), 1, 1, 1, EF_ABGR8,
				1, 0, EAH_GPU_Read | EAH_Immutable, MakeSpan<1>(init_data));
			*(effect_->ParameterByName("color_map")) = color_map_tex;
		}

		void Material(float4 const & diffuse, float4 const & specular, float glossiness)
		{
			*(effect_->ParameterByName("diffuse")) = float3(diffuse.x(), diffuse.y(), diffuse.z());
			*(effect_->ParameterByName("specular")) = float3(specular.x(), specular.y(), specular.z());
			*(effect_->ParameterByName("glossiness")) = glossiness;
		}

		void RenderingType(int type)
		{
			technique_ = techs_[type];
		}

		void IntegratedBRDFTex(TexturePtr const & tex)
		{
			*(effect_->ParameterByName("integrated_brdf_tex")) = tex;
		}

		void OnRenderBegin()
		{
			StaticMesh::OnRenderBegin();

			App3DFramework const & app = Context::Instance().AppInstance();
			Camera const & camera = app.ActiveCamera();

			float4x4 const mvp = model_mat_ * camera.ViewProjMatrix();

			*(effect_->ParameterByName("model")) = model_mat_;
			*(effect_->ParameterByName("mvp")) = mvp;
			*(effect_->ParameterByName("eye_pos")) = camera.EyePos();
		}

	private:
		array<RenderTechnique*, 6> techs_;
	};

	float4 const diff_parametes[] =
	{
		float4(0.0147f,  0.0332f,  0.064f,   1),
		float4(0.0183f,  0.0657f,  0.0248f,  1),
		float4(0.0712f,  0.0263f,  0.03f,    1),
		float4(0.314f,   0.259f,   0.156f,   1),
		float4(0.226f,   0.233f,   0.0489f,  1),
		float4(0.0864f,  0.0597f,  0.0302f,  1),
		float4(0.31f,    0.248f,   0.151f,   1),
		float4(0.0335f,  0.028f,   0.00244f, 1),
		float4(0.0633f,  0.0603f,  0.0372f,  1),
		float4(0.00289f, 0.00263f, 0.00227f, 1),
		float4(0.000547f,0.00133f, 0.0013f,  1),
		float4(0.0192f,  0.0156f,  0.0104f,  1),
		float4(0.00272f, 0.0022f,  0.00169f, 1),
		float4(0.0173f,  0.0142f,  0.00972f, 1),
		float4(0.00793f, 0.0134f,  0.0198f,  1),
		float4(0.0425f,  0.0698f,  0.095f,   1),
		float4(0.0382f,  0.0272f,  0.0119f,  1),
		float4(0.0173f,  0.0221f,  0.0302f,  1),
		float4(0.1f,     0.0613f,  0.0223f,  1),
		float4(0.0633f,  0.0452f,  0.0226f,  1),
		float4(0.0119f,  0.025f,   0.00997f, 1),
		float4(0.0837f,  0.0128f,  0.00775f, 1),
		float4(0.0432f,  0.0167f,  0.00699f, 1),
		float4(0.00817f, 0.0063f,  0.00474f, 1),
		float4(0.0299f,  0.0273f,  0.0196f,  1),
		float4(0.0916f,  0.027f,   0.00942f, 1),
		float4(0.0749f,  0.0414f,  0.027f,   1),
		float4(0.012f,   0.0143f,  0.0267f,  1),
		float4(0.256f,   0.0341f,  0.0102f,  1),
		float4(0.299f,   0.249f,   0.15f,    1),
		float4(0.314f,   0.183f,   0.0874f,  1),
		float4(0.25f,    0.148f,   0.088f,   1),
		float4(0.0122f,  0.0058f,  0.00354f, 1),
		float4(0.0232f,  0.0216f,  0.0349f,  1),
		float4(0.0474f,  0.0375f,  0.0302f,  1),
		float4(0.0731f,  0.0894f,  0.0271f,  1),
		float4(0.247f,   0.0676f,  0.0414f,  1),
		float4(0.296f,   0.182f,   0.12f,    1),
		float4(0.191f,   0.0204f,  0.00426f, 1),
		float4(0.0515f,  0.0327f,  0.0141f,  1),
		float4(0.164f,   0.0796f,  0.0205f,  1),
		float4(0.102f,   0.0887f,  0.0573f,  1),
		float4(0.00727f, 0.0219f,  0.0132f,  1),
		float4(0.00479f, 0.0318f,  0.0267f,  1),
		float4(0.016f,   0.0701f,  0.0538f,  1),
		float4(0.0307f,  0.0267f,  0.0186f,  1),
		float4(0.0591f,  0.0204f,  0.0062f,  1),
		float4(0.152f,   0.023f,   0.00514f, 1),
		float4(0.191f,   0.0302f,  0.0187f,  1),
		float4(0.0152f,  0.00973f, 0.0177f,  1),
		float4(0.069f,   0.0323f,  0.00638f, 1),
		float4(0.0695f,  0.0628f,  0.0446f,  1),
		float4(0.102f,   0.036f,   0.00995f, 1),
		float4(0.252f,   0.186f,   0.106f,   1),
		float4(0.0301f,  0.0257f,  0.0173f,  1),
		float4(0.236f,   0.204f,   0.127f,   1),
		float4(0.325f,   0.0469f,  0.00486f, 1),
		float4(0.096f,   0.0534f,  0.0184f,  1),
		float4(0.41f,    0.124f,   0.00683f, 1),
		float4(0.00198f, 0.0022f,  0.00203f, 1),
		float4(0.418f,   0.0415f,  0.00847f, 1),
		float4(0.181f,   0.129f,   0.0776f,  1),
		float4(0.29f,    0.161f,   0.0139f,  1),
		float4(0.189f,   0.146f,   0.0861f,  1),
		float4(0.288f,   0.18f,    0.0597f,  1),
		float4(0.146f,   0.0968f,  0.0559f,  1),
		float4(0.201f,   0.109f,   0.0599f,  1),
		float4(0.388f,   0.0835f,  0.043f,   1),
		float4(0.267f,   0.236f,   0.161f,   1),
		float4(0.0555f,  0.0578f,  0.0432f,  1),
		float4(0.0194f,  0.0152f,  0.0105f,  1),
		float4(0.0876f,  0.0322f,  0.0165f,  1),
		float4(0.00498f, 0.00255f, 0.00151f, 1),
		float4(0.289f,   0.22f,    0.13f,    1),
		float4(0.0275f,  0.00723f, 0.00123f, 1),
		float4(0.273f,   0.0276f,  0.0186f,  1),
		float4(0.0316f,  0.0308f,  0.0238f,  1),
		float4(0.302f,   0.0316f,  0.00636f, 1),
		float4(0.132f,   0.0182f,  0.00668f, 1),
		float4(0.00568f, 0.00249f, 0.00118f, 1),
		float4(0.167f,   0.0245f,  0.00789f, 1),
		float4(0.276f,   0.0456f,  0.0109f,  1),
		float4(0.242f,   0.0316f,  0.00946f, 1),
		float4(0.161f,   0.0841f,  0.0537f,  1),
		float4(0.0146f,  0.011f,   0.00606f, 1),
		float4(0.021f,   0.0162f,  0.0106f,  1),
		float4(0.0303f,  0.0187f,  0.0122f,  1),
		float4(0.0156f,  0.0162f,  0.0112f,  1),
		float4(0.345f,   0.291f,   0.196f,   1),
		float4(0.303f,   0.261f,   0.178f,   1),
		float4(0.026f,   0.0172f,  0.00442f, 1),
		float4(0.0708f,  0.0167f,  0.013f,   1),
		float4(0.245f,   0.053f,   0.0749f,  1),
		float4(0.00321f, 0.00218f, 0.00141f, 1),
		float4(0.284f,   0.196f,   0.075f,   1),
		float4(0.317f,   0.234f,   0.107f,   1),
		float4(0.312f,   0.265f,   0.178f,   1),
		float4(0.307f,   0.118f,   0.0101f,  1),
		float4(0.293f,   0.104f,   0.0162f,  1),
		float4(0.253f,   0.187f,   0.0263f,  1)
	};

	float4 const spec_parameters[] = 
	{
		float4(0.0016f,		0.00115f,	0.000709f,	1),
		float4(0.00161f,	0.00121f,	0.000781f,	1),
		float4(0.000956f,	0.000608f,	0.000389f,	1),
		float4(0.00103f,	0.000739f,	0.000481f,	1),
		float4(0.00135f,	0.000975f,	0.000641f,	1),
		float4(0.015f,		0.00818f,	0.00381f,	1),
		float4(0.00206f,	0.00133f,	0.00171f,	1),
		float4(0.0243f,		0.0187f,	0.00893f,	1),
		float4(0.00499f,	0.00443f,	0.00246f,	1),
		float4(0.00253f,	0.00166f,	0.000999f,	1),
		float4(0.00211f,	0.00148f,	0.000854f,	1),
		float4(0.00692f,	0.00559f,	0.00365f,	1),
		float4(0.00684f,	0.00508f,	0.00374f,	1),
		float4(0.0014f,		0.00114f,	0.000769f,	1),
		float4(0.000914f,	0.000609f,	0.000417f,	1),
		float4(0.00533f,	0.00471f,	0.00333f,	1),
		float4(0.0367f,		0.015f,		0.00537f,	1),
		float4(0.00854f,	0.0115f,	0.0152f,	1),
		float4(0.0396f,		0.019f,		0.00728f,	1),
		float4(0.0459f,		0.0342f,	0.0193f,	1),
		float4(0.00201f,	0.00428f,	0.00175f,	1),
		float4(0.0096f,		0.00153f,	0.000535f,	1),
		float4(0.0092f,		0.00786f,	0.00539f,	1),
		float4(0.0213f,		0.0151f,	0.00766f,	1),
		float4(0.0235f,		0.0169f,	0.00922f,	1),
		float4(0.00946f,	0.00753f,	0.00509f,	1),
		float4(0.0756f,		0.0437f,	0.0202f,	1),
		float4(0.0074f,		0.00513f,	0.00364f,	1),
		float4(0.00473f,	0.00271f,	0.00194f,	1),
		float4(0.00755f,	0.00654f,	0.00461f,	1),
		float4(0.00149f,	0.000803f,	0.000127f,	1),
		float4(0.000481f,	0.000186f,	0.000052f,	1),
		float4(0.000109f,	0.0000582f, 0.0000441f, 1),
		float4(0.000272f,	0.000254f,	0.000286f,	1),
		float4(0.00177f,	0.00223f,	0.00172f,	1),
		float4(0.000423f,	0.000389f,	0.000147f,	1),
		float4(0.0218f,		0.00913f,	0.00523f,	1),
		float4(0.0118f,		0.00811f,	0.00272f,	1),
		float4(0.00634f,	0.00133f,	0.00056f,	1),
		float4(0.0107f,		0.00876f,	0.00573f,	1),
		float4(0.0392f,		0.0265f,	0.0107f,	1),
		float4(0.00699f,	0.00566f,	0.0036f,	1),
		float4(0.00247f,	0.00158f,	0.00107f,	1),
		float4(0.0126f,		0.019f,		0.0138f,	1),
		float4(0.000998f,	0.00068f,	0.000413f,	1),
		float4(0.0269f,		0.0234f,	0.0163f,	1),
		float4(0.0102f,		0.00878f,	0.00617f,	1),
		float4(0.00197f,	0.00116f,	0.000782f,	1),
		float4(0.00289f,	0.00171f,	0.00117f,	1),
		float4(0.0216f,		0.0177f,	0.0291f,	1),
		float4(0.0738f,		0.0434f,	0.0104f,	1),
		float4(0.0742f,		0.0615f,	0.0412f,	1),
		float4(0.0117f,		0.00997f,	0.00676f,	1),
		float4(0.00732f,	0.00617f,	0.00381f,	1),
		float4(0.0665f,		0.0486f,	0.0285f,	1),
		float4(0.00572f,	0.00479f,	0.0031f,	1),
		float4(0.00125f,	0.00069f,	0.000432f,	1),
		float4(0.0057f,		0.00486f,	0.00313f,	1),
		float4(0.00928f,	0.00433f,	0.00317f,	1),
		float4(0.0127f,		0.0104f,	0.00679f,	1),
		float4(0.00735f,	0.00462f,	0.00362f,	1),
		float4(0.0443f,		0.0389f,	0.0276f,	1),
		float4(0.014f,		0.0115f,	0.00616f,	1),
		float4(0.0485f,		0.0346f,	0.0161f,	1),
		float4(0.00477f,	0.00387f,	0.00252f,	1),
		float4(0.00986f,	0.00855f,	0.00584f,	1),
		float4(0.00494f,	0.00423f,	0.00294f,	1),
		float4(0.00137f,	0.00204f,	0.00146f,	1),
		float4(0.00362f,	0.00425f,	0.00259f,	1),
		float4(0.00934f,	0.00771f,	0.00512f,	1),
		float4(0.000311f,	0.000266f,	0.000179f,	1),
		float4(0,			0,			0,			1),
		float4(0.00536f,	0.00443f,	0.00304f,	1),
		float4(0.00497f,	0.00421f,	0.0021f,	1),
		float4(0.0131f,		0.0111f,	0.00757f,	1),
		float4(0.00872f,	0.0069f,	0.00475f,	1),
		float4(0.00814f,	0.00619f,	0.00395f,	1),
		float4(0.00093f,	0.000782f,	0.000514f,	1),
		float4(0.00817f,	0.00571f,	0.00394f,	1),
		float4(0.0117f,		0.0101f,	0.00699f,	1),
		float4(0.00239f,	0.00185f,	0.00117f,	1),
		float4(0.00801f,	0.00662f,	0.00463f,	1),
		float4(0.00237f,	0.00156f,	0.000996f,	1),
		float4(0.00241f,	0.00181f,	0.0011f,	1),
		float4(0.00394f,	0.00299f,	0.00319f,	1),
		float4(0.011f,		0.00946f,	0.00645f,	1),
		float4(0.0232f,		0.0161f,	0.00974f,	1),
		float4(0.0189f,		0.0103f,	0.00561f,	1),
		float4(0,			0,			0,			1),
		float4(0.01f,		0.009f,		0.00649f,	1),
		float4(0.0268f,		0.0157f,	0.012f,		1),
		float4(0.00109f,	0.000602f,	0.000442f,	1),
		float4(0.00649f,	0.00455f,	0.00342f,	1),
		float4(0.0117f,		0.0102f,	0.00705f,	1),
		float4(0.001f,		0.000612f,	0.000465f,	1),
		float4(0.00716f,	0.00615f,	0.00379f,	1),
		float4(0.0068f,		0.00514f,	0.00358f,	1),
		float4(0.000493f,	0.000338f,	0.000288f,	1),
		float4(0.00424f,	0.00333f,	0.00211f,	1),
		float4(0.00376f,	0.00389f,	0.00213f,	1)
	};

	float const glossiness_parametes[] =
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
		Zoom,
		Exit,
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Zoom, MS_Z),
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

	float3 ImportanceSampleBP(float2 const & xi, float shininess)
	{
		float phi = 2 * PI * xi.x();
		float cos_theta = pow(1 - xi.y() * (shininess + 1) / (shininess + 2), 1 / (shininess + 1));
		float sin_theta = sqrt(1 - cos_theta * cos_theta);
		return float3(sin_theta * cos(phi), sin_theta * sin(phi), cos_theta);
	}

	float GImplicit(float n_dot_v, float n_dot_l)
	{
		return n_dot_v * n_dot_l;
	}

	float2 IntegrateBRDFBP(float shininess, float n_dot_v)
	{
		float3 view(sqrt(1.0f - n_dot_v * n_dot_v), 0, n_dot_v);
		float2 rg(0, 0);

		uint32_t const NUM_SAMPLES = 1024;
		for (uint32_t i = 0; i < NUM_SAMPLES; ++ i)
		{
			float2 xi = Hammersley2D(i, NUM_SAMPLES);
			float3 h = ImportanceSampleBP(xi, shininess);
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

	void GenIntegratedBRDF(uint32_t width, uint32_t height, std::vector<float2>& integrate_brdf_f32)
	{
		integrate_brdf_f32.resize(width * height);
		for (uint32_t y = 0; y < height; ++ y)
		{
			float shininess = Glossiness2Shininess((y + 0.5f) / height);
			for (uint32_t x = 0; x < width; ++ x)
			{
				float cos_theta = (x + 0.5f) / width;

				integrate_brdf_f32[y * width + x] = IntegrateBRDFBP(shininess, cos_theta);
			}
		}
	}

	TexturePtr GenIntegratedBRDF(uint32_t width, uint32_t height)
	{
		std::vector<float2> integrate_brdf_f32(width * height);
		GenIntegratedBRDF(width, height, integrate_brdf_f32);

		std::vector<uint8_t> integrate_brdf_gr(width * height * 2);
		for (uint32_t y = 0; y < height; ++ y)
		{
			for (uint32_t x = 0; x < width; ++ x)
			{
				float2 const lut = integrate_brdf_f32[y * width + x];
				integrate_brdf_gr[(y * width + x) * 2 + 0]
					= static_cast<uint8_t>(MathLib::clamp(static_cast<int>(lut.x() * 255 + 0.5f), 0, 255));
				integrate_brdf_gr[(y * width + x) * 2 + 1]
					= static_cast<uint8_t>(MathLib::clamp(static_cast<int>(lut.y() * 100 * 255 + 0.5f), 0, 255));
			}
		}

		ElementInitData init_data;
		init_data.data = &integrate_brdf_gr[0];
		init_data.row_pitch = width * 2;
		init_data.slice_pitch = init_data.row_pitch * height;

		TexturePtr ret = MakeSharedPtr<SoftwareTexture>(Texture::TT_2D, width, height, 1, 1, 1, EF_GR8, false);
		ret->CreateHWResource(MakeSpan<1>(init_data), nullptr);

		return ret;
	}

#ifdef CALC_FITTING_TABLE
	void GenFittedBRDF(uint32_t width, uint32_t height, std::vector<float2>& fitted_brdf_f32,
		std::span<float4 const> r_factors, std::span<float4 const> g_factors)
	{
		fitted_brdf_f32.resize(width * height);
		for (uint32_t y = 0; y < height; ++ y)
		{
			float glossiness = (y + 0.5f) / height;
			for (uint32_t x = 0; x < width; ++ x)
			{
				float n_dot_v = (x + 0.5f) / width;

				float2 env_brdf;
				float4 tmp = ((r_factors[0] * glossiness + r_factors[1]) * glossiness + r_factors[2]) * glossiness + r_factors[3];
				env_brdf.x() = (((tmp.x() * n_dot_v + tmp.y()) * n_dot_v + tmp.z()) * n_dot_v) + tmp.w();
				tmp = ((g_factors[0] * glossiness + g_factors[1]) * glossiness + g_factors[2]) * glossiness + g_factors[3];
				env_brdf.y() = (((tmp.x() * n_dot_v + tmp.y()) * n_dot_v + tmp.z()) * n_dot_v) + tmp.w();

				fitted_brdf_f32[y * width + x] = env_brdf;
			}
		}
	}

	void GenFittedBRDF(uint32_t width, uint32_t height, std::vector<float2>& fitted_brdf_f32)
	{
		std::array<float4, 4> const r_min_factors_base =
		{
			float4(3.221071959f, -4.037492752f, 2.019851685f, -0.3509000242f),
			float4(-5.483835697f, 4.748570442f, -2.599167109f, 0.8398050666f),
			float4(2.386495829f, 0.3970752358f, 0.1965616345f, -0.6608897448f),
			float4(-0.2426506728f, 0.05738930777f, 0.318114996f, 0.1741847545f),
		};
		std::array<float4, 4> const g_min_factors_base =
		{
			float4(-0.645807467f, 1.143745551f, -0.578012509f, 0.069540519f),
			float4(0.895991894f, -1.581523545f, 0.81029122f, -0.108531864f),
			float4(-0.088478638f, 0.154233504f, -0.098784305f, 0.029798974f),
			float4(0.001030646f, 0.008038982f, -0.016316089f, 0.007532373f),
		};

		GenFittedBRDF(width, height, fitted_brdf_f32,
			MakeSpan(r_min_factors_base.data(), r_min_factors_base.size()),
			MakeSpan(g_min_factors_base.data(), g_min_factors_base.size()));
	}

	TexturePtr GenFittedBRDF(uint32_t width, uint32_t height)
	{
		std::vector<float2> fitted_brdf_f32(width * height);
		GenFittedBRDF(width, height, fitted_brdf_f32);

		std::vector<uint8_t> fitted_brdf_gr(width * height * 2);
		for (uint32_t y = 0; y < height; ++ y)
		{
			for (uint32_t x = 0; x < width; ++ x)
			{
				float2 const & env_brdf = fitted_brdf_f32[y * width + x];

				fitted_brdf_gr[(y * width + x) * 2 + 0]
					= static_cast<uint8_t>(MathLib::clamp(static_cast<int>(env_brdf.x() * 255 + 0.5f), 0, 255));
				fitted_brdf_gr[(y * width + x) * 2 + 1]
					= static_cast<uint8_t>(MathLib::clamp(static_cast<int>(env_brdf.y() * 100 * 255 + 0.5f), 0, 255));
			}
		}

		ElementInitData init_data;
		init_data.data = &fitted_brdf_gr[0];
		init_data.row_pitch = width * 2;
		init_data.slice_pitch = init_data.row_pitch * height;

		TexturePtr ret = MakeSharedPtr<SoftwareTexture>(Texture::TT_2D, width, height, 1, 1, 1, EF_GR8, false);
		ret->CreateHWResource(MakeSpan(init_data), nullptr);

		return ret;
	}

	void CalcMse(std::vector<float2> const & ground_truth_table, std::vector<float2> const & test_table,
		uint32_t width, uint32_t height, float2& mse, float2& min_diff, float2& max_diff)
	{
		mse = float2(0, 0);
		min_diff = float2(1, 1);
		max_diff = float2(-1, -1);
		for (uint32_t y = 0; y < height; ++ y)
		{
			for (uint32_t x = 0; x < width; ++ x)
			{
				float2 const diff = ground_truth_table[y * width + x] - test_table[y * width + x];

				min_diff.x() = std::min(min_diff.x(), diff.x());
				max_diff.x() = std::max(max_diff.x(), diff.x());

				min_diff.y() = std::min(min_diff.y(), diff.y());
				max_diff.y() = std::max(max_diff.y(), diff.y());

				mse.x() += diff.x() * diff.x();
				mse.y() += diff.y() * diff.y();
			}
		}

		mse /= (width * height);
	}

	void CalcMse(Texture& ground_truth_tex, Texture& test_tex, float2& mse, float2& min_diff, float2& max_diff)
	{
		uint32_t const width = ground_truth_tex.Width(0);
		uint32_t const height = ground_truth_tex.Height(0);

		Texture::Mapper ground_truth_mapper(ground_truth_tex, 0, 0, TMA_Read_Only, 0, 0, width, height);
		Texture::Mapper test_mapper(test_tex, 0, 0, TMA_Read_Only, 0, 0, width, height);

		uint8_t* ground_truth_ptr = ground_truth_mapper.Pointer<uint8_t>();
		uint8_t* test_ptr = test_mapper.Pointer<uint8_t>();

		std::vector<float2> ground_truth_table(width * height);
		std::vector<float2> test_table(ground_truth_table.size());
		for (uint32_t y = 0; y < height; ++ y)
		{
			for (uint32_t x = 0; x < width; ++ x)
			{
				ground_truth_table[y * width + x] = float2(ground_truth_ptr[y * ground_truth_mapper.RowPitch() + x * 2 + 0] / 255.0f,
					ground_truth_ptr[y * ground_truth_mapper.RowPitch() + x * 2 + 1] / 100.0f / 255.0f);

				test_table[y * width + x] = float2(test_ptr[y * test_mapper.RowPitch() + x * 2 + 0] / 255.0f,
					test_ptr[y * test_mapper.RowPitch() + x * 2 + 1] / 100.0f / 255.0f);
			}
		}

		CalcMse(ground_truth_table, test_table, width, height,
			mse, min_diff, max_diff);
	}
#endif
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

void EnvLightingApp::OnCreate()
{
	uint32_t const WIDTH = 128;
	uint32_t const HEIGHT = 128;

	font_ = SyncLoadFont("gkai00mp.kfont");

	TexturePtr y_cube_map = ASyncLoadTexture("uffizi_cross_filtered_y.dds", EAH_GPU_Read | EAH_Immutable);
	TexturePtr c_cube_map = ASyncLoadTexture("uffizi_cross_filtered_c.dds", EAH_GPU_Read | EAH_Immutable);
	if (ResLoader::Instance().Locate("IntegratedBRDF.dds").empty())
	{
		SaveTexture(GenIntegratedBRDF(WIDTH, HEIGHT), "../../Samples/media/EnvLighting/IntegratedBRDF.dds");
	}

#ifdef CALC_FITTING_TABLE
	{
		uint32_t const width = 128;
		uint32_t const height = 32;

		std::vector<float2> integrated_brdf;
		GenIntegratedBRDF(width, height, integrated_brdf);

		std::vector<float2> fitted_brdf;
		GenFittedBRDF(width, height, fitted_brdf);

		std::ofstream ofs_x("IntegratedBRDF_128_32_x.csv");
		std::ofstream ofs_y("IntegratedBRDF_128_32_y.csv");

		ofs_x << std::setprecision(10);
		ofs_y << std::setprecision(10);

		for (uint32_t y = 0; y < height; ++ y)
		{
			for (uint32_t x = 0; x < width; ++ x)
			{
				float2 const lut = integrated_brdf[y * width + x];

				ofs_x << lut.x() << ',';
				ofs_y << lut.y() << ',';
			}
			ofs_x << endl;
			ofs_y << endl;
		}
	}

	{
		auto ground_truth_tex = GenIntegratedBRDF(WIDTH, HEIGHT);

		{
			auto test_tex = GenFittedBRDF(WIDTH, HEIGHT);

			float2 mse;
			float2 min_diff;
			float2 max_diff;
			CalcMse(*ground_truth_tex, *test_tex, mse, min_diff, max_diff);

			float2 psnr;
			psnr.x() = 10 * -log10(mse.x());
			psnr.y() = 10 * -log10(mse.y());

			std::cout << "Fitted table" << std::endl;

			std::cout << "Min: (" << min_diff.x() << ", " << min_diff.y() << ")" << std::endl;
			std::cout << "Max: (" << max_diff.x() << ", " << max_diff.y() << ")" << std::endl;
			std::cout << "MSE: (" << mse.x() << ", " << mse.y() << ")" << std::endl;
			std::cout << "PSNR: (" << psnr.x() << ", " << psnr.y() << ")" << std::endl << std::endl;
		}

		uint2 const size_combinations[] = 
		{
			uint2(WIDTH, HEIGHT / 4),
			uint2(WIDTH / 4, HEIGHT),
			uint2(WIDTH / 4, HEIGHT / 4),
			uint2(WIDTH, HEIGHT / 32),
			uint2(WIDTH / 32, HEIGHT),
			uint2(WIDTH / 32, HEIGHT / 32),
		};
		for (size_t i = 0; i < std::size(size_combinations); ++ i)
		{
			auto downsampled_tex = GenIntegratedBRDF(size_combinations[i].x(), size_combinations[i].y());

			auto test_tex = MakeSharedPtr<SoftwareTexture>(Texture::TT_2D, WIDTH, HEIGHT, 1, 1, 1, EF_GR8, false);
			test_tex->CreateHWResource({}, nullptr);
			downsampled_tex->CopyToTexture(*test_tex);

			float2 mse;
			float2 min_diff;
			float2 max_diff;
			CalcMse(*ground_truth_tex, *test_tex, mse, min_diff, max_diff);

			float2 psnr;
			psnr.x() = 10 * -log10(mse.x());
			psnr.y() = 10 * -log10(mse.y());

			std::cout << "Downsampled (" << downsampled_tex->Width(0) << ", " << downsampled_tex->Height(0) << ") table" << std::endl;

			std::cout << "Min: (" << min_diff.x() << ", " << min_diff.y() << ")" << std::endl;
			std::cout << "Max: (" << max_diff.x() << ", " << max_diff.y() << ")" << std::endl;
			std::cout << "MSE: (" << mse.x() << ", " << mse.y() << ")" << std::endl;
			std::cout << "PSNR: (" << psnr.x() << ", " << psnr.y() << ")" << std::endl << std::endl;
		}
	}
#endif

	auto& rf = Context::Instance().RenderFactoryInstance();
	auto const & caps = rf.RenderEngineInstance().DeviceCaps();
	ElementFormat const fmt = caps.BestMatchTextureFormat(MakeSpan({EF_GR8, EF_ABGR8, EF_ARGB8}));
	if (fmt == EF_GR8)
	{
		integrated_brdf_tex_ = ASyncLoadTexture("IntegratedBRDF.dds", EAH_GPU_Read | EAH_Immutable);
	}
	else
	{
		auto integrated_brdf_sw_tex = LoadSoftwareTexture("IntegratedBRDF.dds");

		integrated_brdf_tex_ = rf.MakeTexture2D(integrated_brdf_sw_tex->Width(0), integrated_brdf_sw_tex->Height(0), 1, 1, fmt, 1, 0, EAH_GPU_Read);
		integrated_brdf_sw_tex->CopyToTexture(*integrated_brdf_tex_);
	}

	auto& root_node = Context::Instance().SceneManagerInstance().SceneRootNode();

	AmbientLightSourcePtr ambient_light = MakeSharedPtr<AmbientLightSource>();
	ambient_light->SkylightTex(y_cube_map, c_cube_map);
	root_node.AddComponent(ambient_light);

	sphere_group_ = MakeSharedPtr<SceneNode>(SceneNode::SOA_Cullable);
	root_node.AddChild(sphere_group_);

	sphere_group_->OnMainThreadUpdate().Connect([this](SceneNode& node, float app_time, float elapsed_time)
		{
			KFL_UNUSED(app_time);
			KFL_UNUSED(elapsed_time);

			auto& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			auto const& camera = *re.CurFrameBuffer()->Viewport()->Camera();

			node.TransformToParent(MathLib::translation(0.0f, 0.0f, distance_) * camera.InverseViewMatrix());
		});

	auto sphere_model_unique = SyncLoadModel("sphere_high.glb", EAH_GPU_Read | EAH_Immutable,
		SceneNode::SOA_Cullable, nullptr,
		CreateModelFactory<RenderModel>, CreateMeshFactory<SphereRenderable>);

	sphere_models_.resize(std::size(diff_parametes));
	for (size_t i = 0; i < sphere_models_.size(); ++ i)
	{
		sphere_models_[i] = sphere_model_unique->Clone(CreateModelFactory<RenderModel>, CreateMeshFactory<SphereRenderable>);

		uint32_t const spheres_row = 10;
		uint32_t const spheres_column = 10;
		size_t const y = i / spheres_column;
		size_t const x = i - y * spheres_column;

		sphere_models_[i]->RootNode()->TransformToParent(MathLib::translation(
			(-static_cast<float>(spheres_column / 2) + x + 0.5f) * 0.06f,
			-(-static_cast<float>(spheres_row / 2) + y + 0.5f) * 0.06f,
			0.0f));

		sphere_models_[i]->ForEachMesh([i, this](Renderable& mesh)
			{
				auto& sphere_mesh = checked_cast<SphereRenderable&>(mesh);
				sphere_mesh.Material(diff_parametes[i], spec_parameters[i], glossiness_parametes[i]);
				sphere_mesh.IntegratedBRDFTex(integrated_brdf_tex_);
			});

		sphere_group_->AddChild(sphere_models_[i]->RootNode());
	}

	auto skybox = MakeSharedPtr<RenderableSkyBox>();
	skybox->CompressedCubeMap(y_cube_map, c_cube_map);
	root_node.AddChild(MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(skybox), SceneNode::SOA_NotCastShadow));

	this->LookAt(float3(0.0f, 0.0f, -0.8f), float3(0, 0, 0));
	this->Proj(0.05f, 100);

	obj_controller_.AttachCamera(this->ActiveCamera());
	obj_controller_.Scalers(0.003f, 0.003f);

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

	UIManager::Instance().Load(ResLoader::Instance().Open("EnvLighting.uiml"));

	dialog_ = UIManager::Instance().GetDialog("Method");
	id_type_combo_ = dialog_->IDFromName("TypeCombo");

	dialog_->Control<UIComboBox>(id_type_combo_)->OnSelectionChangedEvent().Connect(
		[this](UIComboBox const & sender)
		{
			this->TypeChangedHandler(sender);
		});
	this->TypeChangedHandler(*dialog_->Control<UIComboBox>(id_type_combo_));
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
	case Zoom:
		{
			auto const param = checked_pointer_cast<InputMouseActionParam>(action.second);
			float const delta = -param->wheel_delta / 120.0f * 0.05f;
			distance_ += delta;
		}
		break;

	case Exit:
		this->Quit();
		break;
	}
}

void EnvLightingApp::TypeChangedHandler(KlayGE::UIComboBox const & sender)
{
	rendering_type_ = sender.GetSelectedIndex();
	for (size_t i = 0; i < sphere_models_.size(); ++ i)
	{
		sphere_models_[i]->ForEachMesh([this](Renderable& mesh)
			{
				checked_cast<SphereRenderable&>(mesh).RenderingType(rendering_type_);
			});
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
	re.CurFrameBuffer()->AttachedDsv()->ClearDepthStencil(1.0f, 0);

	return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
}
