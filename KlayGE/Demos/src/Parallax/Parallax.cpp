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
#include <KlayGE/ResLocator.hpp>
#include <KlayGE/DiskFile/DiskFile.hpp>
#include <KlayGE/D3D9/D3D9RenderSettings.hpp>
#include <KlayGE/D3D9/D3D9RenderFactory.hpp>
#include <KlayGE/OCTree/OCTree.hpp>

#include <vector>
#include <sstream>

#include "Parallax.hpp"

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
					size_t const offset((line * TGAHeader.width + x) * (TGAHeader.pixelSize / 8));

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

	class RenderPolygon : public Renderable
	{
	public:
		RenderPolygon()
			: rb_(new RenderBuffer(RenderBuffer::BT_TriangleList))
		{
			effect_ = LoadRenderEffect("Parallax.fx");
			*(effect_->ParameterByName("diffusemap")) = (LoadTGA(*(ResLocator::Instance().Locate("Diffuse.tga")->Load())));
			*(effect_->ParameterByName("normalmap")) = (LoadTGA(*(ResLocator::Instance().Locate("Normal.tga")->Load())));
			*(effect_->ParameterByName("heightmap")) = (LoadTGA(*(ResLocator::Instance().Locate("Height.tga")->Load())));
			effect_->SetTechnique("Parallax");

			Vector3 xyzs[] =
			{
				Vector3(-1, 1,  0),
				Vector3(1,	1,	0),
				Vector3(1,	-1,	1),
				Vector3(-1, -1, 1),
			};

			Vector2 texs[] =
			{
				Vector2(0, 0),
				Vector2(1, 0),
				Vector2(1, 1),
				Vector2(0, 1)
			};

			U16 indices[] = 
			{
				0, 1, 2, 2, 3, 0
			};

			MathLib::ComputeBoundingBox(box_, &xyzs[0], &xyzs[4]);

			Vector3 t[4], b[4];
			MathLib::ComputeTangent(t, b,
				indices, indices + sizeof(indices) / sizeof(indices[0]),
				xyzs, xyzs + sizeof(xyzs) / sizeof(xyzs[0]), texs);

			rb_->AddVertexStream(VST_Positions, sizeof(float), 3);
			rb_->AddVertexStream(VST_TextureCoords0, sizeof(float), 2);
			rb_->AddVertexStream(VST_TextureCoords1, sizeof(float), 3);
			rb_->AddVertexStream(VST_TextureCoords2, sizeof(float), 3);
			rb_->GetVertexStream(VST_Positions)->Assign(xyzs, sizeof(xyzs) / sizeof(xyzs[0]));
			rb_->GetVertexStream(VST_TextureCoords0)->Assign(texs, sizeof(texs) / sizeof(texs[0]));
			rb_->GetVertexStream(VST_TextureCoords1)->Assign(t, sizeof(t) / sizeof(t[0]));
			rb_->GetVertexStream(VST_TextureCoords2)->Assign(b, sizeof(b) / sizeof(b[0]));

			rb_->AddIndexStream();
			rb_->GetIndexStream()->Assign(indices, sizeof(indices) / sizeof(U16));
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
			static const std::wstring name(L"Polygon");
			return name;
		}

		KlayGE::RenderBufferPtr rb_;
		KlayGE::RenderEffectPtr effect_;

		Box box_;
	};

	boost::shared_ptr<RenderPolygon> renderPolygon;
}


class TheRenderSettings : public D3D9RenderSettings
{
private:
	bool DoConfirmDevice(D3DCAPS9 const & caps, U32 behavior, D3DFORMAT format) const
	{
		if (caps.VertexShaderVersion < D3DVS_VERSION(1, 1))
		{
			return false;
		}
		if (caps.PixelShaderVersion < D3DPS_VERSION(1, 4))
		{
			return false;
		}
		return true;
	}
};

int main()
{
	Parallax app;
	SceneManager sceneMgr;

	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
	Context::Instance().SceneManagerInstance(sceneMgr);

	TheRenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.colorDepth = 32;
	settings.fullScreen = false;

	app.Create("Parallax", settings);
	app.Run();

	return 0;
}

Parallax::Parallax()
{
	ResLocator::Instance().AddPath("../media");
	ResLocator::Instance().AddPath("../media/Parallax");
}

void Parallax::InitObjects()
{
	// 建立字体
	font_ = Context::Instance().RenderFactoryInstance().MakeFont(L"宋体", 16);

	renderPolygon = boost::shared_ptr<RenderPolygon>(new RenderPolygon);

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	Matrix4 matView, matProj;
	MathLib::LookAtLH(matView, Vector3(2, 0, -2), Vector3(0, 0, 0));
	MathLib::PerspectiveFovLH(matProj, PI / 4, 800.0f / 600, 0.1f, 20.0f);

	*(renderPolygon->effect_->ParameterByName("worldviewproj")) = matView * matProj;
	*(renderPolygon->effect_->ParameterByName("eyePos")) = Vector4(2, 0, -2, 1) - Vector4(0, 0, 0, 0);
}

void Parallax::Update()
{
	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());

	static float degree(0);
	Vector3 lightPos(2, 0, -2);
	Matrix4 matRot;
	MathLib::RotationZ(matRot, degree);
	MathLib::TransformCoord(lightPos, lightPos, matRot);
	*(renderPolygon->effect_->ParameterByName("lightPos")) = Vector4(lightPos.x(), lightPos.y(), lightPos.z(), 1);
	degree += 0.01f;

	std::wostringstream stream;
	stream << (*renderEngine.ActiveRenderTarget())->FPS();

	sceneMgr.PushRenderable(renderPolygon);
	sceneMgr.PushRenderable(font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Parallax测试"));
	sceneMgr.PushRenderable(font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str()));
}
