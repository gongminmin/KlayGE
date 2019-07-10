#include <KlayGE/KlayGE.hpp>
#include <KFL/CXX17/iterator.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/Window.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>

#include "SampleCommon.hpp"
#include "JudaTexViewer.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	uint32_t const BORDER_SIZE = 4;

#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(push, 1)
#endif
	struct tile_instance
	{
		float2 pos;
		uint32_t tile_id;
	};
#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(pop)
#endif

	class RenderTile : public Renderable
	{
	public:
		RenderTile()
			: Renderable(L"Tile")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = SyncLoadRenderEffect("JudaTexViewer.fxml");
			technique_ = effect_->TechniqueByName("Render");

			float2 texs[] =
			{
				float2(0, 0),
				float2(1, 0),
				float2(0, 1),
				float2(1, 1)
			};

			uint16_t indices[] = 
			{
				0, 1, 2, 3
			};

			rls_[0] = rf.MakeRenderLayout();
			rls_[0]->TopologyType(RenderLayout::TT_TriangleStrip);

			GraphicsBufferPtr tex_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(texs), texs);
			rls_[0]->BindVertexStream(tex_vb, VertexElement(VEU_Position, 0, EF_GR32F));

			GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(indices), indices);
			rls_[0]->BindIndexStream(ib, EF_R16UI);
		}

		void SetPosBuffer(GraphicsBufferPtr const & pos_vb)
		{
			rls_[0]->BindVertexStream(pos_vb,
				MakeSpan({VertexElement(VEU_TextureCoord, 0, EF_GR32F), VertexElement(VEU_Diffuse, 0, EF_ABGR8)}),
				RenderLayout::ST_Instance);
		}

		void OnRenderBegin()
		{
			FrameBufferPtr const & fb = Context::Instance().RenderFactoryInstance().RenderEngineInstance().CurFrameBuffer();
			*(effect_->ParameterByName("world_mat")) = model_mat_ * MathLib::scaling(2.0f / fb->Width(), 2.0f / fb->Height(), 1.0f);
		}
	};
	
	class RenderGridBorder : public Renderable
	{
	public:
		RenderGridBorder()
			: Renderable(L"Grid")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = SyncLoadRenderEffect("JudaTexViewer.fxml");
			technique_ = effect_->TechniqueByName("GridBorder");

			float2 texs[] =
			{
				float2(0, 0),
				float2(1, 0),
				float2(1, 1),
				float2(0, 1)
			};

			uint16_t indices[] =
			{
				0, 1, 2, 3, 0
			};

			rls_[0] = rf.MakeRenderLayout();
			rls_[0]->TopologyType(RenderLayout::TT_LineStrip);

			GraphicsBufferPtr tex_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(texs), texs);
			rls_[0]->BindVertexStream(tex_vb, VertexElement(VEU_Position, 0, EF_GR32F));

			GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(indices), indices);
			rls_[0]->BindIndexStream(ib, EF_R16UI);
		}

		void SetPosBuffer(GraphicsBufferPtr const & pos_vb)
		{
			rls_[0]->BindVertexStream(pos_vb,
				MakeSpan({VertexElement(VEU_TextureCoord, 0, EF_GR32F), VertexElement(VEU_Diffuse, 0, EF_ABGR8)}),
				RenderLayout::ST_Instance);
		}

		void OnRenderBegin()
		{
			FrameBufferPtr const & fb = Context::Instance().RenderFactoryInstance().RenderEngineInstance().CurFrameBuffer();
			*(effect_->ParameterByName("world_mat")) = model_mat_ * MathLib::scaling(2.0f / fb->Width(), 2.0f / fb->Height(), 1.0f);
		}
	};


	enum
	{
		Exit,
		Move,
		Scale
	};

	InputActionDefine actions[] =
	{
		InputActionDefine(Exit, KS_Escape),

		InputActionDefine(Move, MS_X),
		InputActionDefine(Move, MS_Y),
		InputActionDefine(Scale, MS_Z)
	};
}


int SampleMain()
{
	ContextCfg cfg = Context::Instance().Config();
	cfg.graphics_cfg.hdr = false;
	cfg.graphics_cfg.gamma = false;
	cfg.graphics_cfg.ppaa = false;
	cfg.graphics_cfg.color_grading = false;
	Context::Instance().Config(cfg);

	JudaTexViewer app;
	app.Create();
	app.Run();

	return 0;
}

JudaTexViewer::JudaTexViewer()
			: App3DFramework("JudaTexViewer"),
				sx_(0), sy_(0), ex_(0), ey_(0),
				last_mouse_pt_(-1, -1), position_(0, 0), scale_(1)
{
	ResLoader::Instance().AddPath("../../Samples/media/JudaTexViewer");
}

void JudaTexViewer::OnCreate()
{
	font_ = SyncLoadFont("gkai00mp.kfont");

	tile_renderable_ = MakeSharedPtr<RenderTile>();
	grid_border_renderable_ = MakeSharedPtr<RenderGridBorder>();

	node_ = MakeSharedPtr<SceneNode>(0);
	node_->AddComponent(MakeSharedPtr<RenderableComponent>(tile_renderable_));
	node_->AddComponent(MakeSharedPtr<RenderableComponent>(grid_border_renderable_));
	Context::Instance().SceneManagerInstance().SceneRootNode().AddChild(node_);

	this->OpenJudaTex("klayge_logo.jdt");

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

	UIManager::Instance().Load(ResLoader::Instance().Open("JudaTexViewer.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_open_ = dialog_->IDFromName("Open");
	dialog_->Control<UIButton>(id_open_)->OnClickedEvent().Connect(
		[this](UIButton const & sender)
		{
			this->OpenHandler(sender);
		});
}

void JudaTexViewer::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls();
}

void JudaTexViewer::InputHandler(InputEngine const & /*sender*/, InputAction const & action)
{
	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;

	case Move:
		{
			InputMouseActionParamPtr param = checked_pointer_cast<InputMouseActionParam>(action.second);
			int2 this_mouse_pt(param->abs_coord);
			if (param->buttons_state & MB_Left)
			{
				if ((last_mouse_pt_.x() != -1) || (last_mouse_pt_.y() != -1))
				{
					position_ += float2(this_mouse_pt - last_mouse_pt_) / scale_;

					mat_translation_ = MathLib::translation(+position_.x(), -position_.y(), 0.0f);

					float4x4 const mat = mat_tile_scaling_ * mat_translation_ * mat_scaling_;
					checked_cast<RenderTile&>(*tile_renderable_).ModelMatrix(mat);
					checked_cast<RenderGridBorder&>(*grid_border_renderable_).ModelMatrix(mat);
				}
			}

			last_mouse_pt_ = this_mouse_pt;
		}
		break;

	case Scale:
		{
			InputMouseActionParamPtr param = checked_pointer_cast<InputMouseActionParam>(action.second);
			float f = 1.0f + MathLib::clamp(param->wheel_delta / 1200.0f, -0.5f, 0.5f);
			float2 p = float2(-param->abs_coord) / scale_ + position_;
			float2 new_position = (position_ - p * (1 - f)) / f;
			float new_scale = scale_ * f;
			if (tile_size_ * new_scale > 32)
			{
				position_ = new_position;
				scale_ = new_scale;

				mat_translation_ = MathLib::translation(+position_.x(), -position_.y(), 0.0f);
				mat_scaling_ = MathLib::scaling(scale_, scale_, 1.0f);

				float4x4 const mat = mat_tile_scaling_ * mat_translation_ * mat_scaling_;
				checked_cast<RenderTile&>(*tile_renderable_).ModelMatrix(mat);
				checked_cast<RenderGridBorder&>(*grid_border_renderable_).ModelMatrix(mat);
			}
		}
		break;
	}
}

void JudaTexViewer::OpenJudaTex(std::string const & name)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	juda_tex_ = LoadJudaTexture(name);

	auto const fmt = rf.RenderEngineInstance().DeviceCaps().BestMatchTextureFormat(MakeSpan({EF_BC1, EF_ABGR8, EF_ARGB8}));
	BOOST_ASSERT(fmt != EF_Unknown);
	juda_tex_->CacheProperty(1024, fmt, BORDER_SIZE);

	num_tiles_ = juda_tex_->NumTiles();
	tile_size_ = juda_tex_->TileSize();

	position_ = float2(0, 0);
	scale_ = 1;

	juda_tex_->SetParams(*tile_renderable_->GetRenderEffect());

	mat_tile_scaling_ = MathLib::scaling(1.0f, -1.0f, 1.0f)
		* MathLib::scaling(static_cast<float>(tile_size_), static_cast<float>(tile_size_), 1.0f);
	mat_translation_ = MathLib::translation(+position_.x(), -position_.y(), 0.0f);
	mat_scaling_ = MathLib::scaling(scale_, scale_, 1.0f);

	float4x4 const mat = mat_tile_scaling_ * mat_translation_ * mat_scaling_;
	checked_cast<RenderTile&>(*tile_renderable_).ModelMatrix(mat);
	checked_cast<RenderGridBorder&>(*grid_border_renderable_).ModelMatrix(mat);
}

void JudaTexViewer::OpenHandler(KlayGE::UIButton const & /*sender*/)
{
#if defined KLAYGE_PLATFORM_WINDOWS_DESKTOP
	OPENFILENAMEA ofn;
	char fn[260];
	HWND hwnd = this->MainWnd()->HWnd();

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = fn;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(fn);
	ofn.lpstrFilter = "JDT File\0*.jdt\0All\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileNameA(&ofn))
	{
		HCURSOR cur = GetCursor();
		SetCursor(LoadCursor(nullptr, IDC_WAIT));

		if (last_file_path_.empty())
		{
			ResLoader::Instance().DelPath(last_file_path_);
		}

		std::string file_name = fn;
		last_file_path_ = file_name.substr(0, file_name.find_last_of('\\'));
		ResLoader::Instance().AddPath(last_file_path_);
		
		this->OpenJudaTex(fn);

		SetCursor(cur);
	}
#endif
}

void JudaTexViewer::DoUpdateOverlay()
{
	UIManager::Instance().Render();

	std::wostringstream stream;
	stream.precision(2);
	stream << std::fixed << this->FPS() << " FPS";

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"Juda Texture Viewer", 16);
	font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);

	if (tile_size_ * scale_ > 64)
	{
		for (uint32_t y = sy_; y < ey_; ++ y)
		{
			for (uint32_t x = sx_; x < ex_; ++ x)
			{
				stream.str(L"");
				stream << x << ',' << y;
				std::wstring str = stream.str();
				Size_T<uint32_t> s = font_->CalcSize(str, 16);
				float fx = (position_.x() + (x + 0.5f) * tile_size_) * scale_;
				float fy = (position_.y() + (y + 0.5f) * tile_size_) * scale_;
				font_->RenderText(fx - s.cx() / 2.0f, fy - s.cy() / 2.0f, 0.7f, 1.0f, 1.0f, Color(1, 0, 1, 0.5f), str, 16);
			}
		}
	}
}

uint32_t JudaTexViewer::DoUpdate(uint32_t /*pass*/)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();
	RenderEngine& re = rf.RenderEngineInstance();
	
	uint32_t const level = juda_tex_->TreeLevels() - 1;
		
	sx_ = static_cast<uint32_t>(std::max(0, static_cast<int>(-position_.x() / tile_size_)));
	sy_ = static_cast<uint32_t>(std::max(0, static_cast<int>(-position_.y() / tile_size_)));
	ex_ = std::min(num_tiles_, static_cast<uint32_t>(std::ceil((re.CurFrameBuffer()->Width() / scale_ - position_.x()) / tile_size_ + 1)));
	ey_ = std::min(num_tiles_, static_cast<uint32_t>(std::ceil((re.CurFrameBuffer()->Height() / scale_ - position_.y()) / tile_size_ + 1)));
	uint32_t nx = ex_ - sx_;
	uint32_t ny = ey_ - sy_;

	std::vector<uint32_t> tile_ids(nx * ny);
	uint32_t const new_tile_pos_size = sizeof(tile_instance) * nx * ny;
	if (!tile_pos_vb_ || (tile_pos_vb_->Size() < new_tile_pos_size))
	{
		tile_pos_vb_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_CPU_Write, new_tile_pos_size, nullptr);
	}
	{
		GraphicsBuffer::Mapper mapper(*tile_pos_vb_, BA_Write_Only);
		tile_instance* instance_data = mapper.Pointer<tile_instance>();
		for (uint32_t y = 0; y < ny; ++ y)
		{
			for (uint32_t x = 0; x < nx; ++ x)
			{
				instance_data[y * nx + x].pos.x() = static_cast<float>(sx_ + x);
				instance_data[y * nx + x].pos.y() = static_cast<float>(sy_ + y);
				instance_data[y * nx + x].tile_id = juda_tex_->EncodeTileID(level, sx_ + x, sy_ + y);
				tile_ids[y * nx + x] = instance_data[y * nx + x].tile_id;
			}
		}
	}

	checked_cast<RenderTile&>(*tile_renderable_).SetPosBuffer(tile_pos_vb_);
	checked_cast<RenderGridBorder&>(*grid_border_renderable_).SetPosBuffer(tile_pos_vb_);

	RenderLayout& rl_tile = tile_renderable_->GetRenderLayout();
	for (uint32_t i = 0; i < rl_tile.NumVertexStreams(); ++ i)
	{
		rl_tile.VertexStreamFrequencyDivider(i, RenderLayout::ST_Geometry, nx * ny);
	}

	RenderLayout& rl_border = checked_cast<RenderGridBorder&>(*grid_border_renderable_).GetRenderLayout();
	for (uint32_t i = 0; i < rl_border.NumVertexStreams(); ++ i)
	{
		rl_border.VertexStreamFrequencyDivider(i, RenderLayout::ST_Geometry, nx * ny);
	}

	juda_tex_->UpdateCache(tile_ids);

	Color clear_clr(0.2f, 0.4f, 0.6f, 1);
	if (Context::Instance().Config().graphics_cfg.gamma)
	{
		clear_clr.r() = 0.029f;
		clear_clr.g() = 0.133f;
		clear_clr.b() = 0.325f;
	}
	re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);
	return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
}
