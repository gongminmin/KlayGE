// RenderView.hpp
// KlayGE 渲染视图类 头文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2006-2007
// Homepage: http://www.klayge.org
//
// 3.7.0
// 增加了Clear (2007.8.23)
//
// 3.3.0
// 初次建立 (2006.5.31)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERVIEW_HPP
#define _RENDERVIEW_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/ElementFormat.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/Texture.hpp>

namespace KlayGE
{
	class KLAYGE_CORE_API ShaderResourceView : boost::noncopyable
	{
	public:
		virtual ~ShaderResourceView() noexcept;

		ElementFormat Format() const
		{
			return pf_;
		}

		TexturePtr const & TextureResource() const
		{
			return tex_;
		}
		uint32_t FirstArrayIndex() const
		{
			return first_array_index_;
		}
		uint32_t ArraySize() const
		{
			return array_size_;
		}
		uint32_t FirstLevel() const
		{
			return first_level_;
		}
		uint32_t NumLevels() const
		{
			return num_levels_;
		}

		GraphicsBufferPtr const & BufferResource() const
		{
			return buff_;
		}
		uint32_t FirstElement() const
		{
			return first_elem_;
		}
		uint32_t NumElements() const
		{
			return num_elems_;
		}

	protected:
		ElementFormat pf_;

		// For textures
		TexturePtr tex_;
		uint32_t first_array_index_;
		uint32_t array_size_;
		uint32_t first_level_;
		uint32_t num_levels_;

		// For graphics buffers
		GraphicsBufferPtr buff_;
		uint32_t first_elem_;
		uint32_t num_elems_;
	};

	class KLAYGE_CORE_API RenderTargetView : boost::noncopyable
	{
	public:
		virtual ~RenderTargetView() noexcept;

		uint32_t Width() const
		{
			return width_;
		}
		uint32_t Height() const
		{
			return height_;
		}
		ElementFormat Format() const
		{
			return pf_;
		}
		uint32_t SampleCount() const
		{
			return sample_count_;
		}
		uint32_t SampleQuality() const
		{
			return sample_quality_;
		}

		TexturePtr const & TextureResource() const
		{
			return tex_;
		}
		uint32_t FirstArrayIndex() const
		{
			return first_array_index_;
		}
		uint32_t ArraySize() const
		{
			return array_size_;
		}
		uint32_t Level() const
		{
			return level_;
		}
		uint32_t FirstSlice() const
		{
			return first_slice_;
		}
		uint32_t NumSlices() const
		{
			return num_slices_;
		}
		Texture::CubeFaces FirstFace() const
		{
			return first_face_;
		}
		uint32_t NumFaces() const
		{
			return num_faces_;
		}

		GraphicsBufferPtr const & BufferResource() const
		{
			return buff_;
		}
		uint32_t FirstElement() const
		{
			return first_elem_;
		}
		uint32_t NumElements() const
		{
			return num_elems_;
		}

		virtual void ClearColor(Color const & clr) = 0;

		virtual void Discard() = 0;

		virtual void OnAttached(FrameBuffer& fb, FrameBuffer::Attachment att) = 0;
		virtual void OnDetached(FrameBuffer& fb, FrameBuffer::Attachment att) = 0;

	protected:
		uint32_t width_;
		uint32_t height_;
		ElementFormat pf_;
		uint32_t sample_count_;
		uint32_t sample_quality_;

		// For textures
		TexturePtr tex_;
		uint32_t first_array_index_;
		uint32_t array_size_;
		uint32_t level_;

		// For 3D textures
		uint32_t first_slice_;
		uint32_t num_slices_;

		// For cube textures
		Texture::CubeFaces first_face_;
		uint32_t num_faces_;

		// For buffers
		GraphicsBufferPtr buff_;
		uint32_t first_elem_;
		uint32_t num_elems_;
	};

	class KLAYGE_CORE_API DepthStencilView : boost::noncopyable
	{
	public:
		virtual ~DepthStencilView() noexcept;

		uint32_t Width() const
		{
			return width_;
		}
		uint32_t Height() const
		{
			return height_;
		}
		ElementFormat Format() const
		{
			return pf_;
		}
		uint32_t SampleCount() const
		{
			return sample_count_;
		}
		uint32_t SampleQuality() const
		{
			return sample_quality_;
		}

		TexturePtr const & TextureResource() const
		{
			return tex_;
		}
		uint32_t FirstArrayIndex() const
		{
			return first_array_index_;
		}
		uint32_t ArraySize() const
		{
			return array_size_;
		}
		uint32_t Level() const
		{
			return level_;
		}
		uint32_t FirstSlice() const
		{
			return first_slice_;
		}
		uint32_t NumSlices() const
		{
			return num_slices_;
		}
		Texture::CubeFaces FirstFace() const
		{
			return first_face_;
		}
		uint32_t NumFaces() const
		{
			return num_faces_;
		}

		virtual void ClearDepth(float depth) = 0;
		virtual void ClearStencil(int32_t stencil) = 0;
		virtual void ClearDepthStencil(float depth, int32_t stencil) = 0;

		virtual void Discard() = 0;

		virtual void OnAttached(FrameBuffer& fb) = 0;
		virtual void OnDetached(FrameBuffer& fb) = 0;

	protected:
		uint32_t width_;
		uint32_t height_;
		ElementFormat pf_;
		uint32_t sample_count_;
		uint32_t sample_quality_;

		// For textures
		TexturePtr tex_;
		uint32_t first_array_index_;
		uint32_t array_size_;
		uint32_t level_;

		// For 3D textures
		uint32_t first_slice_;
		uint32_t num_slices_;

		// For cube textures
		Texture::CubeFaces first_face_;
		uint32_t num_faces_;
	};

	class KLAYGE_CORE_API UnorderedAccessView : boost::noncopyable
	{
	public:
		virtual ~UnorderedAccessView() noexcept;

		ElementFormat Format() const
		{
			return pf_;
		}

		void InitCount(uint32_t count)
		{
			init_count_ = count;
		}
		uint32_t InitCount() const
		{
			return init_count_;
		}

		TexturePtr const & TextureResource() const
		{
			return tex_;
		}
		uint32_t FirstArrayIndex() const
		{
			return first_array_index_;
		}
		uint32_t ArraySize() const
		{
			return array_size_;
		}
		uint32_t Level() const
		{
			return level_;
		}
		uint32_t FirstSlice() const
		{
			return first_slice_;
		}
		uint32_t NumSlices() const
		{
			return num_slices_;
		}
		Texture::CubeFaces FirstFace() const
		{
			return first_face_;
		}
		uint32_t NumFaces() const
		{
			return num_faces_;
		}

		GraphicsBufferPtr const & BufferResource() const
		{
			return buff_;
		}
		uint32_t FirstElement() const
		{
			return first_elem_;
		}
		uint32_t NumElements() const
		{
			return num_elems_;
		}

		virtual void Clear(float4 const & val) = 0;
		virtual void Clear(uint4 const & val) = 0;

		virtual void Discard() = 0;

		virtual void OnAttached(FrameBuffer& fb, uint32_t att) = 0;
		virtual void OnDetached(FrameBuffer& fb, uint32_t att) = 0;

	protected:
		ElementFormat pf_;
		uint32_t init_count_ = 0;

		// For textures
		TexturePtr tex_;
		uint32_t first_array_index_;
		uint32_t array_size_;
		uint32_t level_;

		// For 3D textures
		uint32_t first_slice_;
		uint32_t num_slices_;

		// For cube textures
		Texture::CubeFaces first_face_;
		uint32_t num_faces_;

		// For buffers
		GraphicsBufferPtr buff_;
		uint32_t first_elem_;
		uint32_t num_elems_;
	};
}

#endif			// _RENDERVIEW_HPP
