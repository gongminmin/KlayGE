/**
 * @file Blitter.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <KlayGE/Blitter.hpp>

namespace KlayGE
{
	Blitter::Blitter()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		frame_buffer_ = rf.MakeFrameBuffer();
		quad_rl_ = rf.RenderEngineInstance().PostProcessRenderLayout();
		tex_to_buff_rl_ = rf.MakeRenderLayout();
		tex_to_buff_rl_->TopologyType(RenderLayout::TT_PointList);
		tex_to_buff_so_rl_ = rf.MakeRenderLayout();
		tex_to_buff_so_rl_->TopologyType(RenderLayout::TT_PointList);
		buff_to_tex_rl_ = rf.MakeRenderLayout();
		buff_to_tex_rl_->TopologyType(RenderLayout::TT_PointList);

		effect_ = SyncLoadRenderEffect("Blitter.fxml");
		blit_point_2d_tech_ = effect_->TechniqueByName("BlitPoint2D");
		blit_point_2d_array_tech_ = effect_->TechniqueByName("BlitPoint2DArray");
		blit_linear_2d_tech_ = effect_->TechniqueByName("BlitLinear2D");
		blit_linear_2d_array_tech_ = effect_->TechniqueByName("BlitLinear2DArray");
		blit_point_3d_tech_ = effect_->TechniqueByName("BlitPoint3D");
		blit_linear_3d_tech_ = effect_->TechniqueByName("BlitLinear3D");
		blit_buff_to_2d_tech_ = effect_->TechniqueByName("BlitBuffTo2D");
		blit_2d_to_buff_tech_ = effect_->TechniqueByName("Blit2DToBuff");
		blit_2d_array_to_buff_tech_ = effect_->TechniqueByName("Blit2DArrayToBuff");
		src_2d_tex_param_ = effect_->ParameterByName("src_2d_tex");
		src_2d_tex_array_param_ = effect_->ParameterByName("src_2d_tex_array");
		src_3d_tex_param_ = effect_->ParameterByName("src_3d_tex");
		src_array_index_param_ = effect_->ParameterByName("src_array_index");
		src_level_param_ = effect_->ParameterByName("src_level");
		src_offset_param_ = effect_->ParameterByName("src_offset");
		src_scale_param_ = effect_->ParameterByName("src_scale");
		dst_offset_param_ = effect_->ParameterByName("dst_offset");
		dst_scale_param_ = effect_->ParameterByName("dst_scale");
		dst_width_height_param_ = effect_->ParameterByName("dst_width_height");
	}

	void Blitter::Blit(TexturePtr const& dst, uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset,
		uint32_t dst_width, uint32_t dst_height, TexturePtr const& src, uint32_t src_array_index, uint32_t src_level, float src_x_offset,
		float src_y_offset, float src_width, float src_height, TextureFilter filter)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();

		frame_buffer_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(dst, dst_array_index, 1, dst_level));
		frame_buffer_->Viewport()->Left(dst_x_offset);
		frame_buffer_->Viewport()->Top(dst_y_offset);
		frame_buffer_->Viewport()->Width(dst_width);
		frame_buffer_->Viewport()->Height(dst_height);

		uint32_t const src_w = src->Width(src_level);
		uint32_t const src_h = src->Height(src_level);

		*src_array_index_param_ = static_cast<int32_t>(src_array_index);
		*src_level_param_ = static_cast<int32_t>(src_level);
		*src_offset_param_ = float3(src_x_offset / src_w, src_y_offset / src_h, 0);
		*src_scale_param_ = float3(src_width / src_w, src_height / src_h, 1);

		auto srv = rf.MakeTextureSrv(src, src_array_index, 1, src_level, 1);
		RenderTechnique* tech;
		if ((Texture::TT_Cube == src->Type()) || (src->ArraySize() > 1))
		{
			*src_2d_tex_array_param_ = srv;
			tech = (filter == TextureFilter::Linear) ? blit_linear_2d_array_tech_ : blit_point_2d_array_tech_;
		}
		else
		{
			BOOST_ASSERT(0 == src_array_index);

			*src_2d_tex_param_ = srv;
			tech = (filter == TextureFilter::Linear) ? blit_linear_2d_tech_ : blit_point_2d_tech_;
		}

		FrameBufferPtr curr_fb = re.CurFrameBuffer();
		re.BindFrameBuffer(frame_buffer_);
		re.Render(*effect_, *tech, *quad_rl_);
		re.BindFrameBuffer(curr_fb);
	}

	void Blitter::Blit(TexturePtr const& dst, uint32_t dst_array_index, uint32_t dst_level, uint32_t dst_x_offset, uint32_t dst_y_offset,
		uint32_t dst_z_offset, uint32_t dst_width, uint32_t dst_height, uint32_t dst_depth, TexturePtr const& src, uint32_t src_array_index,
		uint32_t src_level, float src_x_offset, float src_y_offset, float src_z_offset, float src_width, float src_height, float src_depth,
		TextureFilter filter)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();

		frame_buffer_->Viewport()->Left(dst_x_offset);
		frame_buffer_->Viewport()->Top(dst_y_offset);
		frame_buffer_->Viewport()->Width(dst_width);
		frame_buffer_->Viewport()->Height(dst_height);

		uint32_t const src_w = src->Width(src_level);
		uint32_t const src_h = src->Height(src_level);
		uint32_t const src_d = src->Depth(src_level);

		*src_array_index_param_ = static_cast<int32_t>(src_array_index);
		*src_level_param_ = static_cast<int32_t>(src_level);
		*src_offset_param_ = float3(src_x_offset / src_w, src_y_offset / src_h, src_z_offset / src_d);
		*src_scale_param_ = float3(src_width / src_w, src_height / src_h, src_depth / src_d);

		*src_3d_tex_param_ = rf.MakeTextureSrv(src, src_array_index, 1, src_level, 1);
		RenderTechnique* tech = (filter == TextureFilter::Linear) ? blit_linear_3d_tech_ : blit_point_3d_tech_;

		FrameBufferPtr curr_fb = re.CurFrameBuffer();
		for (uint32_t z = 0; z < dst_depth; ++ z)
		{
			frame_buffer_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(dst, dst_array_index, dst_z_offset + z, dst_level));
			re.BindFrameBuffer(frame_buffer_);
			re.Render(*effect_, *tech, *quad_rl_);
		}
		re.BindFrameBuffer(curr_fb);
	}

	void Blitter::Blit(TexturePtr const& dst, uint32_t dst_array_index, Texture::CubeFaces dst_face, uint32_t dst_level,
		uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height, TexturePtr const& src,
		uint32_t src_array_index, Texture::CubeFaces src_face, uint32_t src_level, float src_x_offset, float src_y_offset,
		float src_width, float src_height, TextureFilter filter)
	{
		this->Blit(dst, dst_array_index * 6 + dst_face, dst_level, dst_x_offset, dst_y_offset, dst_width, dst_height, src,
			src_array_index * 6 + src_face, src_level, src_x_offset, src_y_offset, src_width, src_height, filter);
	}

	void Blitter::Blit(GraphicsBufferPtr const& dst, uint32_t dst_x_offset, TexturePtr const& src, uint32_t src_array_index,
		uint32_t src_level, float src_x_offset, float src_y_offset, uint32_t src_width, uint32_t src_height)
	{
		// TODO: Relax this limitation
		BOOST_ASSERT(0 == dst_x_offset);
		KFL_UNUSED(dst_x_offset);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();

		uint32_t const src_w = src->Width(src_level);
		uint32_t const src_h = src->Height(src_level);

		*src_array_index_param_ = static_cast<int32_t>(src_array_index);
		*src_level_param_ = static_cast<int32_t>(src_level);
		*src_offset_param_ = float3(src_x_offset / src_w, src_y_offset / src_h, 0);
		*src_scale_param_ = float3(1.0f / src_w, 1.0f / src_h, 1);
		*dst_width_height_param_ = float4(static_cast<float>(src_width), 0, 0, 0);

		auto srv = rf.MakeTextureSrv(src, src_array_index, 1, src_level, 1);
		RenderTechnique* tech;
		if (src->ArraySize() > 1)
		{
			*src_2d_tex_array_param_ = srv;
			tech = blit_2d_array_to_buff_tech_;
		}
		else
		{
			BOOST_ASSERT(0 == src_array_index);

			*src_2d_tex_param_ = srv;
			tech = blit_2d_to_buff_tech_;
		}

		size_t const pos_size = src_width * src_height;
		auto pos_data = MakeUniquePtr<float[]>(pos_size);
		for (uint32_t y = 0; y < src_height; ++ y)
		{
			for (uint32_t x = 0; x < src_width; ++ x)
			{
				pos_data[y * src_width + x] = y * src_width + x + 0.5f;
			}
		}

		GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(
			BU_Static, EAH_GPU_Read | EAH_Immutable, static_cast<uint32_t>(pos_size * sizeof(pos_data[0])), pos_data.get());
		tex_to_buff_rl_->BindVertexStream(pos_vb, VertexElement(VEU_Position, 0, EF_R32F));

		tex_to_buff_so_rl_->BindVertexStream(dst, VertexElement(VEU_Diffuse, 0, EF_ABGR32F));

		re.BindSOBuffers(tex_to_buff_so_rl_);
		re.Render(*effect_, *tech, *tex_to_buff_rl_);
		re.BindSOBuffers(RenderLayoutPtr());
	}

	void Blitter::Blit(TexturePtr const & dst, uint32_t dst_array_index, uint32_t dst_level,
		uint32_t dst_x_offset, uint32_t dst_y_offset, uint32_t dst_width, uint32_t dst_height,
		GraphicsBufferPtr const & src, uint32_t src_x_offset, ElementFormat src_fmt)
	{
		// TODO: Relax this limitation
		BOOST_ASSERT(0 == src_x_offset);
		KFL_UNUSED(src_x_offset);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEngine& re = rf.RenderEngineInstance();

		frame_buffer_->Attach(FrameBuffer::Attachment::Color0, rf.Make2DRtv(dst, dst_array_index, 1, dst_level));
		frame_buffer_->Viewport()->Left(dst_x_offset);
		frame_buffer_->Viewport()->Top(dst_y_offset);
		frame_buffer_->Viewport()->Width(dst_width);
		frame_buffer_->Viewport()->Height(dst_height);

		*dst_width_height_param_ = float4(static_cast<float>(dst_width), static_cast<float>(dst_height),
			1.0f / dst_width, 1.0f / dst_height);

		size_t const pos_size = dst_width * dst_height;
		auto pos_data = MakeUniquePtr<float[]>(pos_size);
		for (uint32_t y = 0; y < dst_height; ++ y)
		{
			for (uint32_t x = 0; x < dst_width; ++ x)
			{
				pos_data[y * dst_width + x] = y * dst_width + x + 0.5f;
			}
		}

		GraphicsBufferPtr pos_vb = rf.MakeVertexBuffer(
			BU_Static, EAH_GPU_Read | EAH_Immutable, static_cast<uint32_t>(pos_size * sizeof(pos_data[0])), pos_data.get());
		buff_to_tex_rl_->BindVertexStream(pos_vb, VertexElement(VEU_Position, 0, EF_R32F));
		buff_to_tex_rl_->BindVertexStream(src, VertexElement(VEU_Diffuse, 0, src_fmt));

		FrameBufferPtr curr_fb = re.CurFrameBuffer();
		re.BindFrameBuffer(frame_buffer_);
		re.Render(*effect_, *blit_buff_to_2d_tech_, *buff_to_tex_rl_);
		re.BindFrameBuffer(curr_fb);
	}
}
