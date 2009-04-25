// D3D9ShaderObject.hpp
// KlayGE D3D9 shader对象类 头文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2006-2008
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 改为直接传入RenderEffect (2008.7.4)
//
// 3.5.0
// 初次建立 (2006.11.2)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9SHADEROBJECT_HPP
#define _D3D9SHADEROBJECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/ShaderObject.hpp>
#include <KlayGE/MapVector.hpp>

#include <boost/function.hpp>

#include <KlayGE/D3D9/D3D9Typedefs.hpp>

namespace KlayGE
{
#ifdef KLAYGE_PLATFORM_WINDOWS
#pragma pack(push, 2)
#endif
	struct D3D9ShaderParameterHandle
	{
		uint8_t shader_type;
		uint8_t register_set;

		uint16_t register_index;
		uint16_t register_count;

		uint8_t rows;
		uint8_t columns;
	};
#ifdef KLAYGE_PLATFORM_WINDOWS
#pragma pack(pop)
#endif

	class D3D9ShaderObject : public ShaderObject
	{
	public:
		D3D9ShaderObject();

		std::string GenShaderText(RenderEffect const & effect) const;

		void SetShader(RenderEffect& effect, boost::shared_ptr<std::vector<shader_desc> > const & shader_descs);
		ShaderObjectPtr Clone(RenderEffect& effect);

		void Bind();
		void Unbind();

	private:
		struct parameter_bind_t
		{
			std::string combined_sampler_name;
			RenderEffectParameterPtr param;
			D3D9ShaderParameterHandle p_handle;
			boost::function<void()> func;
		};
		typedef std::vector<parameter_bind_t> parameter_binds_t;

		parameter_bind_t GetBindFunc(D3D9ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param);

	private:
		boost::array<parameter_binds_t, ST_NumShaderTypes> param_binds_;
		boost::array<bool, ST_NumShaderTypes> is_shader_validate_;

		ID3D9VertexShaderPtr vertex_shader_;
		ID3D9PixelShaderPtr pixel_shader_;

		typedef Vector_T<BOOL, 4> BOOL4;
		boost::array<uint32_t, ST_NumShaderTypes> bool_start_;
		boost::array<uint32_t, ST_NumShaderTypes> int_start_;
		boost::array<uint32_t, ST_NumShaderTypes> float_start_;
		boost::array<std::vector<BOOL4>, ST_NumShaderTypes> bool_registers_;
		boost::array<std::vector<int4>, ST_NumShaderTypes> int_registers_;
		boost::array<std::vector<float4>, ST_NumShaderTypes> float_registers_;
		boost::array<std::vector<std::pair<TexturePtr, SamplerStateObjectPtr> >, ST_NumShaderTypes> samplers_;

		mutable std::vector<std::pair<std::string, std::pair<RenderEffectParameterPtr, RenderEffectParameterPtr> > > tex_sampler_binds_;
	};

	typedef boost::shared_ptr<D3D9ShaderObject> D3D9ShaderObjectPtr;
}

#endif			// _D3D9SHADEROBJECT_HPP
