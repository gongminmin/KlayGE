#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/RenderBuffer.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>

#include <KlayGE/D3D9/D3D9RenderSettings.hpp>
#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <vector>
#include <sstream>

#include "VertexDisplacement.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	int const WIDTH = 4;
	int const HEIGHT = 3;

	int const MAX_X = 8;
	int const MAX_Y = 6;

#pragma pack(push, 1)

	struct
	{
		uint8_t		infoLength;
		uint8_t		colorMapType;
		uint8_t		imageTypeCode;

		int16_t		colorMapEntry;
		int16_t		colorMapLength;
		uint8_t		colorMapBits;

		int16_t		leftbottomX;
		int16_t		leftbottomY;

		int16_t		width;
		int16_t		height;

		uint8_t		pixelSize;
		uint8_t		imageDescriptor;
	} TGAHeader;

#pragma pack(pop)


	KlayGE::TexturePtr LoadTGA(std::istream& file)
	{
		try
		{
			file.read(reinterpret_cast<char*>(&TGAHeader), sizeof(TGAHeader));
			file.seekg(TGAHeader.infoLength, std::ios_base::cur);

			KlayGE::TexturePtr texture(Context::Instance().RenderFactoryInstance().MakeTexture(TGAHeader.width,
				TGAHeader.height, 0, PF_X8R8G8B8));

			vector<uint8_t> data(TGAHeader.width * TGAHeader.height * TGAHeader.pixelSize / 8);
			file.read(reinterpret_cast<char*>(&data[0]), data.size());

			vector<uint8_t> tgaData;
			tgaData.reserve(TGAHeader.width * TGAHeader.height * 4);
			for (short y = 0; y < TGAHeader.height; ++ y)
			{
				short line(y);
				if (0 == (TGAHeader.imageDescriptor & 0x20))
				{
					// 图像从下到上
					line = TGAHeader.height - y - 1;
				}

				for (short x = 0; x < TGAHeader.width; ++ x)
				{
					size_t const offset((line * TGAHeader.width + x) * (TGAHeader.pixelSize / 8));

					tgaData.push_back(data[offset + 0]);
					tgaData.push_back(data[offset + 1]);
					tgaData.push_back(data[offset + 2]);
					tgaData.push_back(0xFF);
				}
			}

			texture->CopyMemoryToTexture(0, &tgaData[0], PF_X8R8G8B8, texture->Width(), texture->Height(), 0, 0);
			texture->BuildMipSubLevels();

			return texture;
		}
		catch (...)
		{
			return KlayGE::TexturePtr();
		}
	}

	class Flag : public Renderable
	{
	public:
		Flag()
			: rb_(new RenderBuffer(RenderBuffer::BT_TriangleList))
		{
			std::vector<float> pos;
			for (int y = 0; y < MAX_Y * 2 + 1; ++ y)
			{
				for (int x = 0; x < MAX_X * 2 + 1; ++ x)
				{
					pos.push_back(+x * (0.5f * WIDTH / MAX_X));
					pos.push_back(-y * (0.5f * HEIGHT / MAX_Y));
					pos.push_back(0.0f);
				}
			}

			std::vector<float> tex;
			for (int y = 0; y < MAX_Y * 2 + 1; ++ y)
			{
				for (int x = 0; x < MAX_X * 2 + 1; ++ x)
				{
					tex.push_back(x / (MAX_X * 2.0f));
					tex.push_back(y / (MAX_Y * 2.0f));
				}
			}

			std::vector<uint16_t> index;
			for (int y = 0; y < MAX_Y * 2; ++ y)
			{
				for (int x = 0; x < MAX_X * 2; ++ x)
				{
					index.push_back((y + 0) * (MAX_X * 2 + 1) + (x + 0));
					index.push_back((y + 0) * (MAX_X * 2 + 1) + (x + 1));
					index.push_back((y + 1) * (MAX_X * 2 + 1) + (x + 1));

					index.push_back((y + 1) * (MAX_X * 2 + 1) + (x + 1));
					index.push_back((y + 1) * (MAX_X * 2 + 1) + (x + 0));
					index.push_back((y + 0) * (MAX_X * 2 + 1) + (x + 0));
				}
			}

			effect_ = LoadRenderEffect("VertexDisplacement.fx");
			*(effect_->ParameterByName("flag")) = LoadTGA(*(ResLoader::Instance().Load("Flag.tga")));
			effect_->SetTechnique("VertexDisplacement");

			rb_->AddVertexStream(VST_Positions, sizeof(float), 3, true);
			rb_->AddVertexStream(VST_TextureCoords0, sizeof(float), 2, true);
			rb_->GetVertexStream(VST_Positions)->Assign(&pos[0], pos.size() / 3);
			rb_->GetVertexStream(VST_TextureCoords0)->Assign(&tex[0], tex.size() / 2);

			rb_->AddIndexStream(true);
			rb_->GetIndexStream()->Assign(&index[0], index.size());

			box_ = Box(Vector3(0, 0, 0), Vector3(0, 0, 0));
		}

		RenderEffectPtr GetRenderEffect() const
		{
			return effect_;
		}

		RenderBufferPtr GetRenderBuffer() const
		{
			return rb_;
		}

		Box GetBound() const
		{
			return box_; 
		}

		std::wstring const & Name() const
		{
			static const std::wstring name(L"Flag");
			return name;
		}

		KlayGE::RenderBufferPtr rb_;
		KlayGE::RenderEffectPtr effect_;

		Box box_;
	};

	boost::shared_ptr<Flag> flag;
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
		return true;
	}
};

int main()
{
	VertexDisplacement app;
	SceneManager sceneMgr;

	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
	Context::Instance().SceneManagerInstance(sceneMgr);

	TheRenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.colorDepth = 32;
	settings.fullScreen = false;

	app.Create("VertexDisplacement", settings);
	app.Run();

	return 0;
}

VertexDisplacement::VertexDisplacement()
{
	ResLoader::Instance().AddPath("../media");
	ResLoader::Instance().AddPath("../media/VertexDisplacement");
}

void VertexDisplacement::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("SIMYOU.TTF", 16);

	flag = boost::shared_ptr<Flag>(new Flag);

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	Matrix4 matWorld;
	MathLib::Translation(matWorld, -WIDTH / 2.0f, HEIGHT / 2.0f, 0.0f);

	Matrix4 matView;
	MathLib::LookAtLH(matView, Vector3(0, 0, -10), Vector3(0, 0, 0));

	Matrix4 matProj;
	MathLib::PerspectiveFovLH(matProj, PI / 4, 800.0f / 600, 0.1f, 20.0f);

	*(flag->GetRenderEffect()->ParameterByName("worldviewproj")) = matWorld * matView * matProj;
}

void VertexDisplacement::Update()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	static float currentAngle = 0;
	*(flag->GetRenderEffect()->ParameterByName("currentAngle")) = currentAngle;
	currentAngle += 0.01f;
	if (currentAngle > 2 * PI)
	{
		currentAngle = 0;
	}

	std::wostringstream stream;
	stream << (*renderEngine.ActiveRenderTarget())->FPS();

	flag->Render();
	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"顶点位移");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str());
}
