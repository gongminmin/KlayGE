// D3D10ShaderObject.hpp
// KlayGE D3D10 shader对象类 头文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2008
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 初次建立 (2008.9.21)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D10SHADEROBJECT_HPP
#define _D3D10SHADEROBJECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/ShaderObject.hpp>
#include <KlayGE/MapVector.hpp>

#include <boost/function.hpp>

#include <KlayGE/D3D10/D3D10Typedefs.hpp>

namespace KlayGE
{
#ifdef KLAYGE_PLATFORM_WINDOWS
#pragma pack(push, 2)
#endif
	struct D3D10ShaderParameterHandle
	{
		uint32_t shader_type;

		D3D10_SHADER_VARIABLE_CLASS param_class;
	    D3D10_SHADER_VARIABLE_TYPE param_type;

		uint32_t cbuff;

		uint32_t offset;
		uint32_t elements;
		uint8_t rows;
		uint8_t columns;
	};
#ifdef KLAYGE_PLATFORM_WINDOWS
#pragma pack(pop)
#endif

	class D3D10ShaderObject : public ShaderObject
	{
	public:
		D3D10ShaderObject();

		std::string GenShaderText(RenderEffect const & effect) const;

		void SetShader(RenderEffect& effect, boost::shared_ptr<std::vector<uint32_t> > const & shader_desc_ids,
			uint32_t tech_index, uint32_t pass_index);
		ShaderObjectPtr Clone(RenderEffect& effect);

		void Bind();
		void Unbind();

		ID3D10BlobPtr const & VSCode() const
		{
			return vs_code_;
		}

	private:
		struct parameter_bind_t
		{
			RenderEffectParameterPtr param;
			D3D10ShaderParameterHandle p_handle;
			boost::function<void()> func;
		};
		typedef std::vector<parameter_bind_t> parameter_binds_t;

		parameter_bind_t GetBindFunc(D3D10ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param);

	private:
		boost::array<parameter_binds_t, ST_NumShaderTypes> param_binds_;
		boost::array<bool, ST_NumShaderTypes> is_shader_validate_;

		ID3D10VertexShaderPtr vertex_shader_;
		ID3D10PixelShaderPtr pixel_shader_;
		ID3D10GeometryShaderPtr geometry_shader_;
		ID3D10BlobPtr vs_code_;

		boost::array<std::vector<TexturePtr>, ST_NumShaderTypes> textures_;
		boost::array<std::vector<SamplerStateObjectPtr>, ST_NumShaderTypes> samplers_;
		boost::array<std::vector<GraphicsBufferPtr>, ST_NumShaderTypes> buffers_;

		boost::array<std::vector<char>, ST_NumShaderTypes> dirty_;
		boost::array<std::vector<std::vector<uint8_t> >, ST_NumShaderTypes> cbufs_;
		boost::array<std::vector<ID3D10BufferPtr>, ST_NumShaderTypes> d3d_cbufs_;
	};

	typedef boost::shared_ptr<D3D10ShaderObject> D3D10ShaderObjectPtr;
}

#endif			// _D3D10SHADEROBJECT_HPP
