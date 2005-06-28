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

#include <KlayGE/D3D9/D3D9RenderSettings.hpp>
#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

#include <numeric>
#include <sstream>
#include <iostream>
#include <map>

#include "AsciiArts.hpp"

using namespace KlayGE;
using namespace std;

int const WIDTH = 800;
int const HEIGHT = 600;
int const CELL_WIDTH = 8;
int const CELL_HEIGHT = 8;
int const NUM_ASCII = 128;
int const CHAR_WIDTH = 16;
int const CHAR_HEIGHT = 16;

namespace
{
	class RenderQuad : public Renderable
	{
	public:
		RenderQuad(int width, int height)
			: vb_(new VertexBuffer(VertexBuffer::BT_TriangleList))
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
			RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

			*(effect_->ParameterByName("cell_per_row")) = static_cast<float>(CELL_WIDTH) / (*renderEngine.ActiveRenderTarget())->Width();
			*(effect_->ParameterByName("cell_per_line")) = static_cast<float>(CELL_HEIGHT) / (*renderEngine.ActiveRenderTarget())->Height();
			*(effect_->ParameterByName("num_ascii")) = NUM_ASCII;
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

		const std::wstring& Name() const
		{
			static std::wstring name(L"Quad");
			return name;
		}

		Box box_;
		VertexBufferPtr vb_;
		RenderEffectPtr effect_;
	};

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


	bool cmp_diff_lum_to_iter(std::pair<float, std::multimap<float, unsigned char>::iterator> const & lhs,
							std::pair<float, std::multimap<float, unsigned char>::iterator> const & rhs)
	{
		return lhs.first > rhs.first;
	}

	void BuildAsciiLums(TexturePtr ascii_tex, TexturePtr ascii_lums_tex)
	{
		std::vector<float> lums(128, 0);
		std::vector<uint8_t> ascii_data(NUM_ASCII * CHAR_WIDTH * CHAR_HEIGHT);
		ascii_tex->CopyToMemory2D(0, &ascii_data[0]);
		for (int x = 0; x < NUM_ASCII; ++ x)
		{
			for (int yc = 0; yc < CHAR_HEIGHT; ++ yc)
			{
				for (int xc = 0; xc < CHAR_WIDTH; ++ xc)
				{
					lums[x] += ascii_data[yc * NUM_ASCII * CHAR_WIDTH + (x * CHAR_WIDTH + xc)] / 256.0f;
				}
			}
		}

		std::multimap<float, uint8_t> lum_to_char;
		float max_lun = *std::max_element(lums.begin(), lums.end());
		for (size_t i = 0; i < lums.size(); ++ i)
		{
			lum_to_char.insert(std::make_pair(lums[i] / max_lun * 32, static_cast<uint8_t>(i)));
		}
		std::vector<std::pair<float, std::multimap<float, uint8_t>::iterator> > diff_lum_to_iter;
		for (std::multimap<float, uint8_t>::iterator iter = lum_to_char.begin(); iter != lum_to_char.end(); ++ iter)
		{
			float diff_lum;

			if (iter != lum_to_char.begin())
			{
				std::multimap<float, uint8_t>::iterator prev_iter = iter;
				-- prev_iter;
				diff_lum = iter->first - prev_iter->first;
			}
			else
			{
				diff_lum = iter->first;
			}

			diff_lum_to_iter.push_back(std::make_pair(diff_lum, iter));
		}
		std::sort(diff_lum_to_iter.begin(), diff_lum_to_iter.end(), cmp_diff_lum_to_iter);
		while (diff_lum_to_iter.size() > 32)
		{
			diff_lum_to_iter.pop_back();
		}
		assert(32 == diff_lum_to_iter.size());
		std::map<float, uint8_t> final_lum_to_char;
		for (size_t i = 0; i < diff_lum_to_iter.size(); ++ i)
		{
			final_lum_to_char.insert(*diff_lum_to_iter[i].second);
		}
		assert(32 == final_lum_to_char.size());
		std::vector<uint8_t> lums_map(8 * 32 * CHAR_WIDTH * CHAR_HEIGHT);
		for (std::map<float, uint8_t>::iterator iter = final_lum_to_char.begin();
			iter != final_lum_to_char.end(); ++ iter)
		{
			int lum = std::distance(final_lum_to_char.begin(), iter);

			for (int y = 0; y < CHAR_HEIGHT; ++ y)
			{
				for (int x = 0; x < CHAR_WIDTH; ++ x)
				{
					lums_map[y * 8 * 32 * CHAR_WIDTH + (lum * CHAR_WIDTH + x)]
						= ascii_data[y * NUM_ASCII * CHAR_WIDTH + (iter->second * CHAR_WIDTH + x)];
				}
			}
		}
		for (int i = 7; i >= 0; -- i)
		{
			for (int y = 0; y < CHAR_HEIGHT; ++ y)
			{
				for (int x = 0; x < 32 * CHAR_WIDTH; ++ x)
				{
					lums_map[y * 8 * 32 * CHAR_WIDTH + (i * 32 * CHAR_WIDTH + x)]
						= lums_map[y * 8 * 32 * CHAR_WIDTH + x] / 8 * (i + 1);
				}
			}
		}

		ascii_lums_tex->CopyMemoryToTexture2D(0, &lums_map[0], PF_L8, 8 * 32 * CHAR_WIDTH, CHAR_HEIGHT, 0, 0);
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
	ResLoader::Instance().AddPath("../media");
	ResLoader::Instance().AddPath("../media/AsciiArts");
}

void AsciiArts::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gbsn00lp.ttf", 16);

	this->LookAt(Vector3(0.0f, 0.3f, -0.2f), Vector3(0.0f, 0.1f, 0.0f));
	this->Proj(0.1f, 100.0f);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.005f, 0.05f);

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	mesh_ = LoadKMesh("bunny.kmesh");
	mesh_->AddToSceneManager();

	KlayGE::TexturePtr ascii_tex = LoadTexture("font.dds");
	ascii_lums_tex_ = Context::Instance().RenderFactoryInstance().MakeTexture2D(8 * 32 * CHAR_WIDTH, CHAR_HEIGHT, 1, PF_L8);
	BuildAsciiLums(ascii_tex, ascii_lums_tex_);

	rendered_tex_ = Context::Instance().RenderFactoryInstance().MakeTexture2D(WIDTH, HEIGHT, 1, PF_A8R8G8B8, Texture::TU_RenderTarget);
	render_buffer_ = Context::Instance().RenderFactoryInstance().MakeRenderTexture();
	render_buffer_->AttachTexture2D(rendered_tex_);

	downsample_tex_ = Context::Instance().RenderFactoryInstance().MakeTexture2D(WIDTH / CELL_WIDTH, HEIGHT / CELL_HEIGHT, 1, PF_A8R8G8B8, Texture::TU_RenderTarget);

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
}
