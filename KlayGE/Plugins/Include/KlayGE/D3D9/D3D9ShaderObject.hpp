// D3D9ShaderObject.hpp
// KlayGE D3D9 shader对象类 头文件
// Ver 3.5.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
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

		void SetShader(ShaderType type, boost::shared_ptr<std::vector<shader_desc> > const & shader_descs,
			boost::shared_ptr<std::string> const & shader_text);
		ShaderObjectPtr Clone();

		bool HasParameter(ShaderType type, boost::shared_ptr<std::string> const & name) const;

		void SetParameter(boost::shared_ptr<std::string> const & name, bool value);
		void SetParameter(boost::shared_ptr<std::string> const & name, int value);
		void SetParameter(boost::shared_ptr<std::string> const & name, float value);
		void SetParameter(boost::shared_ptr<std::string> const & name, float4 const & value);
		void SetParameter(boost::shared_ptr<std::string> const & name, float4x4 const & value);
		void SetParameter(boost::shared_ptr<std::string> const & name, SamplerPtr const & value);
		void SetParameter(boost::shared_ptr<std::string> const & name, std::vector<bool> const & value);
		void SetParameter(boost::shared_ptr<std::string> const & name, std::vector<int> const & value);
		void SetParameter(boost::shared_ptr<std::string> const & name, std::vector<float> const & value);
		void SetParameter(boost::shared_ptr<std::string> const & name, std::vector<float4> const & value);
		void SetParameter(boost::shared_ptr<std::string> const & name, std::vector<float4x4> const & value);

		void Active();

	private:
		struct less_shared_ptr_string
			: public std::binary_function<boost::shared_ptr<std::string>, boost::shared_ptr<std::string>, bool>
		{
			bool operator()(boost::shared_ptr<std::string> lhs, boost::shared_ptr<std::string> rhs) const
			{
				return *lhs < *rhs;
			}
		};

		typedef MapVector<boost::shared_ptr<std::string>, D3D9ShaderParameterHandle, less_shared_ptr_string> parameter_descs_t;
		parameter_descs_t::const_iterator FindParam(ShaderType type, boost::shared_ptr<std::string> const & name) const;

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
