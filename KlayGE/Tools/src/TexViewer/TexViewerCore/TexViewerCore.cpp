#include <KlayGE/KlayGE.hpp>
#include <KFL/ErrorHandling.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/Camera.hpp>
#include <KlayGE/SceneNode.hpp>
#include <KlayGE/DeferredRenderingLayer.hpp>
#include <KlayGE/UI.hpp>
#include <KlayGE/Mesh.hpp>
#include <KlayGE/Window.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Input.hpp>
#include <KlayGE/InputFactory.hpp>

#include <sstream>
#include <fstream>

#include "TexViewerCore.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	class RenderQuad : public Renderable
	{
	public:
		RenderQuad()
			: Renderable(L"Quad")
		{
			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			effect_ = SyncLoadRenderEffect("TexViewer.fxml");
			technique_ = effect_->TechniqueByName("TexViewerTech");
			texture_2d_param_ = effect_->ParameterByName("texture_2d");
			zoom_param_ = effect_->ParameterByName("zoom");
			rgb_scale_param_ = effect_->ParameterByName("rgb_scale");
			color_mask_param_ = effect_->ParameterByName("color_mask");
			width_height_param_ = effect_->ParameterByName("width_height");

			float2 xyzs[] =
			{
				float2(0, 0),
				float2(1, 0),
				float2(0, 1),
				float2(1, 1),
			};

			rls_[0] = rf.MakeRenderLayout();
			rls_[0]->TopologyType(RenderLayout::TT_TriangleStrip);

			GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read | EAH_Immutable, sizeof(xyzs), xyzs);

			rls_[0]->BindVertexStream(pos_vb, VertexElement(VEU_Position, 0, EF_GR32F));

			pos_aabb_ = AABBox(float3(-1, -1, -1), float3(1, 1, 1));
			tc_aabb_ = AABBox(float3(0, 0, 0), float3(1, 1, 0));
		}

		void OnRenderBegin()
		{
			Renderable::OnRenderBegin();

			auto const& pmccb = Context::Instance().RenderFactoryInstance().RenderEngineInstance().PredefinedModelCameraCBufferInstance();
			pmccb.Mvp(*model_camera_cbuffer_) = MathLib::transpose(model_mat_);
			model_camera_cbuffer_->Dirty(true);
		}

		void Texture(TexturePtr const & tex)
		{
			texture_ = tex;
			*texture_2d_param_ = tex;
			*width_height_param_ = int2(tex->Width(0), tex->Height(0));
		}

		void Zoom(float value)
		{
			*zoom_param_ = value;
		}

		void Stops(float value)
		{
			*rgb_scale_param_ = pow(2.0f, value);
		}

		void ColorMask(bool r, bool g, bool b, bool a)
		{
			*color_mask_param_ = float4(r ? 1.0f : 0.0f, g ? 1.0f : 0.0f, b ? 1.0f : 0.0f, a ? 1.0f : 0.0f);
		}

	private:
		TexturePtr texture_;

		RenderEffectParameter* texture_2d_param_;
		RenderEffectParameter* zoom_param_;
		RenderEffectParameter* rgb_scale_param_;
		RenderEffectParameter* color_mask_param_;
		RenderEffectParameter* width_height_param_;
	};
}

namespace KlayGE
{
	TexViewerCore::TexViewerCore(void* native_wnd)
				: App3DFramework("TexViewer", native_wnd),
					active_array_index_(0), active_face_(0), active_depth_index_(0), active_mipmap_level_(0),
					offset_x_(0), offset_y_(0),
					zoom_(1), stops_(0)
	{
		ResLoader::Instance().AddPath("../../Tools/media/TexViewer");
	}

	void TexViewerCore::Resize(uint32_t width, uint32_t height)
	{
		Context::Instance().RenderFactoryInstance().RenderEngineInstance().Resize(width, height);
	}

	void TexViewerCore::OnCreate()
	{
		font_ = SyncLoadFont("gkai00mp.kfont");

		quad_ = MakeSharedPtr<RenderQuad>();
		quad_so_ = MakeSharedPtr<SceneNode>(MakeSharedPtr<RenderableComponent>(quad_),
			SceneNode::SOA_Cullable | SceneNode::SOA_Moveable | SceneNode::SOA_NotCastShadow);
		quad_so_->Visible(false);
		Context::Instance().SceneManagerInstance().SceneRootNode().AddChild(quad_so_);

		this->LookAt(float3(-5, 5, -5), float3(0, 1, 0), float3(0.0f, 1.0f, 0.0f));
		this->Proj(0.1f, 100);
	}

	void TexViewerCore::OnDestroy()
	{
		font_.reset();
	}

	void TexViewerCore::OnResize(uint32_t width, uint32_t height)
	{
		App3DFramework::OnResize(width, height);
	}

	void TexViewerCore::OpenTexture(std::string const & name)
	{
		if (!last_file_path_.empty())
		{
			ResLoader::Instance().DelPath(last_file_path_);
		}

		std::string file_name = name;
		last_file_path_ = file_name.substr(0, file_name.find_last_of('\\'));
		ResLoader::Instance().AddPath(last_file_path_);

		texture_original_ = SyncLoadTexture(name, EAH_CPU_Read);
		texels_.clear();

		quad_so_->Visible(true);

		active_array_index_ = 0;
		active_face_ = 0;
		active_depth_index_ = 0;
		active_mipmap_level_ = 0;
		offset_x_ = 0;
		offset_y_ = 0;
		zoom_ = 1;
		stops_ = 0;

		this->UpdateDisplayTexture();

		this->UpdateQuadMatrix();
		this->ColorMask(true, true, true, true);
	}

	void TexViewerCore::SaveAsTexture(std::string const & name)
	{
		SaveTexture(texture_original_, name);
	}

	void TexViewerCore::DoUpdateOverlay()
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		std::wostringstream stream;
		stream.precision(2);
		stream << std::fixed << this->FPS() << " FPS";

		font_->RenderText(0, 0, Color(1, 1, 0, 1), re.ScreenFrameBuffer()->Description(), 16);
		font_->RenderText(0, 18, Color(1, 1, 0, 1), stream.str(), 16);
	}

	uint32_t TexViewerCore::DoUpdate(uint32_t pass)
	{
		KFL_UNUSED(pass);

		Color clear_clr(0.2f, 0.4f, 0.6f, 1);
		if (Context::Instance().Config().graphics_cfg.gamma)
		{
			clear_clr.r() = 0.029f;
			clear_clr.g() = 0.133f;
			clear_clr.b() = 0.325f;
		}

		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
		re.CurFrameBuffer()->Clear(FrameBuffer::CBM_Color | FrameBuffer::CBM_Depth, clear_clr, 1.0f, 0);

		return App3DFramework::URV_NeedFlush | App3DFramework::URV_Finished;
	}

	Texture::TextureType TexViewerCore::TextureType() const
	{
		return texture_original_->Type();
	}

	uint32_t TexViewerCore::ArraySize() const
	{
		return texture_original_->ArraySize();
	}

	uint32_t TexViewerCore::NumMipmaps() const
	{
		return texture_original_->NumMipMaps();
	}

	uint32_t TexViewerCore::Width(uint32_t mipmap) const
	{
		return texture_original_->Width(mipmap);
	}

	uint32_t TexViewerCore::Height(uint32_t mipmap) const
	{
		return texture_original_->Height(mipmap);
	}

	uint32_t TexViewerCore::Depth(uint32_t mipmap) const
	{
		return texture_original_->Depth(mipmap);
	}

	ElementFormat TexViewerCore::Format() const
	{
		return texture_original_->Format();
	}

	void TexViewerCore::ArrayIndex(uint32_t array_index)
	{
		texels_.clear();

		active_array_index_ = array_index;
		this->UpdateDisplayTexture();
	}

	void TexViewerCore::Face(uint32_t face)
	{
		texels_.clear();

		active_face_ = face;
		this->UpdateDisplayTexture();
	}

	void TexViewerCore::DepthIndex(uint32_t depth_index)
	{
		texels_.clear();

		active_depth_index_ = depth_index;
		this->UpdateDisplayTexture();
	}

	void TexViewerCore::MipmapLevel(uint32_t mipmap)
	{
		texels_.clear();

		active_mipmap_level_ = mipmap;

		this->UpdateDisplayTexture();
		this->UpdateQuadMatrix();
	}

	void TexViewerCore::Stops(float value)
	{
		stops_ = value;
		checked_pointer_cast<RenderQuad>(quad_)->Stops(value);
	}

	void TexViewerCore::OffsetAndZoom(float x, float y, float zoom)
	{
		offset_x_ = x;
		offset_y_ = y;
		zoom_ = zoom;

		this->UpdateQuadMatrix();
		checked_pointer_cast<RenderQuad>(quad_)->Zoom(zoom);
	}

	void TexViewerCore::ColorMask(bool r, bool g, bool b, bool a)
	{
		checked_pointer_cast<RenderQuad>(quad_)->ColorMask(r, g, b, a);
	}

	float4 TexViewerCore::RetrieveColor(uint32_t x, uint32_t y)
	{
		uint32_t const width = texture_display_->Width(0);
		uint32_t const height = texture_display_->Height(0);

		if ((x < width) && (y < height))
		{
			if (texels_.empty())
			{
				RenderFactory& rf = Context::Instance().RenderFactoryInstance();
				TexturePtr texture_cpu = rf.MakeTexture2D(width, height, 1, 1, EF_ABGR32F, 1, 0, EAH_CPU_Read);
				texture_display_->CopyToSubTexture2D(*texture_cpu, 0, 0, 0, 0, width, height,
					0, 0, 0, 0, width, height);

				texels_.resize(width * height);

				Texture::Mapper mapper(*texture_cpu, 0, 0, TMA_Read_Only, 0, 0, width, height);
				float4 const * p = mapper.Pointer<float4>();
				uint32_t pitch = mapper.RowPitch() / sizeof(float4);
				for (uint32_t i = 0; i < height; ++ i)
				{
					memcpy(&texels_[i * width], p + i * pitch, width * sizeof(float4));
				}
			}

			return texels_[y * width + x];
		}
		else
		{
			return float4(0, 0, 0, 0);
		}
	}

	void TexViewerCore::UpdateDisplayTexture()
	{
		uint32_t const width = texture_original_->Width(active_mipmap_level_);
		uint32_t const height = texture_original_->Height(active_mipmap_level_);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		texture_display_ = rf.MakeTexture2D(width, height, 1, 1, texture_original_->Format(), 1, 0, EAH_GPU_Read);
		switch (texture_original_->Type())
		{
		case Texture::TT_1D:
		case Texture::TT_2D:
			texture_original_->CopyToSubTexture2D(*texture_display_, 0, 0, 0, 0, width, height,
				active_array_index_, active_mipmap_level_, 0, 0, width, height);
			break;

		case Texture::TT_3D:
			{
				TexturePtr texture_3d_cpu = rf.MakeTexture3D(width, height, 1, 1, 1, texture_original_->Format(),
					1, 0, EAH_CPU_Read);
				texture_original_->CopyToSubTexture3D(*texture_3d_cpu, 0, 0, 0, 0, 0, width, height, 1,
					active_array_index_, active_mipmap_level_, 0, 0, active_depth_index_, width, height, 1);
				Texture::Mapper mapper_3d(*texture_3d_cpu, 0, 0, TMA_Read_Only, 0, 0, 0, width, height, 1);
				ElementInitData init_data;
				init_data.data = mapper_3d.Pointer<uint8_t>();
				init_data.row_pitch = mapper_3d.RowPitch();
				init_data.slice_pitch = mapper_3d.SlicePitch();
				TexturePtr texture_2d_cpu = rf.MakeTexture2D(width, height, 1, 1, texture_original_->Format(),
					1, 0, EAH_CPU_Write, MakeSpan<1>(init_data));
				texture_2d_cpu->CopyToSubTexture2D(*texture_display_, 0, 0, 0, 0, width, height,
					0, 0, 0, 0, width, height);
			}
			break;

		case Texture::TT_Cube:
			texture_original_->CopyToSubTexture2D(*texture_display_, 0, 0, 0, 0, width, height,
				active_array_index_ * 6 + active_face_, active_mipmap_level_, 0, 0, width, height);
			break;

		default:
			KFL_UNREACHABLE("Invalid texture type");
		}

		checked_pointer_cast<RenderQuad>(quad_)->Texture(texture_display_);
	}

	void TexViewerCore::UpdateQuadMatrix()
	{
		if (texture_original_)
		{
			uint32_t width = texture_original_->Width(active_mipmap_level_);
			uint32_t height = texture_original_->Height(active_mipmap_level_);

			RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();
			auto const & fb = re.CurFrameBuffer();
			uint32_t vp_width = fb->Width();
			uint32_t vp_height = fb->Height();

			float4x4 mat = MathLib::scaling(width * zoom_ / vp_width * 2.0f, height * zoom_ / vp_height * 2.0f, 1.0f)
				* MathLib::translation((offset_x_ * 2 - width) * zoom_ / vp_width, (offset_y_ * 2 - height) * zoom_ / vp_height, 0.0f);
			quad_so_->TransformToParent(mat);
		}
	}
}
