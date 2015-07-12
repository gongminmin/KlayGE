#include <KlayGE/KlayGE.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KFL/XMLDom.hpp>

#include <iostream>
#include <fstream>
#include <string>

using namespace KlayGE;
using namespace std;

namespace
{
	std::string fxml_name;
	std::string shader_text;
}

class FXML2ShaderApp : public KlayGE::App3DFramework
{
public:
	FXML2ShaderApp()
		: App3DFramework("FXML2Shader")
	{
	}

	void OnCreate()
	{
		RenderEffectPtr effect = SyncLoadRenderEffect(fxml_name);

		ofstream ofs((fxml_name + ".shader").c_str(), std::ios_base::binary);
		ofs << "#define KLAYGE_D3D11 1" << "\r\n";
		ofs << "#define KLAYGE_SHADER_MODEL (5 * 4 + 0)" << "\r\n";
		ofs << "#define KLAYGE_MAX_TEX_ARRAY_LEN 512" << "\r\n";
		ofs << "#define KLAYGE_MAX_TEX_DEPTH 512" << "\r\n";
		ofs << "#define KLAYGE_MAX_TEX_UNITS 32" << "\r\n";
		ofs << "#define KLAYGE_NO_TEX_LOD 0" << "\r\n";
		ofs << "#define KLAYGE_FLIPPING -1" << "\r\n";
		ofs << "#define KLAYGE_DERIVATIVES 1" << "\r\n";
		ofs << "\r\n";

		for (uint32_t i = 0; i < effect->NumMacros(); ++ i)
		{
			std::pair<std::string, std::string> const & name_value = effect->MacroByIndex(i);
			ofs << "#define " << name_value.first << " " << name_value.second << "\r\n";
		}
		ofs << "\r\n";

		for (uint32_t i = 0; i < effect->NumCBuffers(); ++ i)
		{
			RenderEffectConstantBufferPtr const & cbuff = effect->CBufferByIndex(i);
			ofs << "cbuffer " << *cbuff->Name() << std::endl;
			ofs << "{" << "\r\n";

			for (uint32_t j = 0; j < cbuff->NumParameters(); ++ j)
			{
				RenderEffectParameter& param = *effect->ParameterByIndex(cbuff->ParameterIndex(j));
				switch (param.Type())
				{
				case REDT_texture1D:
				case REDT_texture2D:
				case REDT_texture3D:
				case REDT_textureCUBE:
				case REDT_texture1DArray:
				case REDT_texture2DArray:
				case REDT_texture3DArray:
				case REDT_textureCUBEArray:
				case REDT_sampler:
				case REDT_buffer:
				case REDT_structured_buffer:
				case REDT_byte_address_buffer:
				case REDT_rw_buffer:
				case REDT_rw_structured_buffer:
				case REDT_rw_texture1D:
				case REDT_rw_texture2D:
				case REDT_rw_texture3D:
				case REDT_rw_texture1DArray:
				case REDT_rw_texture2DArray:
				case REDT_rw_byte_address_buffer:
				case REDT_append_structured_buffer:
				case REDT_consume_structured_buffer:
					break;

				default:
					ofs << "\t" << effect->TypeName(param.Type()) << " " << *param.Name();
					if (param.ArraySize())
					{
						ofs << "[" << *param.ArraySize() << "]";
					}
					ofs << ";" << "\r\n";
					break;
				}
			}

			ofs << "};" << "\r\n\r\n";
		}

		for (uint32_t i = 0; i < effect->NumParameters(); ++ i)
		{
			RenderEffectParameter& param = *(effect->ParameterByIndex(i));

			switch (param.Type())
			{
			case REDT_texture1D:
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ofs << "Texture1D<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_texture2D:
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ofs << "Texture2D<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_texture3D:
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ofs << "Texture3D<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_textureCUBE:
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ofs << "TextureCube<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_texture1DArray:
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ofs << "Texture1DArray<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_texture2DArray:
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ofs << "Texture2DArray<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_textureCUBEArray:
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ofs << "TextureCubeArray<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_buffer:
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ofs << "Buffer<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_sampler:
				ofs << "sampler " << *param.Name() << ";" << "\r\n";
				break;

			case REDT_structured_buffer:
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ofs << "StructuredBuffer<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_byte_address_buffer:
				{
					ofs << "ByteAddressBuffer " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_rw_buffer:
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ofs << "RWBuffer<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_rw_structured_buffer:
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ofs << "RWStructuredBuffer<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_rw_texture1D:
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ofs << "RWTexture1D<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_rw_texture2D:
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ofs << "RWTexture2D<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_rw_texture3D:
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ofs << "RWTexture3D<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;
			case REDT_rw_texture1DArray:
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ofs << "RWTexture1DArray<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_rw_texture2DArray:
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ofs << "RWTexture2DArray<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_rw_byte_address_buffer:
				{
					ofs << "RWByteAddressBuffer " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_append_structured_buffer:
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ofs << "AppendStructuredBuffer<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_consume_structured_buffer:
				{
					std::string elem_type;
					param.Var()->Value(elem_type);
					ofs << "ConsumeStructuredBuffer<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			default:
				break;
			}
		}

		for (uint32_t i = 0; i < effect->NumShaders(); ++ i)
		{
			RenderShaderFunc const & effect_shader = effect->ShaderByIndex(i);
			ofs << effect_shader.str() << "\r\n";
		}
	}

	void DoUpdateOverlay()
	{
	}

	uint32_t DoUpdate(uint32_t /*pass*/)
	{
		return URV_Finished;
	}
};

int main(int argc, char* argv[])
{
	using namespace KlayGE;

	if (argc < 2)
	{
		cout << "Usage: FXML2Shader xxx.fxml" << endl;
		return 1;
	}

	fxml_name = argv[1];

	Context::Instance().LoadCfg("KlayGE.cfg");
	ContextCfg context_cfg = Context::Instance().Config();
	context_cfg.render_factory_name = "D3D11";
	context_cfg.graphics_cfg.hide_win = true;
	context_cfg.graphics_cfg.hdr = false;
	Context::Instance().Config(context_cfg);

	FXML2ShaderApp app;
	app.Create();

	return 0;
}
