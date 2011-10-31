#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
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
#include <KlayGE/SceneObjectHelper.hpp>
#include <KlayGE/Window.hpp>

#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/InputFactory.hpp>

#include <vector>
#include <sstream>
#include <boost/bind.hpp>
#include <boost/typeof/typeof.hpp>

#include "JudaTexViewer.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	uint32_t const BORDER_SIZE = 4;

#pragma pack(push, 1)
	struct tile_instance
	{
		float2 pos;
		uint32_t tile_id;
	};
#pragma pack(pop)

	class RenderTile : public RenderableHelper
	{
	public:
		RenderTile()
			: RenderableHelper(L"Tile")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("JudaTexViewer.fxml")->TechniqueByName("Render");

			float2 texs[] =
			{
				float2(0, 0),
				float2(1, 0),
				float2(0, 1),
				float2(1, 1)
			};

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_TriangleStrip);

			ElementInitData init_data;
			init_data.row_pitch = sizeof(texs);
			init_data.slice_pitch = 0;
			init_data.data = texs;
			GraphicsBufferPtr tex_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
			rl_->BindVertexStream(tex_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_GR32F)));
		}

		void SetPosBuffer(GraphicsBufferPtr const & pos_vb)
		{
			rl_->BindVertexStream(pos_vb,
				boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_GR32F), vertex_element(VEU_Diffuse, 0, EF_ABGR8)),
				RenderLayout::ST_Instance);
		}

		void SetModel(float4x4 const & model)
		{
			model_ = model;
		}

		void OnRenderBegin()
		{
			FrameBufferPtr const & fb = Context::Instance().RenderFactoryInstance().RenderEngineInstance().CurFrameBuffer();
			*(technique_->Effect().ParameterByName("world_mat")) = model_ * MathLib::scaling(2.0f / fb->Width(), 2.0f / fb->Height(), 1.0f);
		}

	private:
		float4x4 model_;
	};

	class TileObject : public SceneObjectHelper
	{
	public:
		TileObject()
			: SceneObjectHelper(0),
				position_(0.0f, 0.0f), scale_(1.0f)
		{
			renderable_.reset(new RenderTile());
		}

		void TileSize(uint32_t tile_size)
		{
			mat_tile_scaling_ = MathLib::scaling(1.0f, -1.0f, 1.0f)
				* MathLib::scaling(static_cast<float>(tile_size), static_cast<float>(tile_size), 1.0f);
		}

		void SetPosBuffer(GraphicsBufferPtr const & pos_vb)
		{
			checked_pointer_cast<RenderTile>(renderable_)->SetPosBuffer(pos_vb);
		}

		void Position(float2 const & pos)
		{
			position_ = pos;
			mat_translation_ = MathLib::translation(-position_.x(), -position_.y(), 0.0f);
			checked_pointer_cast<RenderTile>(renderable_)->SetModel(mat_tile_scaling_ * mat_translation_ * mat_scaling_);
		}

		void Scale(float scale)
		{
			scale_ = scale;
			mat_scaling_ = MathLib::scaling(scale_, scale_, 1.0f);
			checked_pointer_cast<RenderTile>(renderable_)->SetModel(mat_tile_scaling_ * mat_translation_ * mat_scaling_);
		}

	private:
		float2 position_;
		float scale_;
		float4x4 mat_tile_scaling_;
		float4x4 mat_translation_;
		float4x4 mat_scaling_;
	};

	
	class RenderGridBorder : public RenderableHelper
	{
	public:
		RenderGridBorder()
			: RenderableHelper(L"Grid")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			technique_ = rf.LoadEffect("JudaTexViewer.fxml")->TechniqueByName("GridBorder");

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

			rl_ = rf.MakeRenderLayout();
			rl_->TopologyType(RenderLayout::TT_LineStrip);

			ElementInitData init_data;
			init_data.row_pitch = sizeof(texs);
			init_data.slice_pitch = 0;
			init_data.data = texs;
			GraphicsBufferPtr tex_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
			rl_->BindVertexStream(tex_vb, boost::make_tuple(vertex_element(VEU_Position, 0, EF_GR32F)));

			init_data.row_pitch = sizeof(indices);
			init_data.slice_pitch = 0;
			init_data.data = indices;
			GraphicsBufferPtr ib = rf.MakeIndexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, &init_data);
			rl_->BindIndexStream(ib, EF_R16UI);
		}

		void SetPosBuffer(GraphicsBufferPtr const & pos_vb)
		{
			rl_->BindVertexStream(pos_vb,
				boost::make_tuple(vertex_element(VEU_TextureCoord, 0, EF_GR32F), vertex_element(VEU_Diffuse, 0, EF_ABGR8)),
				RenderLayout::ST_Instance);
		}

		void SetModel(float4x4 const & model)
		{
			model_ = model;
		}

		void OnRenderBegin()
		{
			FrameBufferPtr const & fb = Context::Instance().RenderFactoryInstance().RenderEngineInstance().CurFrameBuffer();
			*(technique_->Effect().ParameterByName("world_mat")) = model_ * MathLib::scaling(2.0f / fb->Width(), 2.0f / fb->Height(), 1.0f);
		}

	private:
		float4x4 model_;
	};

	class GridBorderObject : public SceneObjectHelper
	{
	public:
		GridBorderObject()
			: SceneObjectHelper(0),
				position_(0.0f, 0.0f), scale_(1.0f)
		{
			renderable_.reset(new RenderGridBorder());
		}

		void TileSize(uint32_t tile_size)
		{
			mat_tile_scaling_ = MathLib::scaling(1.0f, -1.0f, 1.0f)
				* MathLib::scaling(static_cast<float>(tile_size), static_cast<float>(tile_size), 1.0f);
		}

		void SetPosBuffer(GraphicsBufferPtr const & pos_vb)
		{
			checked_pointer_cast<RenderGridBorder>(renderable_)->SetPosBuffer(pos_vb);
		}

		void Position(float2 const & pos)
		{
			position_ = pos;
			mat_translation_ = MathLib::translation(-position_.x(), -position_.y(), 0.0f);
			checked_pointer_cast<RenderGridBorder>(renderable_)->SetModel(mat_tile_scaling_ * mat_translation_ * mat_scaling_);
		}

		void Scale(float scale)
		{
			scale_ = scale;
			mat_scaling_ = MathLib::scaling(scale_, scale_, 1.0f);
			checked_pointer_cast<RenderGridBorder>(renderable_)->SetModel(mat_tile_scaling_ * mat_translation_ * mat_scaling_);
		}

	private:
		float2 position_;
		float scale_;
		float4x4 mat_tile_scaling_;
		float4x4 mat_translation_;
		float4x4 mat_scaling_;
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


int main()
{
	ResLoader::Instance().AddPath("../../Samples/media/Common");

	Context::Instance().LoadCfg("KlayGE.cfg");

	JudaTexViewer app;
	app.Create();
	app.Run();

	return 0;
}

JudaTexViewer::JudaTexViewer()
			: App3DFramework("JudaTexViewer"),
				sx_(0), sy_(0), ex_(0), ey_(0),
				last_mouse_x_(-1), last_mouse_y_(-1),
				position_(0, 0), scale_(1)
{
	ResLoader::Instance().AddPath("../../Samples/media/JudaTexViewer");
}

bool JudaTexViewer::ConfirmDevice()
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	RenderDeviceCaps const & caps = re.DeviceCaps();
	if (caps.max_shader_model < 2)
	{
		return false;
	}
	return true;
}

void JudaTexViewer::InitObjects()
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	font_ = rf.MakeFont("gkai00mp.kfont");

	tile_pos_vb_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_CPU_Write, NULL);

	tile_ = MakeSharedPtr<TileObject>();
	tile_->AddToSceneManager();
	checked_pointer_cast<TileObject>(tile_)->SetPosBuffer(tile_pos_vb_);

	grid_border_ = MakeSharedPtr<GridBorderObject>();
	grid_border_->AddToSceneManager();
	checked_pointer_cast<GridBorderObject>(grid_border_)->SetPosBuffer(tile_pos_vb_);

	this->OpenJudaTex("klayge_logo.jdt");

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));

	action_handler_t input_handler = MakeSharedPtr<input_signal>();
	input_handler->connect(boost::bind(&JudaTexViewer::InputHandler, this, _1, _2));
	inputEngine.ActionMap(actionMap, input_handler, true);

	UIManager::Instance().Load(ResLoader::Instance().Open("JudaTexViewer.uiml"));
	dialog_ = UIManager::Instance().GetDialogs()[0];

	id_open_ = dialog_->IDFromName("Open");
	dialog_->Control<UIButton>(id_open_)->OnClickedEvent().connect(boost::bind(&JudaTexViewer::OpenHandler, this, _1));
}

void JudaTexViewer::OnResize(uint32_t width, uint32_t height)
{
	App3DFramework::OnResize(width, height);

	UIManager::Instance().SettleCtrls(width, height);
}

void JudaTexViewer::InputHandler(InputEngine const & sender, InputAction const & action)
{
	InputMousePtr mouse;
	for (uint32_t i = 0; i < sender.NumDevices(); ++ i)
	{
		InputMousePtr m = boost::dynamic_pointer_cast<InputMouse>(sender.Device(i));
		if (m)
		{
			mouse = m;
			break;
		}
	}

	switch (action.first)
	{
	case Exit:
		this->Quit();
		break;

	case Move:
		if (mouse)
		{
			int32_t x = mouse->AbsX();
			int32_t y = mouse->AbsY();
			if (mouse->LeftButton())
			{
				if ((last_mouse_x_ != -1) || (last_mouse_y_ != -1))
				{
					position_.x() += (last_mouse_x_ - x) / scale_;
					position_.y() += (y - last_mouse_y_) / scale_;
				}

				checked_pointer_cast<TileObject>(tile_)->Position(position_);
				checked_pointer_cast<GridBorderObject>(grid_border_)->Position(position_);
			}

			last_mouse_x_ = x;
			last_mouse_y_ = y;
		}
		break;

	case Scale:
		if (mouse)
		{
			float f = 1.0f + (mouse->Z() * 0.1f) / 120;
			float2 p = float2(static_cast<float>(mouse->AbsX()), static_cast<float>(-mouse->AbsY())) / scale_ + position_;
			float2 new_position = (position_ - p * (1 - f)) / f;
			float new_scale = scale_ * f;
			if (tile_size_ * new_scale > 64)
			{
				position_ = new_position;
				scale_ = new_scale;

				checked_pointer_cast<TileObject>(tile_)->Position(position_);
				checked_pointer_cast<TileObject>(tile_)->Scale(scale_);

				checked_pointer_cast<GridBorderObject>(grid_border_)->Position(position_);
				checked_pointer_cast<GridBorderObject>(grid_border_)->Scale(scale_);
			}
		}
		break;
	}
}

void JudaTexViewer::OpenJudaTex(std::string const & name)
{
	RenderFactory& rf = Context::Instance().RenderFactoryInstance();

	juda_tex_ = LoadJudaTexture(name);

	ElementFormat fmt;
	if (rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_BC1))
	{
		fmt = EF_BC1;
	}
	else if (rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ABGR8))
	{
		fmt = EF_ABGR8;
	}
	else
	{
		BOOST_ASSERT(rf.RenderEngineInstance().DeviceCaps().texture_format_support(EF_ARGB8));

		fmt = EF_ARGB8;
	}
	juda_tex_->CacheProperty(1024, fmt, BORDER_SIZE);

	num_tiles_ = juda_tex_->NumTiles();
	tile_size_ = juda_tex_->TileSize();

	position_ = float2(0, 0);
	scale_ = 1;

	juda_tex_->SetParams(tile_->GetRenderable()->GetRenderTechnique());

	checked_pointer_cast<TileObject>(tile_)->TileSize(tile_size_);
	checked_pointer_cast<TileObject>(tile_)->Position(position_);
	checked_pointer_cast<TileObject>(tile_)->Scale(scale_);

	checked_pointer_cast<GridBorderObject>(grid_border_)->TileSize(tile_size_);
	checked_pointer_cast<GridBorderObject>(grid_border_)->Position(position_);
	checked_pointer_cast<GridBorderObject>(grid_border_)->Scale(scale_);
}

void JudaTexViewer::OpenHandler(KlayGE::UIButton const & /*sender*/)
{
#if defined KLAYGE_PLATFORM_WINDOWS
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
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameA(&ofn))
	{
		HCURSOR cur = GetCursor();
		SetCursor(LoadCursor(NULL, IDC_WAIT));
		
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

	for (uint32_t y = sy_; y < ey_; ++ y)
	{
		for (uint32_t x = sx_; x < ex_; ++ x)
		{
			stream.str(L"");
			stream << x << ',' << y;
			std::wstring str = stream.str();
			Size_T<uint32_t> s = font_->CalcSize(str, 16);
			float fx = (-position_.x() + (x + 0.5f) * tile_size_) * scale_;
			float fy = (+position_.y() + (y + 0.5f) * tile_size_) * scale_;
			font_->RenderText(fx - s.cx() / 2.0f, fy - s.cy() / 2.0f, 0.7f, 1.0f, 1.0f, Color(1, 0, 1, 0.5f), str, 16);
		}
	}
}

uint32_t JudaTexViewer::DoUpdate(uint32_t /*pass*/)
{
	RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
	
	uint32_t const level = juda_tex_->TreeLevels() - 1;
		
	sx_ = static_cast<uint32_t>(std::max(0, static_cast<int>(position_.x() / tile_size_)));
	sy_ = static_cast<uint32_t>(std::max(0, static_cast<int>(-position_.y() / tile_size_)));
	ex_ = std::min(num_tiles_, static_cast<uint32_t>(std::ceil((re.CurFrameBuffer()->Width() / scale_ + position_.x()) / tile_size_ + 1)));
	ey_ = std::min(num_tiles_, static_cast<uint32_t>(std::ceil((re.CurFrameBuffer()->Height() / scale_ - position_.y()) / tile_size_ + 1)));
	uint32_t nx = ex_ - sx_;
	uint32_t ny = ey_ - sy_;

	std::vector<uint32_t> tile_ids(nx * ny);
	tile_pos_vb_->Resize(sizeof(tile_instance) * nx * ny);
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

	RenderLayoutPtr const & rl_tile = tile_->GetRenderable()->GetRenderLayout();
	for (uint32_t i = 0; i < rl_tile->NumVertexStreams(); ++ i)
	{
		rl_tile->VertexStreamFrequencyDivider(i, RenderLayout::ST_Geometry, nx * ny);
	}

	RenderLayoutPtr const & rl_border = grid_border_->GetRenderable()->GetRenderLayout();
	for (uint32_t i = 0; i < rl_border->NumVertexStreams(); ++ i)
	{
		rl_border->VertexStreamFrequencyDivider(i, RenderLayout::ST_Geometry, nx * ny);
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
	return App3DFramework::URV_Need_Flush | App3DFramework::URV_Skip_Postprocess | App3DFramework::URV_Finished;
}
