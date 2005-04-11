#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/VertexBuffer.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/PerlinNoise.hpp>

#include <KlayGE/D3D9/D3D9RenderSettings.hpp>
#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <vector>
#include <sstream>
#include <ctime>

#include "Electro.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderElectro : public Renderable
	{
	public:
		RenderElectro()
			: vb_(new VertexBuffer(VertexBuffer::BT_TriangleList))
		{
			MathLib::PerlinNoise<float>& pn = MathLib::PerlinNoise<float>::Instance();

			int const XSIZE = 128;
            int const YSIZE = 32;
			int const ZSIZE = 32;
			float const XSCALE = 0.08f;
			float const YSCALE = 0.16f;
			float const ZSCALE = 0.16f;

			std::vector<unsigned char> turbBuffer;
			turbBuffer.reserve(XSIZE * YSIZE * ZSIZE);
			unsigned int min = 255, max = 0;
			for (int z = 0; z < ZSIZE; ++ z)
			{
				for (int y = 0; y < YSIZE; ++ y)
				{
					for (int x = 0; x < XSIZE; ++ x)
					{
						unsigned char t = static_cast<unsigned char>(127.5f * (1 + pn.tileable_turbulence(XSCALE * x, YSCALE * y, ZSCALE * z, XSIZE * XSCALE, YSIZE * YSCALE, ZSIZE * ZSCALE, 16)));
						if (t > max)
						{
							max = t;
						}
						if (t < min)
						{
							min = t;
						}

						turbBuffer.push_back(t);
					}
				}
			}
			for (unsigned int i = 0; i < XSIZE * YSIZE * ZSIZE; ++ i)
			{
				turbBuffer[i] = (255 * (turbBuffer[i] - min)) / (max - min);
			}

			RenderFactory& renderFactory(Context::Instance().RenderFactoryInstance());
			TexturePtr texture = renderFactory.MakeTexture3D(XSIZE, YSIZE, ZSIZE, 1, PF_L8);
			texture->CopyMemoryToTexture3D(0, &turbBuffer[0], PF_L8, XSIZE, YSIZE, ZSIZE, 0, 0, 0);

			effect_ = LoadRenderEffect("Electro.fx");
			*(effect_->ParameterByName("electroMap")) = texture;
			effect_->SetTechnique("Electro");

			Vector3 xyzs[] =
			{
				Vector3(-0.8f, 0.8f, 1),
				Vector3(0.8f, 0.8f, 1),
				Vector3(-0.8f, 0, 1),
				Vector3(0.8f, 0, 1),
				Vector3(-0.8f, -0.8f, 1),
				Vector3(0.8f, -0.8f, 1),
			};

			Vector3 tex0s[] =
			{
				Vector3(-1, -1, 0),
				Vector3(1, -1, 0),
				Vector3(-1, 0, 0),
				Vector3(1, 0, 0),
				Vector3(-1, -1, 0),
				Vector3(1, -1, 0),
			};

			Vector2 tex1s[] =
			{
				Vector2(1, -0.4f),
				Vector2(1, 0.4f),
				Vector2(0, -0.4f),
				Vector2(0, 0.4f),
				Vector2(-1, -0.4f),
				Vector2(-1, 0.4f),
			};

			uint16_t indices[] = 
			{
				0, 1, 3, 3, 2, 0, 2, 3, 5, 5, 4, 2
			};

			MathLib::ComputeBoundingBox(box_, &xyzs[0], &xyzs[0] + sizeof(xyzs) / sizeof(xyzs[0]));

			vb_->AddVertexStream(VST_Positions, sizeof(float), 3);
			vb_->AddVertexStream(VST_TextureCoords0, sizeof(float), 3);
			vb_->AddVertexStream(VST_TextureCoords1, sizeof(float), 2);
			vb_->GetVertexStream(VST_Positions)->Assign(xyzs, sizeof(xyzs) / sizeof(xyzs[0]));
			vb_->GetVertexStream(VST_TextureCoords0)->Assign(tex0s, sizeof(tex0s) / sizeof(tex0s[0]));
			vb_->GetVertexStream(VST_TextureCoords1)->Assign(tex1s, sizeof(tex1s) / sizeof(tex1s[0]));

			vb_->AddIndexStream();
			vb_->GetIndexStream()->Assign(indices, sizeof(indices) / sizeof(uint16_t));
		}

		void OnRenderBegin()
		{
			float t = std::clock() * 0.0002f;

			*(effect_->ParameterByName("y")) = t * 2;
			*(effect_->ParameterByName("z")) = t;
		}

		RenderEffectPtr GetRenderEffect() const
		{
			return effect_;
		}

		VertexBufferPtr GetVertexBuffer() const
		{
			return vb_;
		}

		Box GetBound() const
		{
			return box_;
		}

		std::wstring const & Name() const
		{
			static const std::wstring name(L"Electro");
			return name;
		}

		KlayGE::VertexBufferPtr vb_;
		KlayGE::RenderEffectPtr effect_;

		Box box_;
	};

	boost::shared_ptr<RenderElectro> renderElectro;
}


class TheRenderSettings : public D3D9RenderSettings
{
private:
	bool DoConfirmDevice(D3DCAPS9 const & caps, uint32_t behavior, D3DFORMAT format) const
	{
		if (caps.VertexShaderVersion < D3DVS_VERSION(1, 1))
		{
			return false;
		}
		if (caps.PixelShaderVersion < D3DPS_VERSION(2, 0))
		{
			return false;
		}
		return true;
	}
};

int main()
{
	SceneManager sceneMgr;
	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
	Context::Instance().SceneManagerInstance(sceneMgr);

	TheRenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.colorDepth = 32;
	settings.fullScreen = false;

	Electro app;
	app.Create("Electro", settings);
	app.Run();

	return 0;
}

Electro::Electro()
{
	ResLoader::Instance().AddPath("../media");
	ResLoader::Instance().AddPath("../media/Electro");
}

void Electro::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("SIMYOU.TTF", 16);

	renderElectro = boost::shared_ptr<RenderElectro>(new RenderElectro);
	renderElectro->AddToSceneManager();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));
}

void Electro::Update()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	std::wostringstream stream;
	stream << (*renderEngine.ActiveRenderTarget())->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Electro²âÊÔ");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str());
}
