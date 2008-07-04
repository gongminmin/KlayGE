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

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/ShaderObject.hpp>
#include <KlayGE/MapVector.hpp>

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

		void SetShader(RenderEffect& effect, ShaderType type, boost::shared_ptr<std::vector<shader_desc> > const & shader_descs,
			boost::shared_ptr<std::string> const & shader_text);
		ShaderObjectPtr Clone(RenderEffect& effect);

		void Active();

	private:
		typedef MapVector<RenderEffectParameterPtr, D3D9ShaderParameterHandle> parameter_descs_t;

		void SetParameter(D3D9ShaderParameterHandle const & p_handle, bool value);
		void SetParameter(D3D9ShaderParameterHandle const & p_handle, int value);
		void SetParameter(D3D9ShaderParameterHandle const & p_handle, float value);
		void SetParameter(D3D9ShaderParameterHandle const & p_handle, float4 const & value);
		void SetParameter(D3D9ShaderParameterHandle const & p_handle, float4x4 const & value);
		void SetParameter(D3D9ShaderParameterHandle const & p_handle, SamplerPtr const & value);
		void SetParameter(D3D9ShaderParameterHandle const & p_handle, std::vector<bool> const & value);
		void SetParameter(D3D9ShaderParameterHandle const & p_handle, std::vector<int> const & value);
		void SetParameter(D3D9ShaderParameterHandle const & p_handle, std::vector<float> const & value);
		void SetParameter(D3D9ShaderParameterHandle const & p_handle, std::vector<float4> const & value);
		void SetParameter(D3D9ShaderParameterHandle const & p_handle, std::vector<float4x4> const & value);

	private:
		boost::array<parameter_descs_t, ST_NumShaderTypes> param_descs_;
		boost::array<bool, ST_NumShaderTypes> is_shader_validate_;

		ID3D9VertexShaderPtr vertex_shader_;
		ID3D9PixelShaderPtr pixel_shader_;

		boost::array<uint32_t, ST_NumShaderTypes> bool_start_;
		boost::array<uint32_t, ST_NumShaderTypes> int_start_;
		boost::array<uint32_t, ST_NumShaderTypes> float_start_;
		boost::array<std::vector<BOOL>, ST_NumShaderTypes> bool_registers_;
		boost::array<std::vector<int>, ST_NumShaderTypes> int_registers_;
		boost::array<std::vector<float>, ST_NumShaderTypes> float_registers_;
		boost::array<std::vector<SamplerPtr>, ST_NumShaderTypes> samplers_;
	};

	typedef boost::shared_ptr<D3D9ShaderObject> D3D9ShaderObjectPtr;
}

#endif			// _D3D9SHADEROBJECT_HPP
