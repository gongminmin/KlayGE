#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/SharedPtr.hpp>
#include <KlayGE/RenderBuffer.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/alloc.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Engine.hpp>
#include <KlayGE/DiskFile/DiskFile.hpp>
#include <KlayGE/D3D9/D3D9RenderSettings.hpp>
#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <vector>
#include <sstream>

#include "PerPixelLighting.hpp"

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


	KlayGE::TexturePtr LoadTGA(const KlayGE::WString& fileName)
	{
		try
		{
			DiskFile file(fileName, VFile::OM_Read);
			file.Read(&TGAHeader, sizeof(TGAHeader));
			file.Seek(TGAHeader.infoLength, VFile::SM_Current);

			KlayGE::TexturePtr texture(Engine::RenderFactoryInstance().MakeTexture(TGAHeader.width,
				TGAHeader.height, 0, PF_X8R8G8B8));

			vector<U8, alloc<U8> > data(TGAHeader.width * TGAHeader.height * TGAHeader.pixelSize / 8);
			file.Read(&data[0], data.size());

			vector<U8, alloc<U8> > tgaData;
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

	class RenderPolygon : public Renderable
	{
	public:
		RenderPolygon()
			: rb_(new RenderBuffer(RenderBuffer::BT_TriangleList))
		{
			effect_ = LoadRenderEffect(L"PerPixelLighting.fx");
			effect_->SetTexture("diffusemap", LoadTGA(L"Diffuse.tga"));
			effect_->SetTexture("normalmap", LoadTGA(L"Normal.tga"));
			effect_->SetTexture("specularmap", LoadTGA(L"Specular.tga"));
			effect_->SetTechnique("PerPixelLighting");

			float xyzs[] =
			{
				-1, 1,  1,
				1,	1,	1,
				1,	-1,	1,
				-1, -1, 1,
			};

			float texs[] =
			{
				0, 0,
				1, 0,
				1, 1,
				0, 1
			};

			U16 indices[] = 
			{
				0, 1, 2, 2, 3, 0
			};

			rb_->AddVertexStream(VST_Positions, sizeof(float), 3);
			rb_->AddVertexStream(VST_TextureCoords0, sizeof(float), 2);
			rb_->AddVertexStream(VST_TextureCoords1, sizeof(float), 2);
			rb_->AddVertexStream(VST_TextureCoords2, sizeof(float), 2);
			rb_->GetVertexStream(VST_Positions)->Assign(xyzs, sizeof(xyzs) / sizeof(float) / 3);
			rb_->GetVertexStream(VST_TextureCoords0)->Assign(texs, sizeof(texs) / sizeof(float) / 2);
			rb_->GetVertexStream(VST_TextureCoords1)->Assign(texs, sizeof(texs) / sizeof(float) / 2);
			rb_->GetVertexStream(VST_TextureCoords2)->Assign(texs, sizeof(texs) / sizeof(float) / 2);

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

		const WString& Name() const
		{
			static WString name(L"Polygon");
			return name;
		}

		KlayGE::RenderBufferPtr rb_;
		KlayGE::RenderEffectPtr effect_;
	};

	SharedPtr<RenderPolygon> renderPolygon;
}


int main()
{
	PerPixelLighting app;

	Engine::RenderFactoryInstance(D3D9RenderFactoryInstance());

	D3D9RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.colorDepth = 32;
	settings.fullScreen = false;

	app.Create("PerPixelLighting", settings);
	app.Run();

	return 0;
}

PerPixelLighting::PerPixelLighting()
{
}

void PerPixelLighting::InitObjects()
{
	// 建立字体
	font_ = Engine::RenderFactoryInstance().MakeFont(L"宋体", 16);

	renderPolygon = SharedPtr<RenderPolygon>(new RenderPolygon);

	RenderEngine& renderEngine(Engine::RenderFactoryInstance().RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	this->LookAt(MakeVector(0.0f, 0.0f, -3.0f), MakeVector(0.0f, 0.0f, 0.0f));
	this->Proj(0.1f, 20.0f);
}

void PerPixelLighting::Update()
{
	RenderEngine& renderEngine(Engine::RenderFactoryInstance().RenderEngineInstance());
	MathLib& math(Engine::MathInstance());
	SceneManager& sceneMgr(Engine::SceneManagerInstance());

	static float degree(0);
	Vector3 lightPos(MakeVector(0.0f, 0.0f, 1.0f));
	Matrix4 matRot;
	math.RotationY(matRot, degree);
	math.TransformNormal(lightPos, lightPos, matRot);
	math.Normalize(lightPos, lightPos);
	renderPolygon->effect_->SetVector("lightPos",
		MakeVector(lightPos.x(), lightPos.y(), lightPos.z(), 1.0f));

	Vector3 halfway(lightPos + MakeVector(0.0f, 0.0f, 1.0f));
	math.Normalize(halfway, halfway);
	renderPolygon->effect_->SetVector("halfway",
		MakeVector(halfway.x(), halfway.y(), halfway.z(), 1.0f));

	degree += 0.01f;

	std::wostringstream stream;
	stream << (*renderEngine.ActiveRenderTarget())->FPS();

	sceneMgr.PushRenderable(renderPolygon);
	sceneMgr.PushRenderable(font_->RenderText(0, 0, Color(1, 1, 0, 1), L"像素光照测试"));
	sceneMgr.PushRenderable(font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str()));
}
