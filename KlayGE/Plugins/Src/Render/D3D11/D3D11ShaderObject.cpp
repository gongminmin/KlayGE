// D3D11ShaderObject.cpp
// KlayGE D3D11 shader对象类 实现文件
// Ver 3.8.0
// 版权所有(C) 龚敏敏, 2009
// Homepage: http://klayge.sourceforge.net
//
// 3.8.0
// 支持Gemoetry Shader (2009.2.5)
// 初次建立 (2009.1.30)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#define INITGUID
#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEffect.hpp>

#include <string>
#include <map>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <boost/assert.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

#include <KlayGE/D3D11/D3D11MinGWDefs.hpp>
#include <d3d11.h>
#include <d3dx11.h>
#include <D3D11Shader.h>
#include <D3DCompiler.h>

#include <KlayGE/D3D11/D3D11RenderEngine.hpp>
#include <KlayGE/D3D11/D3D11RenderStateObject.hpp>
#include <KlayGE/D3D11/D3D11Mapping.hpp>
#include <KlayGE/D3D11/D3D11Texture.hpp>
#include <KlayGE/D3D11/D3D11ShaderObject.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "d3dcompiler.lib")
#endif

namespace
{
	using namespace KlayGE;

	template <typename SrcType, typename DstType>
	class SetD3D11ShaderParameter
	{
	public:
		SetD3D11ShaderParameter(uint8_t* target, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<DstType*>(target)), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			SrcType v;
			param_->Value(v);

			if (*target_ != static_cast<DstType>(v))
			{
				*target_ = static_cast<DstType>(v);
				*dirty_ = true;
			}
		}

	private:
		DstType* target_;
		RenderEffectParameterPtr param_;
		char* dirty_;
	};

	template <>
	class SetD3D11ShaderParameter<float2, float>
	{
	public:
		SetD3D11ShaderParameter(uint8_t* target, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<float2*>(target)), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			float2 v;
			param_->Value(v);

			if (*target_ != v)
			{
				*target_ = v;
				*dirty_ = true;
			}
		}

	private:
		float2* target_;
		RenderEffectParameterPtr param_;
		char* dirty_;
	};

	template <>
	class SetD3D11ShaderParameter<float3, float>
	{
	public:
		SetD3D11ShaderParameter(uint8_t* target, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<float3*>(target)), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			float3 v;
			param_->Value(v);

			if (*target_ != v)
			{
				*target_ = v;
				*dirty_ = true;
			}
		}

	private:
		float3* target_;
		RenderEffectParameterPtr param_;
		char* dirty_;
	};

	template <>
	class SetD3D11ShaderParameter<float4, float>
	{
	public:
		SetD3D11ShaderParameter(uint8_t* target, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<float4*>(target)), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			float4 v;
			param_->Value(v);

			if (*target_ != v)
			{
				*target_ = v;
				*dirty_ = true;
			}
		}

	private:
		float4* target_;
		RenderEffectParameterPtr param_;
		char* dirty_;
	};

	template <>
	class SetD3D11ShaderParameter<float4x4, float>
	{
	public:
		SetD3D11ShaderParameter(uint8_t* target, uint32_t rows, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<float4*>(target)), size_(rows * sizeof(float4)), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			float4x4 v;
			param_->Value(v);

			v = MathLib::transpose(v);
			memcpy(target_, &v[0], size_);
			*dirty_ = true;
		}

	private:
		float4* target_;
		size_t size_;
		RenderEffectParameterPtr param_;
		char* dirty_;
	};

	template <typename SrcType, typename DstType>
	class SetD3D11ShaderParameter<SrcType*, DstType>
	{
	public:
		SetD3D11ShaderParameter(uint8_t* target, uint32_t elements, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<DstType*>(target)), elements_(elements), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			std::vector<SrcType> v;
			param_->Value(v);

			for (size_t i = 0; i < v.size(); ++ i)
			{
				target_[i * 4] = static_cast<DstType>(v[i]);
			}
			*dirty_ = true;
		}

	private:
		DstType* target_;
		uint32_t elements_;
		RenderEffectParameterPtr param_;
		char* dirty_;
	};

	template <>
	class SetD3D11ShaderParameter<float2*, float>
	{
	public:
		SetD3D11ShaderParameter(uint8_t* target, uint32_t elements, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<float4*>(target)), size_(elements * sizeof(float4)), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			std::vector<float2> v;
			param_->Value(v);

			std::vector<float4> v4(v.size());
			for (size_t i = 0; i < v.size(); ++ i)
			{
				v4[i] = float4(v[i].x(), v[i].y(), 0, 0);
			}

			if (!v.empty())
			{
				memcpy(target_, &v4[0], std::min(size_, v4.size() * sizeof(float4)));
			}
			*dirty_ = true;
		}

	private:
		float4* target_;
		size_t size_;
		RenderEffectParameterPtr param_;
		char* dirty_;
	};

	template <>
	class SetD3D11ShaderParameter<float3*, float>
	{
	public:
		SetD3D11ShaderParameter(uint8_t* target, uint32_t elements, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<float4*>(target)), size_(elements * sizeof(float4)), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			std::vector<float3> v;
			param_->Value(v);

			std::vector<float4> v4(v.size());
			for (size_t i = 0; i < v.size(); ++ i)
			{
				v4[i] = float4(v[i].x(), v[i].y(), v[i].z(), 0);
			}

			if (!v.empty())
			{
				memcpy(target_, &v4[0], std::min(size_, v4.size() * sizeof(float4)));
			}
			*dirty_ = true;
		}

	private:
		float4* target_;
		size_t size_;
		RenderEffectParameterPtr param_;
		char* dirty_;
	};

	template <>
	class SetD3D11ShaderParameter<float4*, float>
	{
	public:
		SetD3D11ShaderParameter(uint8_t* target, uint32_t elements, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<float4*>(target)), size_(elements * sizeof(float4)), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			std::vector<float4> v;
			param_->Value(v);

			if (!v.empty())
			{
				memcpy(target_, &v[0], std::min(size_, v.size() * sizeof(float4)));
			}
			*dirty_ = true;
		}

	private:
		float4* target_;
		size_t size_;
		RenderEffectParameterPtr param_;
		char* dirty_;
	};

	template <>
	class SetD3D11ShaderParameter<float4x4*, float>
	{
	public:
		SetD3D11ShaderParameter(uint8_t* target, size_t rows, RenderEffectParameterPtr const & param, char* dirty)
			: target_(reinterpret_cast<float4*>(target)), rows_(rows), param_(param), dirty_(dirty)
		{
		}

		void operator()()
		{
			std::vector<float4x4> v;
			param_->Value(v);


			size_t start = 0;
			BOOST_FOREACH(BOOST_TYPEOF(v)::reference mat, v)
			{
				mat = MathLib::transpose(mat);
				memcpy(&target_[start], &mat[0], rows_ * sizeof(float4));
				start += rows_;
			}
			*dirty_ = true;
		}

	private:
		float4* target_;
		size_t rows_;
		RenderEffectParameterPtr param_;
		char* dirty_;
	};

	template <>
	class SetD3D11ShaderParameter<TexturePtr, TexturePtr>
	{
	public:
		SetD3D11ShaderParameter(TexturePtr& texture, RenderEffectParameterPtr const & param)
			: texture_(&texture), param_(param)
		{
		}

		void operator()()
		{
			param_->Value(*texture_);
		}

	private:
		TexturePtr* texture_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D11ShaderParameter<SamplerStateObjectPtr, SamplerStateObjectPtr>
	{
	public:
		SetD3D11ShaderParameter(SamplerStateObjectPtr& sampler, RenderEffectParameterPtr const & param)
			: sampler_(&sampler), param_(param)
		{
		}

		void operator()()
		{
			param_->Value(*sampler_);
		}

	private:
		SamplerStateObjectPtr* sampler_;
		RenderEffectParameterPtr param_;
	};

	template <>
	class SetD3D11ShaderParameter<GraphicsBufferPtr, GraphicsBufferPtr>
	{
	public:
		SetD3D11ShaderParameter(GraphicsBufferPtr& buffer, RenderEffectParameterPtr const & param)
			: buffer_(&buffer), param_(param)
		{
		}

		void operator()()
		{
			param_->Value(*buffer_);
		}

	private:
		GraphicsBufferPtr* buffer_;
		RenderEffectParameterPtr param_;
	};
}

namespace KlayGE
{
	D3D11ShaderObject::D3D11ShaderObject()
	{
		is_shader_validate_.assign(true);
	}

	std::string D3D11ShaderObject::GenShaderText(RenderEffect const & effect) const
	{
		std::stringstream ss;

		for (uint32_t i = 0; i < effect.NumMacros(); ++ i)
		{
			std::pair<std::string, std::string> const & name_value = effect.MacroByIndex(i);
			ss << "#define " << name_value.first << " " << name_value.second << std::endl;
		}
		ss << std::endl;

		BOOST_AUTO(cbuffers, effect.CBuffers());
		BOOST_FOREACH(BOOST_TYPEOF(cbuffers)::const_reference cbuff, cbuffers)
		{
			ss << "cbuffer " << cbuff.first << std::endl;
			ss << "{" << std::endl;

			BOOST_FOREACH(BOOST_TYPEOF(cbuff.second)::const_reference param_index, cbuff.second)
			{
				RenderEffectParameter& param = *effect.ParameterByIndex(param_index);
				switch (param.type())
				{
				case REDT_texture1D:
				case REDT_texture2D:
				case REDT_texture3D:
				case REDT_textureCUBE:
				case REDT_sampler:
				case REDT_buffer:
					break;

				default:
					ss << effect.TypeName(param.type()) << " " << *param.Name();
					if (param.ArraySize())
					{
						ss << "[" << *param.ArraySize() << "]";
					}
					ss << ";" << std::endl;
					break;
				}
			}

			ss << "};" << std::endl;
		}

		for (uint32_t i = 0; i < effect.NumParameters(); ++ i)
		{
			RenderEffectParameter& param = *effect.ParameterByIndex(i);

			switch (param.type())
			{
			case REDT_texture1D:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ss << "Texture1D<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_texture2D:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ss << "Texture2D<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_texture3D:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ss << "Texture3D<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_textureCUBE:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ss << "TextureCube<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_buffer:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ss << "Buffer<" << elem_type << "> " << *param.Name() << ";" << std::endl;
				}
				break;

			case REDT_sampler:
				ss << "sampler " << *param.Name() << ";" << std::endl;
				break;

			default:
				break;
			}
		}

		for (uint32_t i = 0; i < effect.NumShaders(); ++ i)
		{
			ss << effect.ShaderByIndex(i).str() << std::endl;
		}

		return ss.str();
	}

	void D3D11ShaderObject::SetShader(RenderEffect& effect, boost::shared_ptr<std::vector<shader_desc> > const & shader_descs)
	{
		D3D11RenderEngine const & render_eng = *checked_cast<D3D11RenderEngine const *>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DevicePtr const & d3d_device = render_eng.D3DDevice();
		D3D_FEATURE_LEVEL feature_level = render_eng.DeviceFeatureLevel();

		std::string shader_text = this->GenShaderText(effect);

		is_validate_ = true;
		for (size_t type = 0; type < ShaderObject::ST_NumShaderTypes; ++ type)
		{
			shader_desc& sd = (*shader_descs)[type];
			if (!sd.profile.empty())
			{
				is_shader_validate_[type] = true;

				std::string shader_profile = sd.profile;
				switch (type)
				{
				case ST_VertexShader:
					if ("auto" == shader_profile)
					{
						shader_profile = render_eng.VertexShaderProfile();
					}
					break;

				case ST_PixelShader:
					if ("auto" == shader_profile)
					{
						shader_profile = render_eng.PixelShaderProfile();
					}
					break;

				case ST_GeometryShader:
					if ("auto" == shader_profile)
					{
						shader_profile = render_eng.GeometryShaderProfile();
					}
					break;

				default:
					is_shader_validate_[type] = false;
					break;
				}

				ID3D10Blob* code;
				ID3D10Blob* err_msg;
				std::vector<D3D10_SHADER_MACRO> macros;
				{
					D3D10_SHADER_MACRO macro_cb = { "CONSTANT_BUFFER", "1" };
					macros.push_back(macro_cb);
				}
				{
					D3D10_SHADER_MACRO macro_d3d11 = { "KLAYGE_D3D11", "1" };
					macros.push_back(macro_d3d11);
				}
				if (feature_level <= D3D_FEATURE_LEVEL_9_3)
				{
					D3D10_SHADER_MACRO macro_bc5_as_bc3 = { "KLAYGE_BC5_AS_AG", "1" };
					macros.push_back(macro_bc5_as_bc3);
				}
				{
					D3D10_SHADER_MACRO macro_end = { NULL, NULL };
					macros.push_back(macro_end);
				}
				D3DX11CompileFromMemory(shader_text.c_str(), static_cast<UINT>(shader_text.size()), NULL, &macros[0],
					NULL, sd.func_name.c_str(), shader_profile.c_str(),
					0, 0, NULL, &code, &err_msg, NULL);
				if (err_msg != NULL)
				{
#ifdef KLAYGE_DEBUG
					std::istringstream iss(shader_text);
					std::string s;
					int line = 1;
					while (iss)
					{
						std::getline(iss, s);
						std::cerr << line << " " << s << std::endl;
						++ line;
					}
					std::cerr << static_cast<char*>(err_msg->GetBufferPointer()) << std::endl;
#endif

					err_msg->Release();
				}

				ID3D10BlobPtr code_blob;
				if (NULL == code)
				{
					is_shader_validate_[type] = false;
				}
				else
				{
					code_blob = MakeCOMPtr(code);
					switch (type)
					{
					case ST_VertexShader:
						ID3D11VertexShader* vs;
						if (FAILED(d3d_device->CreateVertexShader(code_blob->GetBufferPointer(), code_blob->GetBufferSize(), NULL, &vs)))
						{
							is_shader_validate_[type] = false;
						}
						else
						{
							vertex_shader_ = MakeCOMPtr(vs);

							if (!sd.so_decl.empty())
							{
								std::vector<D3D11_SO_DECLARATION_ENTRY> d3d11_decl(sd.so_decl.size());
								for (size_t i = 0; i < sd.so_decl.size(); ++ i)
								{
									d3d11_decl[i] = D3D11Mapping::Mapping(sd.so_decl[i], static_cast<uint8_t>(i));
								}

								ID3D11GeometryShader* gs;
								if (FAILED(d3d_device->CreateGeometryShaderWithStreamOutput(code_blob->GetBufferPointer(), code_blob->GetBufferSize(),
									&d3d11_decl[0], static_cast<UINT>(d3d11_decl.size()), 0, 0, 0, NULL, &gs)))
								{
									is_shader_validate_[type] = false;
								}
								else
								{
									geometry_shader_ = MakeCOMPtr(gs);
								}
							}
						}
						vs_code_ = code_blob;
						break;

					case ST_PixelShader:
						ID3D11PixelShader* ps;
						if (FAILED(d3d_device->CreatePixelShader(code_blob->GetBufferPointer(), code_blob->GetBufferSize(), NULL, &ps)))
						{
							is_shader_validate_[type] = false;
						}
						else
						{
							pixel_shader_ = MakeCOMPtr(ps);
						}
						break;

					case ST_GeometryShader:
						if (!sd.so_decl.empty())
						{
							std::vector<D3D11_SO_DECLARATION_ENTRY> d3d11_decl(sd.so_decl.size());
							for (size_t i = 0; i < sd.so_decl.size(); ++ i)
							{
								d3d11_decl[i] = D3D11Mapping::Mapping(sd.so_decl[i], static_cast<uint8_t>(i));
							}

							ID3D11GeometryShader* gs;
							if (FAILED(d3d_device->CreateGeometryShaderWithStreamOutput(vs_code_->GetBufferPointer(), code_blob->GetBufferSize(),
								&d3d11_decl[0], static_cast<UINT>(d3d11_decl.size()), 0, 0, 0, NULL, &gs)))
							{
								is_shader_validate_[type] = false;
							}
							else
							{
								geometry_shader_ = MakeCOMPtr(gs);
							}
						}
						else
						{
							ID3D11GeometryShader* gs;
							if (FAILED(d3d_device->CreateGeometryShader(code_blob->GetBufferPointer(), code_blob->GetBufferSize(), NULL, &gs)))
							{
								is_shader_validate_[type] = false;
							}
							else
							{
								geometry_shader_ = MakeCOMPtr(gs);
							}
						}
						break;

					default:
						is_shader_validate_[type] = false;
						break;
					}

					ID3D11ShaderReflection* reflection;
					D3DReflect(code_blob->GetBufferPointer(), code_blob->GetBufferSize(), IID_ID3D11ShaderReflection, reinterpret_cast<void**>(&reflection));
					if (reflection != NULL)
					{
						D3D11_SHADER_DESC desc;
						reflection->GetDesc(&desc);

						dirty_[type].resize(desc.ConstantBuffers);
						d3d_cbufs_[type].resize(desc.ConstantBuffers);
						cbufs_[type].resize(desc.ConstantBuffers);
						for (UINT c = 0; c < desc.ConstantBuffers; ++ c)
						{
							ID3D11ShaderReflectionConstantBuffer* reflection_cb = reflection->GetConstantBufferByIndex(c);

							D3D11_SHADER_BUFFER_DESC cb_desc;
							reflection_cb->GetDesc(&cb_desc);
							cbufs_[type][c].resize(cb_desc.Size);

							for (UINT v = 0; v < cb_desc.Variables; ++ v)
							{
								ID3D11ShaderReflectionVariable* reflection_var = reflection_cb->GetVariableByIndex(v);

								D3D11_SHADER_VARIABLE_DESC var_desc;
								reflection_var->GetDesc(&var_desc);
								if (var_desc.uFlags & D3D10_SVF_USED)
								{
									RenderEffectParameterPtr const & p = effect.ParameterByName(var_desc.Name);
									if (p)
									{
										D3D11_SHADER_TYPE_DESC type_desc;
										reflection_var->GetType()->GetDesc(&type_desc);

										D3D11ShaderParameterHandle p_handle;
										p_handle.shader_type = static_cast<uint8_t>(type);
										p_handle.param_class = type_desc.Class;
										p_handle.param_type = type_desc.Type;
										p_handle.cbuff = c;
										p_handle.offset = var_desc.StartOffset;
										p_handle.elements = type_desc.Elements;
										p_handle.rows = static_cast<uint8_t>(type_desc.Rows);
										p_handle.columns = static_cast<uint8_t>(type_desc.Columns);

										param_binds_[type].push_back(this->GetBindFunc(p_handle, p));
									}
								}
							}

							D3D11_BUFFER_DESC desc;
							desc.ByteWidth = cb_desc.Size;
							desc.Usage = D3D11_USAGE_DEFAULT;
							desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
							desc.CPUAccessFlags = 0;
							desc.MiscFlags = 0;
							ID3D11Buffer* tmp_buf;
							TIF(d3d_device->CreateBuffer(&desc, NULL, &tmp_buf));
							d3d_cbufs_[type][c] = MakeCOMPtr(tmp_buf);
						}

						int num_textures = -1;
						int num_samplers = -1;
						int num_buffers = -1;
						for (uint32_t i = 0; i < desc.BoundResources; ++ i)
						{
							D3D11_SHADER_INPUT_BIND_DESC si_desc;
							reflection->GetResourceBindingDesc(i, &si_desc);

							switch (si_desc.Type)
							{
							case D3D10_SIT_TEXTURE:
								if (si_desc.Dimension != D3D10_SRV_DIMENSION_BUFFER)
								{
									num_textures = std::max(num_textures, static_cast<int>(si_desc.BindPoint));
								}
								else
								{
									num_buffers = std::max(num_buffers, static_cast<int>(si_desc.BindPoint));
								}
								break;

							case D3D10_SIT_SAMPLER:
								num_samplers = std::max(num_samplers, static_cast<int>(si_desc.BindPoint));
								break;

							default:
								break;
							}
						}

						textures_[type].resize(num_textures + 1);
						samplers_[type].resize(num_samplers + 1);
						buffers_[type].resize(num_buffers + 1);

						for (uint32_t i = 0; i < desc.BoundResources; ++ i)
						{
							D3D11_SHADER_INPUT_BIND_DESC si_desc;
							reflection->GetResourceBindingDesc(i, &si_desc);

							if ((D3D10_SIT_TEXTURE == si_desc.Type) || (D3D10_SIT_SAMPLER == si_desc.Type))
							{
								RenderEffectParameterPtr const & p = effect.ParameterByName(si_desc.Name);
								if (p)
								{
									D3D11ShaderParameterHandle p_handle;
									p_handle.shader_type = static_cast<uint8_t>(type);
									p_handle.param_class = D3D10_SVC_OBJECT;
									if (D3D10_SIT_TEXTURE == si_desc.Type)
									{
										if (D3D10_SRV_DIMENSION_BUFFER == si_desc.Dimension)
										{
											p_handle.param_type = D3D10_SVT_BUFFER;
										}
										else
										{
											p_handle.param_type = D3D10_SVT_TEXTURE;
										}
									}
									else
									{
										p_handle.param_type = D3D10_SVT_SAMPLER;
									}
									p_handle.offset = si_desc.BindPoint;
									p_handle.elements = 1;
									p_handle.rows = 0;
									p_handle.columns = 1;

									param_binds_[type].push_back(this->GetBindFunc(p_handle, p));
								}
							}
						}

						reflection->Release();
					}
				}
			}

			is_validate_ &= is_shader_validate_[type];
		}
	}

	ShaderObjectPtr D3D11ShaderObject::Clone(RenderEffect& effect)
	{
		ID3D11DevicePtr const & d3d_device = checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance())->D3DDevice();

		D3D11ShaderObjectPtr ret = MakeSharedPtr<D3D11ShaderObject>();
		ret->is_validate_ = is_validate_;
		ret->is_shader_validate_ = is_shader_validate_;
		ret->vertex_shader_ = vertex_shader_;
		ret->pixel_shader_ = pixel_shader_;
		ret->geometry_shader_ = geometry_shader_;
		ret->vs_code_ = vs_code_;
		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			ret->textures_[i].resize(textures_[i].size());
			ret->samplers_[i].resize(samplers_[i].size());
			ret->buffers_[i].resize(buffers_[i].size());

			ret->cbufs_[i] = cbufs_[i];
			ret->dirty_[i] = dirty_[i];

			ret->d3d_cbufs_[i].resize(d3d_cbufs_[i].size());
			D3D11_BUFFER_DESC desc;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;
			for (size_t j = 0; j < d3d_cbufs_[i].size(); ++ j)
			{
				desc.ByteWidth = static_cast<UINT>(cbufs_[i][j].size());
				ID3D11Buffer* tmp_buf;
				TIF(d3d_device->CreateBuffer(&desc, NULL, &tmp_buf));
				ret->d3d_cbufs_[i][j] = MakeCOMPtr(tmp_buf);
			}

			ret->param_binds_[i].reserve(param_binds_[i].size());
			BOOST_FOREACH(BOOST_TYPEOF(param_binds_[i])::const_reference pb, param_binds_[i])
			{
				ret->param_binds_[i].push_back(ret->GetBindFunc(pb.p_handle, effect.ParameterByName(*(pb.param->Name()))));
			}
		}

		return ret;
	}

	D3D11ShaderObject::parameter_bind_t D3D11ShaderObject::GetBindFunc(D3D11ShaderParameterHandle const & p_handle, RenderEffectParameterPtr const & param)
	{
		parameter_bind_t ret;
		ret.param = param;
		ret.p_handle = p_handle;

		switch (param->type())
		{
		case REDT_bool:
			if (param->ArraySize())
			{
				switch (p_handle.param_type)
				{
				case D3D10_SVT_BOOL:
					ret.func = SetD3D11ShaderParameter<bool*, BOOL>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D10_SVT_INT:
					ret.func = SetD3D11ShaderParameter<bool*, int32_t>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D10_SVT_FLOAT:
					ret.func = SetD3D11ShaderParameter<bool*, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
			else
			{
				switch (p_handle.param_type)
				{
				case D3D10_SVT_BOOL:
					ret.func = SetD3D11ShaderParameter<bool, BOOL>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D10_SVT_INT:
					ret.func = SetD3D11ShaderParameter<bool, int32_t>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D10_SVT_FLOAT:
					ret.func = SetD3D11ShaderParameter<bool, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
			break;

		case REDT_dword:
		case REDT_int:
			if (param->ArraySize())
			{
				switch (p_handle.param_type)
				{
				case D3D10_SVT_BOOL:
					ret.func = SetD3D11ShaderParameter<int32_t*, BOOL>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D10_SVT_INT:
					ret.func = SetD3D11ShaderParameter<int32_t*, int32_t>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D10_SVT_FLOAT:
					ret.func = SetD3D11ShaderParameter<int32_t*, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
			else
			{
				switch (p_handle.param_type)
				{
				case D3D10_SVT_BOOL:
					ret.func = SetD3D11ShaderParameter<int32_t, BOOL>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D10_SVT_INT:
					ret.func = SetD3D11ShaderParameter<int32_t, int32_t>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				case D3D10_SVT_FLOAT:
					ret.func = SetD3D11ShaderParameter<int32_t, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
					break;

				default:
					BOOST_ASSERT(false);
					break;
				}
			}
			break;

		case REDT_float:
			if (param->ArraySize())
			{
				ret.func = SetD3D11ShaderParameter<float*, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			else
			{
				ret.func = SetD3D11ShaderParameter<float, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			break;

		case REDT_float2:
			if (param->ArraySize())
			{
				ret.func = SetD3D11ShaderParameter<float2*, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			else
			{
				ret.func = SetD3D11ShaderParameter<float2, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			break;

		case REDT_float3:
			if (param->ArraySize())
			{
				ret.func = SetD3D11ShaderParameter<float3*, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			else
			{
				ret.func = SetD3D11ShaderParameter<float3, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			break;

		case REDT_float4:
			if (param->ArraySize())
			{
				ret.func = SetD3D11ShaderParameter<float4*, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.elements, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			else
			{
				ret.func = SetD3D11ShaderParameter<float4, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			break;

		case REDT_float4x4:
			if (param->ArraySize())
			{
				ret.func = SetD3D11ShaderParameter<float4x4*, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.rows, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			else
			{
				ret.func = SetD3D11ShaderParameter<float4x4, float>(&cbufs_[p_handle.shader_type][p_handle.cbuff][p_handle.offset], p_handle.rows, param, &dirty_[p_handle.shader_type][p_handle.cbuff]);
			}
			break;

		case REDT_texture1D:
		case REDT_texture2D:
		case REDT_texture3D:
		case REDT_textureCUBE:
			ret.func = SetD3D11ShaderParameter<TexturePtr, TexturePtr>(textures_[p_handle.shader_type][p_handle.offset], param);
			break;

		case REDT_sampler:
			ret.func = SetD3D11ShaderParameter<SamplerStateObjectPtr, SamplerStateObjectPtr>(samplers_[p_handle.shader_type][p_handle.offset], param);
			break;

		case REDT_buffer:
			ret.func = SetD3D11ShaderParameter<GraphicsBufferPtr, GraphicsBufferPtr>(buffers_[p_handle.shader_type][p_handle.offset], param);
			break;

		default:
			BOOST_ASSERT(false);
			break;
		}

		return ret;
	}

	void D3D11ShaderObject::Bind()
	{
		D3D11RenderEngine& re = *checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContextPtr const & d3d_imm_ctx = re.D3DDeviceImmContext();

		re.VSSetShader(vertex_shader_);
		re.GSSetShader(geometry_shader_);
		re.PSSetShader(pixel_shader_);

		for (size_t i = 0; i < ST_NumShaderTypes; ++ i)
		{
			BOOST_FOREACH(BOOST_TYPEOF(param_binds_[i])::reference pb, param_binds_[i])
			{
				pb.func();
			}
		}

		std::vector<ID3D11SamplerState*> sss;
		std::vector<ID3D11ShaderResourceView*> srs;

		BOOST_TYPEOF(textures_)::const_reference vs_textures = textures_[ST_VertexShader];
		BOOST_TYPEOF(buffers_)::const_reference vs_buffers = buffers_[ST_VertexShader];
		srs.resize(std::max(vs_textures.size(), vs_buffers.size()));
		for (size_t i = 0; i < srs.size(); ++ i)
		{
			if ((i < vs_textures.size()) && vs_textures[i])
			{
				srs[i] = checked_pointer_cast<D3D11Texture>(vs_textures[i])->D3DShaderResourceView().get();
			}
			else
			{
				if ((i < vs_buffers.size()) && vs_buffers[i])
				{
					srs[i] = checked_pointer_cast<D3D11GraphicsBuffer>(vs_buffers[i])->D3DShaderResourceView().get();
				}
				else
				{
					srs[i] = NULL;
				}
			}
		}
		if (!srs.empty())
		{
			d3d_imm_ctx->VSSetShaderResources(0, static_cast<UINT>(srs.size()), &srs[0]);
		}

		BOOST_TYPEOF(samplers_)::const_reference vs_samplers = samplers_[ST_VertexShader];
		sss.resize(vs_samplers.size());
		for (size_t i = 0; i < vs_samplers.size(); ++ i)
		{
			if (vs_samplers[i])
			{
				sss[i] = checked_pointer_cast<D3D11SamplerStateObject>(vs_samplers[i])->D3DSamplerState().get();
			}
			else
			{
				sss[i] = NULL;
			}
		}
		if (!sss.empty())
		{
			d3d_imm_ctx->VSSetSamplers(0, static_cast<UINT>(sss.size()), &sss[0]);
		}

		BOOST_TYPEOF(textures_)::const_reference ps_textures = textures_[ST_PixelShader];
		BOOST_TYPEOF(buffers_)::const_reference ps_buffers = buffers_[ST_PixelShader];
		srs.resize(std::max(ps_textures.size(), ps_buffers.size()));
		for (size_t i = 0; i < srs.size(); ++ i)
		{
			if ((i < ps_textures.size()) && ps_textures[i])
			{
				srs[i] = checked_pointer_cast<D3D11Texture>(ps_textures[i])->D3DShaderResourceView().get();
			}
			else
			{
				if ((i < ps_buffers.size()) && ps_buffers[i])
				{
					srs[i] = checked_pointer_cast<D3D11GraphicsBuffer>(ps_buffers[i])->D3DShaderResourceView().get();
				}
				else
				{
					srs[i] = NULL;
				}
			}
		}
		if (!srs.empty())
		{
			d3d_imm_ctx->PSSetShaderResources(0, static_cast<UINT>(srs.size()), &srs[0]);
		}

		BOOST_TYPEOF(samplers_)::const_reference ps_samplers = samplers_[ST_PixelShader];
		sss.resize(ps_samplers.size());
		for (size_t i = 0; i < ps_samplers.size(); ++ i)
		{
			if (ps_samplers[i])
			{
				sss[i] = checked_pointer_cast<D3D11SamplerStateObject>(ps_samplers[i])->D3DSamplerState().get();
			}
			else
			{
				sss[i] = NULL;
			}
		}
		if (!sss.empty())
		{
			d3d_imm_ctx->PSSetSamplers(0, static_cast<UINT>(sss.size()), &sss[0]);
		}

		BOOST_TYPEOF(textures_)::const_reference gs_textures = textures_[ST_GeometryShader];
		BOOST_TYPEOF(buffers_)::const_reference gs_buffers = buffers_[ST_GeometryShader];
		srs.resize(std::max(gs_textures.size(), gs_buffers.size()));
		for (size_t i = 0; i < srs.size(); ++ i)
		{
			if ((i < gs_textures.size()) && gs_textures[i])
			{
				srs[i] = checked_pointer_cast<D3D11Texture>(gs_textures[i])->D3DShaderResourceView().get();
			}
			else
			{
				if ((i < gs_buffers.size()) && gs_buffers[i])
				{
					srs[i] = checked_pointer_cast<D3D11GraphicsBuffer>(gs_buffers[i])->D3DShaderResourceView().get();
				}
				else
				{
					srs[i] = NULL;
				}
			}
		}
		if (!srs.empty())
		{
			d3d_imm_ctx->GSSetShaderResources(0, static_cast<UINT>(srs.size()), &srs[0]);
		}

		BOOST_TYPEOF(samplers_)::const_reference gs_samplers = samplers_[ST_GeometryShader];
		sss.resize(gs_samplers.size());
		for (size_t i = 0; i < gs_samplers.size(); ++ i)
		{
			if (gs_samplers[i])
			{
				sss[i] = checked_pointer_cast<D3D11SamplerStateObject>(gs_samplers[i])->D3DSamplerState().get();
			}
			else
			{
				sss[i] = NULL;
			}
		}
		if (!sss.empty())
		{
			d3d_imm_ctx->GSSetSamplers(0, static_cast<UINT>(sss.size()), &sss[0]);
		}

		for (size_t i = 0; i < d3d_cbufs_.size(); ++ i)
		{
			for (size_t j = 0; j < d3d_cbufs_[i].size(); ++ j)
			{
				if (dirty_[i][j])
				{
					d3d_imm_ctx->UpdateSubresource(d3d_cbufs_[i][j].get(), D3D11CalcSubresource(0, 0, 1), NULL, &cbufs_[i][j][0],
						static_cast<UINT>(cbufs_[i][j].size()), static_cast<UINT>(cbufs_[i][j].size()));

					dirty_[i][j] = false;
				}
			}
		}

		if (!d3d_cbufs_[ST_VertexShader].empty())
		{
			std::vector<ID3D11Buffer*> cb(d3d_cbufs_[ST_VertexShader].size());
			for (size_t i = 0; i < cb.size(); ++ i)
			{
				cb[i] = d3d_cbufs_[ST_VertexShader][i].get();
			}
			d3d_imm_ctx->VSSetConstantBuffers(0, static_cast<UINT>(cb.size()), &cb[0]);
		}
		if (!d3d_cbufs_[ST_PixelShader].empty())
		{
			std::vector<ID3D11Buffer*> cb(d3d_cbufs_[ST_PixelShader].size());
			for (size_t i = 0; i < cb.size(); ++ i)
			{
				cb[i] = d3d_cbufs_[ST_PixelShader][i].get();
			}
			d3d_imm_ctx->PSSetConstantBuffers(0, static_cast<UINT>(cb.size()), &cb[0]);
		}
		if (!d3d_cbufs_[ST_GeometryShader].empty())
		{
			std::vector<ID3D11Buffer*> cb(d3d_cbufs_[ST_GeometryShader].size());
			for (size_t i = 0; i < cb.size(); ++ i)
			{
				cb[i] = d3d_cbufs_[ST_GeometryShader][i].get();
			}
			d3d_imm_ctx->GSSetConstantBuffers(0, static_cast<UINT>(cb.size()), &cb[0]);
		}
	}

	void D3D11ShaderObject::Unbind()
	{
		D3D11RenderEngine& re = *checked_cast<D3D11RenderEngine*>(&Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		ID3D11DeviceContextPtr const & d3d_imm_ctx = re.D3DDeviceImmContext();

		std::vector<ID3D11ShaderResourceView*> srs(textures_[ST_VertexShader].size(), NULL);
		if (!srs.empty())
		{
			d3d_imm_ctx->VSSetShaderResources(0, static_cast<UINT>(srs.size()), &srs[0]);
		}
		srs.resize(textures_[ST_PixelShader].size(), NULL);
		if (!srs.empty())
		{
			d3d_imm_ctx->PSSetShaderResources(0, static_cast<UINT>(srs.size()), &srs[0]);
		}
		srs.resize(textures_[ST_GeometryShader].size(), NULL);
		if (!srs.empty())
		{
			d3d_imm_ctx->GSSetShaderResources(0, static_cast<UINT>(srs.size()), &srs[0]);
		}
	}
}