#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/SharedPtr.hpp>
#include <KlayGE/RenderBuffer.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLocator.hpp>
#include <KlayGE/DiskFile/DiskFile.hpp>
#include <KlayGE/D3D9/D3D9RenderSettings.hpp>
#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <vector>
#include <sstream>

#include "VertexDisplacement.hpp"
#include "Flag.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
#pragma pack(push, 1)

	struct
	{
		U8		infoLength;
		U8		colorMapType;
		U8		imageTypeCode;

		short	colorMapEntry;
		short	colorMapLength;
		U8		colorMapBits;

		short	leftbottomX;
		short	leftbottomY;

		short	width;
		short	height;

		U8		pixelSize;
		U8		imageDescriptor;
	} TGAHeader;

#pragma pack(pop)


	KlayGE::TexturePtr LoadTGA(KlayGE::VFile& file)
	{
		try
		{
			file.Read(&TGAHeader, sizeof(TGAHeader));
			file.Seek(TGAHeader.infoLength, VFile::SM_Current);

			KlayGE::TexturePtr texture(Context::Instance().RenderFactoryInstance().MakeTexture(TGAHeader.width,
				TGAHeader.height, 0, PF_X8R8G8B8));

			vector<U8> data(TGAHeader.width * TGAHeader.height * TGAHeader.pixelSize / 8);
			file.Read(&data[0], data.size());

			vector<U8> tgaData;
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
					const size_t offset((line * TGAHeader.width + x) * (TGAHeader.pixelSize / 8));

					tgaData.push_back(data[offset + 0]);
					tgaData.push_back(data[offset + 1]);
					tgaData.push_back(data[offset + 2]);
					tgaData.push_back(0xFF);
				}
			}

			texture->CopyMemoryToTexture(&tgaData[0], PF_X8R8G8B8);

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
			effect_ = LoadRenderEffect("VertexDisplacement.fx");
			effect_->SetTexture("flag", LoadTGA(DiskFile("Flag.tga", VFile::OM_Read)));
			effect_->SetTechnique("VertexDisplacement");

			rb_->AddVertexStream(VST_Positions, sizeof(float), 3, true);
			rb_->AddVertexStream(VST_TextureCoords0, sizeof(float), 2, true);
			rb_->GetVertexStream(VST_Positions)->Assign(Pos, sizeof(Pos) / sizeof(float) / 3);
			rb_->GetVertexStream(VST_TextureCoords0)->Assign(Tex, sizeof(Tex) / sizeof(float) / 2);

			rb_->AddIndexStream(true);
			rb_->GetIndexStream()->Assign(Index, sizeof(Index) / sizeof(U16));
		}

		RenderEffectPtr GetRenderEffect() const
		{
			return effect_;
		}

		RenderBufferPtr GetRenderBuffer() const
		{
			return rb_;
		}

		const std::wstring& Name() const
		{
			static std::wstring name(L"Flag");
			return name;
		}

		KlayGE::RenderBufferPtr rb_;
		KlayGE::RenderEffectPtr effect_;
	};

	SharedPtr<Flag> flag;
}


int main()
{
	VertexDisplacement app;

	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());

	D3D9RenderSettings settings;
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
	ResLocator::Instance().AddPath("D:/KlayGE/RenderFX");
}

void VertexDisplacement::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont(L"宋体", 16);

	flag = SharedPtr<Flag>(new Flag);

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	Matrix4 matView;
	MathLib::LookAtLH(matView, Vector3(0, 0, -10), Vector3(0, 0, 0));

	Matrix4 matProj;
	MathLib::PerspectiveFovLH(matProj, PI / 4, 800.0f / 600, 0.1f, 20.0f);

	flag->effect_->SetMatrix("worldviewproj", matView * matProj);
}

void VertexDisplacement::Update()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	SceneManager& sceneMgr(SceneManager::Instance());

	static float currentAngle = 0;
	flag->effect_->SetFloat("currentAngle", currentAngle);
	currentAngle += 0.01f;
	if (currentAngle > 2 * PI)
	{
		currentAngle = 0;
	}

	std::wostringstream stream;
	stream << (*renderEngine.ActiveRenderTarget())->FPS();

	sceneMgr.PushRenderable(flag);
	sceneMgr.PushRenderable(font_->RenderText(0, 0, Color(1, 1, 0, 1), L"顶点位移"));
	sceneMgr.PushRenderable(font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str()));
}
