#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/VertexBuffer.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderTexture.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/KMesh.hpp>
#include <KlayGE/RenderSettings.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

#include <numeric>
#include <sstream>

#include "ascii_lums_builder.hpp"
#include "AsciiArts.hpp"

using namespace KlayGE;
using namespace std;

int const WIDTH = 800;
int const HEIGHT = 600;
int const CELL_WIDTH = 8;
int const CELL_HEIGHT = 8;
int const INPUT_NUM_ASCII = 128;
int const ASCII_WIDTH = 16;
int const ASCII_HEIGHT = 16;

int const OUTPUT_NUM_ASCII = 32;
int const LUM_LEVEL = 8;

namespace
{
	class RenderQuad : public RenderableHelper
	{
	public:
		RenderQuad(int width, int height)
		{
			std::vector<Vector3> pos;
			for (int y = 0; y < height + 1; ++ y)
			{
				for (int x = 0; x < width + 1; ++ x)
				{
					pos.push_back(Vector3(+x * 2.0f / width - 1,
						-y * 2.0f / height + 1, 1.0f));
				}
			}

			std::vector<Vector2> tex;
			for (int y = 0; y < height + 1; ++ y)
			{
				for (int x = 0; x < width + 1; ++ x)
				{
					tex.push_back(Vector2(static_cast<float>(x) / width, static_cast<float>(y) / height));
				}
			}

			std::vector<uint16_t> index;
			for (int y = 0; y < height; ++ y)
			{
				for (int x = 0; x < width; ++ x)
				{
					index.push_back((y + 0) * (width + 1) + (x + 0));
					index.push_back((y + 0) * (width + 1) + (x + 1));
					index.push_back((y + 1) * (width + 1) + (x + 1));

					index.push_back((y + 1) * (width + 1) + (x + 1));
					index.push_back((y + 1) * (width + 1) + (x + 0));
					index.push_back((y + 0) * (width + 1) + (x + 0));
				}
			}

			effect_ = LoadRenderEffect("AsciiArts.fx");
			effect_->SetTechnique("AsciiArts");

			vb_.reset(new VertexBuffer(VertexBuffer::BT_TriangleList));

			vb_->AddVertexStream(VST_Positions, sizeof(float), 3, true);
			vb_->AddVertexStream(VST_TextureCoords0, sizeof(float), 2, true);
			vb_->GetVertexStream(VST_Positions)->Assign(&pos[0], pos.size());
			vb_->GetVertexStream(VST_TextureCoords0)->Assign(&tex[0], tex.size());

			vb_->AddIndexStream(true);
			vb_->GetIndexStream()->Assign(&index[0], index.size());

			box_ = MathLib::ComputeBoundingBox<float>(pos.begin(), pos.end());
		}

		void SetTexture(TexturePtr const & scene_tex, TexturePtr const & lums_tex)
		{
			*(effect_->ParameterByName("scene_tex")) = scene_tex;
			*(effect_->ParameterByName("lums_tex")) = lums_tex;
		}

		void OnRenderBegin()
		{
			RenderEngine const & renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
			RenderTarget const & renderTarget(*(*renderEngine.ActiveRenderTarget()));

			*(effect_->ParameterByName("cell_per_row")) = static_cast<float>(CELL_WIDTH) / renderTarget.Width();
			*(effect_->ParameterByName("cell_per_line")) = static_cast<float>(CELL_HEIGHT) / renderTarget.Height();
		}

		const std::wstring& Name() const
		{
			static std::wstring name(L"Quad");
			return name;
		}
	};

	std::vector<ascii_tile_type> LoadFromTexture(std::string const & tex_name)
	{
		int const ASCII_IN_A_ROW = 16;

		KlayGE::TexturePtr ascii_tex = LoadTexture(tex_name);
		assert(PF_L8 == ascii_tex->Format());

		std::vector<ascii_tile_type> ret(INPUT_NUM_ASCII);

		std::vector<uint8_t> ascii_tex_data(INPUT_NUM_ASCII * ASCII_WIDTH * ASCII_HEIGHT);
		ascii_tex->CopyToMemory2D(0, &ascii_tex_data[0]);

		for (size_t i = 0; i < ret.size(); ++ i)
		{
			ret[i].resize(ASCII_WIDTH * ASCII_HEIGHT);
			for (int y = 0; y < ASCII_HEIGHT; ++ y)
			{
				for (int x = 0; x < ASCII_WIDTH; ++ x)
				{
					ret[i][y * ASCII_WIDTH + x]
						= ascii_tex_data[((i / ASCII_IN_A_ROW) * ASCII_HEIGHT + y) * ASCII_IN_A_ROW * ASCII_WIDTH
							+ (i % ASCII_IN_A_ROW) * ASCII_WIDTH + x];
				}
			}
		}

		return ret;
	}

	KlayGE::TexturePtr FillTexture(ascii_tiles_type const & ascii_lums)
	{
		assert(OUTPUT_NUM_ASCII == ascii_lums.size());

		std::vector<uint8_t> temp_data(OUTPUT_NUM_ASCII * ASCII_WIDTH * ASCII_HEIGHT);

		for (size_t i = 0; i < OUTPUT_NUM_ASCII; ++ i)
		{
			for (size_t y = 0; y < ASCII_HEIGHT; ++ y)
			{
				for (size_t x = 0; x < ASCII_WIDTH; ++ x)
				{
					temp_data[y * OUTPUT_NUM_ASCII * ASCII_WIDTH + i * ASCII_WIDTH + x]
						= ascii_lums[i][y * ASCII_WIDTH + x];
				}
			}
		}

		KlayGE::TexturePtr ret = Context::Instance().RenderFactoryInstance().MakeTexture2D(OUTPUT_NUM_ASCII * ASCII_WIDTH,
			ASCII_HEIGHT, 1, PF_L8);
		ret->CopyMemoryToTexture2D(0, &temp_data[0], PF_L8, OUTPUT_NUM_ASCII * ASCII_WIDTH, ASCII_HEIGHT, 0, 0);
		SaveToFile(ret, "lums.dds");
		return ret;
	}

	enum
	{
		Switch,
		Exit,
	};

	InputAction actions[] = 
	{
		InputAction(Switch, KS_Space),
		InputAction(Exit, KS_Escape),
	};

	struct TheRenderSettings : public RenderSettings
	{
		bool ConfirmDevice(RenderDeviceCaps const & caps) const
		{
			if (caps.max_shader_model < 2)
			{
				return false;
			}
			return true;
		}
	};
}

int main()
{
	OCTree sceneMgr(Box(Vector3(-10, -10, -10), Vector3(10, 10, 10)), 3);

	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
	Context::Instance().SceneManagerInstance(sceneMgr);

	Context::Instance().InputFactoryInstance(DInputFactoryInstance());

	TheRenderSettings settings;
	settings.width = WIDTH;
	settings.height = HEIGHT;
	settings.colorDepth = 32;
	settings.fullScreen = false;

	AsciiArts app;
	app.Create("ASCII Arts", settings);
	app.Run();

	return 0;
}

AsciiArts::AsciiArts()
			: show_ascii_(true)
{
	ResLoader::Instance().AddPath("../media/Common");
	ResLoader::Instance().AddPath("../media/AsciiArts");
}

void AsciiArts::BuildAsciiLumsTex()
{
	ascii_lums_builder builder(INPUT_NUM_ASCII, OUTPUT_NUM_ASCII, LUM_LEVEL, ASCII_WIDTH, ASCII_HEIGHT);
	ascii_lums_tex_ = FillTexture(builder.build(LoadFromTexture("font.dds")));
}

void AsciiArts::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.ttf", 16);

	this->LookAt(Vector3(0.0f, 0.3f, -0.2f), Vector3(0.0f, 0.1f, 0.0f));
	this->Proj(0.1f, 100.0f);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.005f, 0.05f);

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	mesh_ = LoadKMesh("bunny.kmesh");
	mesh_->AddToSceneManager();

	this->BuildAsciiLumsTex();

	rendered_tex_ = Context::Instance().RenderFactoryInstance().MakeTexture2D(WIDTH, HEIGHT, 1,
		PF_A8R8G8B8, Texture::TU_RenderTarget);
	render_buffer_ = Context::Instance().RenderFactoryInstance().MakeRenderTexture();
	render_buffer_->AttachTexture2D(rendered_tex_);

	downsample_tex_ = Context::Instance().RenderFactoryInstance().MakeTexture2D(WIDTH / CELL_WIDTH, HEIGHT / CELL_HEIGHT,
		1, PF_A8R8G8B8, Texture::TU_RenderTarget);

	screen_iter_ = renderEngine.ActiveRenderTarget();
	render_buffer_iter_ = renderEngine.AddRenderTarget(render_buffer_);

	renderQuad_.reset(new RenderQuad(1, 1));

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	KlayGE::InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));
	action_map_id_ = inputEngine.ActionMap(actionMap, true);
}

void AsciiArts::Update()
{
	fpcController_.Update();

	Camera& camera = this->ActiveCamera();

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionsType actions(inputEngine.Update(action_map_id_));
	for (InputActionsType::iterator iter = actions.begin(); iter != actions.end(); ++ iter)
	{
		switch (iter->first)
		{
		case Switch:
			show_ascii_ = !show_ascii_;
			KlayGE::Sleep(150);
			break;

		case Exit:
			this->Quit();
			break;
		}
	}

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());

	if (show_ascii_)
	{	
		// 第一遍，正常渲染
		renderEngine.ActiveRenderTarget(render_buffer_iter_);
		renderEngine.ViewMatrix(camera.ViewMatrix());
		renderEngine.ProjectionMatrix(camera.ProjMatrix());

		sceneMgr.Clear();
		mesh_->AddToSceneManager();
		sceneMgr.Flush();

		// 降采样
		rendered_tex_->CopyToTexture(*downsample_tex_);

		// 第二遍，匹配，最终渲染
		renderEngine.ActiveRenderTarget(screen_iter_);

		static_cast<RenderQuad*>(renderQuad_.get())->SetTexture(downsample_tex_, ascii_lums_tex_);
		sceneMgr.Clear();
		renderQuad_->AddToSceneManager();
	}
	else
	{
		renderEngine.ActiveRenderTarget(screen_iter_);
		renderEngine.ViewMatrix(camera.ViewMatrix());
		renderEngine.ProjectionMatrix(camera.ProjMatrix());

		sceneMgr.Clear();
		mesh_->AddToSceneManager();
	}

	std::wostringstream stream;
	stream << (*renderEngine.ActiveRenderTarget())->FPS();

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"ASCII艺术");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str().c_str());

	stream.str(L"");
	stream << sceneMgr.NumObjectsRendered() << " Renderables "
		<< sceneMgr.NumPrimitivesRendered() << " Primitives "
		<< sceneMgr.NumVerticesRendered() << " Vertices";
	font_->RenderText(0, 36, Color(1, 1, 1, 1), stream.str().c_str());
}
