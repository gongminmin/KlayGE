#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/RenderSettings.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/XMLDom.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

using namespace KlayGE;
using namespace std;

namespace
{
	std::string fxml_name;
	std::string shader_text;
}

class EmptyApp : public KlayGE::App3DFramework
{
public:
	EmptyApp()
		: App3DFramework("FXML2Shader")
	{
		ResLoader::Instance().AddPath("../../../media/RenderFX");
	}

	void InitObjects()
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		RenderEffectPtr effect = rf.LoadEffect(fxml_name);

		ofstream ofs((fxml_name + ".shader").c_str(), std::ios_base::binary);
		ofs << "#define CONSTANT_BUFFER" << "\r\n";
		ofs << "#define KLAYGE_D3D11" << "\r\n";
		ofs << "#define KLAYGE_SHADER_MODEL 5" << "\r\n";
		ofs << "#define KLAYGE_MAX_TEX_ARRAY_LEN 512" << "\r\n\r\n";

		for (uint32_t i = 0; i < effect->NumMacros(); ++ i)
		{
			std::pair<std::string, std::string> const & name_value = effect->MacroByIndex(i);
			ofs << "#define " << name_value.first << " " << name_value.second << "\r\n";
		}
		ofs << "\r\n";

		BOOST_AUTO(cbuffers, effect->CBuffers());
		BOOST_FOREACH(BOOST_TYPEOF(cbuffers)::const_reference cbuff, cbuffers)
		{
			ofs << "cbuffer " << cbuff.first << "\r\n";
			ofs << "{" << "\r\n";

			BOOST_FOREACH(BOOST_TYPEOF(cbuff.second)::const_reference param_index, cbuff.second)
			{
				RenderEffectParameter& param = *(effect->ParameterByIndex(param_index));
				switch (param.type())
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
					ofs << "\t" << effect->TypeName(param.type()) << " " << *param.Name();
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

			switch (param.type())
			{
			case REDT_texture1D:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ofs << "Texture1D<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_texture2D:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ofs << "Texture2D<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_texture3D:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ofs << "Texture3D<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_textureCUBE:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ofs << "TextureCube<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_texture1DArray:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ofs << "Texture1DArray<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_texture2DArray:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ofs << "Texture2DArray<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_textureCUBEArray:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ofs << "TextureCubeArray<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_buffer:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ofs << "Buffer<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_sampler:
				ofs << "sampler " << *param.Name() << ";" << "\r\n";
				break;

			case REDT_structured_buffer:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
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
					param.var()->Value(elem_type);
					ofs << "RWBuffer<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_rw_structured_buffer:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ofs << "RWStructuredBuffer<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_rw_texture1D:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ofs << "RWTexture1D<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_rw_texture2D:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ofs << "RWTexture2D<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_rw_texture3D:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ofs << "RWTexture3D<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;
			case REDT_rw_texture1DArray:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
					ofs << "RWTexture1DArray<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_rw_texture2DArray:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
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
					param.var()->Value(elem_type);
					ofs << "AppendStructuredBuffer<" << elem_type << "> " << *param.Name() << ";" << "\r\n";
				}
				break;

			case REDT_consume_structured_buffer:
				{
					std::string elem_type;
					param.var()->Value(elem_type);
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

	ResLoader::Instance().AddPath("../../../bin");

	Context::Instance().LoadCfg("KlayGE.cfg");

	EmptyApp app;
	app.Create();

	return 0;
}
