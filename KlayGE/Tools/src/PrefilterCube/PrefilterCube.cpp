#include <KlayGE/KlayGE.hpp>
#include <KFL/Half.hpp>
#include <KFL/Math.hpp>
#include <KFL/Timer.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/PostProcess.hpp>
#include <KlayGE/App3D.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#if defined(KLAYGE_TS_LIBRARY_FILESYSTEM_V3_SUPPORT)
	#include <experimental/filesystem>
#elif defined(KLAYGE_TS_LIBRARY_FILESYSTEM_V2_SUPPORT)
	#include <filesystem>
	namespace std
	{
		namespace experimental
		{
			namespace filesystem = std::tr2::sys;
		}
	}
#else
	#if defined(KLAYGE_COMPILER_GCC)
		#pragma GCC diagnostic push
		#pragma GCC diagnostic ignored "-Wdeprecated-declarations" // Ignore auto_ptr declaration
	#endif
	#include <boost/filesystem.hpp>
	#if defined(KLAYGE_COMPILER_GCC)
		#pragma GCC diagnostic pop
	#endif
	namespace std
	{
		namespace experimental
		{
			namespace filesystem = boost::filesystem;
		}
	}
#endif
#include <boost/assert.hpp>

using namespace std;
using namespace KlayGE;
using namespace std::experimental;

namespace
{
	float3 ToDir(uint32_t face, uint32_t x, uint32_t y, uint32_t size)
	{
		float3 dir;
		switch (face)
		{
		case Texture::CF_Positive_X:
			dir.x() = +1;
			dir.y() = (size - 1 - y + 0.5f) / size * 2 - 1;
			dir.z() = (size - 1 - x + 0.5f) / size * 2 - 1;
			break;

		case Texture::CF_Negative_X:
			dir.x() = -1;
			dir.y() = (size - 1 - y + 0.5f) / size * 2 - 1;
			dir.z() = (x + 0.5f) / size * 2 - 1;
			break;

		case Texture::CF_Positive_Y:
			dir.x() = (x + 0.5f) / size * 2 - 1;
			dir.y() = +1;
			dir.z() = (y + 0.5f) / size * 2 - 1;
			break;

		case Texture::CF_Negative_Y:
			dir.x() = (x + 0.5f) / size * 2 - 1;
			dir.y() = -1;
			dir.z() = (size - 1 - y + 0.5f) / size * 2 - 1;
			break;

		case Texture::CF_Positive_Z:
			dir.x() = (x + 0.5f) / size * 2 - 1;
			dir.y() = (size - 1 - y + 0.5f) / size * 2 - 1;
			dir.z() = +1;
			break;

		case Texture::CF_Negative_Z:
		default:
			dir.x() = (size - 1 - x + 0.5f) / size * 2 - 1;
			dir.y() = (size - 1 - y + 0.5f) / size * 2 - 1;
			dir.z() = -1;
			break;
		}

		return MathLib::normalize(dir);
	}

	void ToAddress(uint32_t& face, uint32_t& x, uint32_t& y, float3 const & dir, uint32_t size)
	{
		float3 n_dir = MathLib::normalize(dir);
		float3 abs_dir = MathLib::abs(n_dir);
		if (abs_dir.x() > abs_dir.y())
		{
			if (abs_dir.x() > abs_dir.z())
			{
				face = n_dir.x() > 0 ? Texture::CF_Positive_X : Texture::CF_Negative_X;
			}
			else
			{
				face = n_dir.z() > 0 ? Texture::CF_Positive_Z : Texture::CF_Negative_Z;
			}
		}
		else
		{
			if (abs_dir.y() > abs_dir.z())
			{
				face = n_dir.y() > 0 ? Texture::CF_Positive_Y : Texture::CF_Negative_Y;
			}
			else
			{
				face = n_dir.z() > 0 ? Texture::CF_Positive_Z : Texture::CF_Negative_Z;
			}
		}

		switch (face)
		{
		case Texture::CF_Positive_X:
			n_dir /= abs_dir.x();
			x = MathLib::clamp(static_cast<uint32_t>((-n_dir.z() * 0.5f + 0.5f) * size), 0U, size - 1);
			y = MathLib::clamp(static_cast<uint32_t>((-n_dir.y() * 0.5f + 0.5f) * size), 0U, size - 1);
			break;

		case Texture::CF_Negative_X:
			n_dir /= abs_dir.x();
			x = MathLib::clamp(static_cast<uint32_t>((+n_dir.z() * 0.5f + 0.5f) * size), 0U, size - 1);
			y = MathLib::clamp(static_cast<uint32_t>((-n_dir.y() * 0.5f + 0.5f) * size), 0U, size - 1);
			break;

		case Texture::CF_Positive_Y:
			n_dir /= abs_dir.y();
			x = MathLib::clamp(static_cast<uint32_t>((+n_dir.x() * 0.5f + 0.5f) * size), 0U, size - 1);
			y = MathLib::clamp(static_cast<uint32_t>((+n_dir.z() * 0.5f + 0.5f) * size), 0U, size - 1);
			break;

		case Texture::CF_Negative_Y:
			n_dir /= abs_dir.y();
			x = MathLib::clamp(static_cast<uint32_t>((+n_dir.x() * 0.5f + 0.5f) * size), 0U, size - 1);
			y = MathLib::clamp(static_cast<uint32_t>((-n_dir.z() * 0.5f + 0.5f) * size), 0U, size - 1);
			break;

		case Texture::CF_Positive_Z:
			n_dir /= abs_dir.z();
			x = MathLib::clamp(static_cast<uint32_t>((+n_dir.x() * 0.5f + 0.5f) * size), 0U, size - 1);
			y = MathLib::clamp(static_cast<uint32_t>((-n_dir.y() * 0.5f + 0.5f) * size), 0U, size - 1);
			break;

		case Texture::CF_Negative_Z:
		default:
			n_dir /= abs_dir.z();
			x = MathLib::clamp(static_cast<uint32_t>((-n_dir.x() * 0.5f + 0.5f) * size), 0U, size - 1);
			y = MathLib::clamp(static_cast<uint32_t>((-n_dir.y() * 0.5f + 0.5f) * size), 0U, size - 1);
			break;
		}
	}

	uint32_t ReverseBits(uint32_t bits)
	{
		bits = (bits << 16) | (bits >> 16);
		bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1);
		bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2);
		bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4);
		bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8);
		return bits;
	}

	float RadicalInverseVdC(uint32_t bits)
	{
		return ReverseBits(bits) * 2.3283064365386963e-10f; // / 0x100000000
	}

	float2 Hammersley2D(uint32_t i, uint32_t N)
	{
		return float2(static_cast<float>(i) / N, RadicalInverseVdC(i));
	}

	float3 ImportanceSampleLambert(float2 const & xi)
	{
		float phi = 2 * PI * xi.x();
		float cos_theta = sqrt(1 - xi.y());
		float sin_theta = sqrt(1 - cos_theta * cos_theta);
		return float3(sin_theta * cos(phi), sin_theta * sin(phi), cos_theta);
	}

	float3 ImportanceSampleLambert(float2 const & xi, float3 const & normal)
	{
		float3 h = ImportanceSampleLambert(xi);

		float3 up_vec = abs(normal.z()) < 0.999f ? float3(0, 0, 1) : float3(1, 0, 0);
		float3 tangent = MathLib::normalize(MathLib::cross(up_vec, normal));
		float3 binormal = MathLib::cross(normal, tangent);
		return tangent * h.x() + binormal * h.y() + normal * h.z();
	}

	float3 ImportanceSampleBP(float2 const & xi, float roughness)
	{
		float phi = 2 * PI * xi.x();
		float cos_theta = pow(1 - xi.y() * (roughness + 1) / (roughness + 2), 1 / (roughness + 1));
		float sin_theta = sqrt(1 - cos_theta * cos_theta);
		return float3(sin_theta * cos(phi), sin_theta * sin(phi), cos_theta);
	}

	float3 ImportanceSampleBP(float2 const & xi, float roughness, float3 const & normal)
	{
		float3 h = ImportanceSampleBP(xi, roughness);

		float3 up_vec = abs(normal.z()) < 0.999f ? float3(0, 0, 1) : float3(1, 0, 0);
		float3 tangent = MathLib::normalize(MathLib::cross(up_vec, normal));
		float3 binormal = MathLib::cross(normal, tangent);
		return tangent * h.x() + binormal * h.y() + normal * h.z();
	}

	Color PrefilterEnvMapDiffuse(float3 const & normal, Color* env_map[6], uint32_t size)
	{
		Color prefiltered_clr(0.0f, 0.0f, 0.0f, 0.0f);

		uint32_t const NUM_SAMPLES = 1024;
		for (uint32_t i = 0; i < NUM_SAMPLES; ++ i)
		{
			float2 xi = Hammersley2D(i, NUM_SAMPLES);
			float3 l = ImportanceSampleLambert(xi, normal);
			uint32_t face, x, y;
			ToAddress(face, x, y, l, size);
			prefiltered_clr += env_map[face][y * size + x];
		}

		return prefiltered_clr / static_cast<float>(NUM_SAMPLES);
	}

	Color PrefilterEnvMapSpecular(float roughness, float3 const & r, Color* env_map[6], uint32_t size)
	{
		float3 normal = r;
		float3 view = r;
		Color prefiltered_clr(0.0f, 0.0f, 0.0f, 0.0f);
		float total_weight = 0;

		uint32_t const NUM_SAMPLES = 1024;
		for (uint32_t i = 0; i < NUM_SAMPLES; ++ i)
		{
			float2 xi = Hammersley2D(i, NUM_SAMPLES);
			float3 h = ImportanceSampleBP(xi, roughness, normal);
			float3 l = -MathLib::reflect(view, h);
			float n_dot_l = MathLib::clamp(MathLib::dot(normal, l), 0.0f, 1.0f);
			if (n_dot_l > 0)
			{
				uint32_t face, x, y;
				ToAddress(face, x, y, l, size);
				prefiltered_clr += env_map[face][y * size + x] * n_dot_l;
				total_weight += n_dot_l;
			}
		}

		return prefiltered_clr / max(1e-6f, total_weight);
	}

	struct PrefilterCubeFaceParam
	{
		std::vector<std::vector<Color>>* prefilted_data;
		std::vector<ElementInitData>* out_data;
		std::vector<std::vector<half>>* out_data_block;
		uint32_t width;
		uint32_t num_mipmaps;
		atomic<uint32_t>* processed_texels;
	};

	void PrefilterCubeFace(PrefilterCubeFaceParam& param, uint32_t face)
	{
		std::vector<std::vector<Color>>& prefilted_data = *param.prefilted_data;
		std::vector<ElementInitData>& out_data = *param.out_data;
		std::vector<std::vector<half>>& out_data_block = *param.out_data_block;
		uint32_t width = param.width;
		uint32_t num_mipmaps = param.num_mipmaps;
		atomic<uint32_t>& processed_texels = *param.processed_texels;

		Color* env_map[6];
		for (uint32_t f = 0; f < 6; ++ f)
		{
			env_map[f] = &prefilted_data[f * num_mipmaps][0];
		}

		uint32_t w = std::max<uint32_t>(1U, width / 2);

		for (uint32_t mip = 1; mip < num_mipmaps - 1; ++ mip)
		{
			prefilted_data[face * num_mipmaps + mip].resize(w * w);
			float roughness = static_cast<float>(num_mipmaps - 2 - mip) / (num_mipmaps - 2);
			roughness = pow(8192.0f, roughness);

			for (uint32_t y = 0; y < w; ++ y)
			{
				for (uint32_t x = 0; x < w; ++ x)
				{
					prefilted_data[face * num_mipmaps + mip][y * w + x]
						= PrefilterEnvMapSpecular(roughness, ToDir(face, x, y, w), env_map, width);
					++ processed_texels;
				}
			}

			w = std::max<uint32_t>(1U, w / 2);
		}

		{
			prefilted_data[face * num_mipmaps + num_mipmaps - 1].resize(w * w);
			for (uint32_t y = 0; y < w; ++ y)
			{
				for (uint32_t x = 0; x < w; ++ x)
				{
					prefilted_data[face * num_mipmaps + num_mipmaps - 1][y * w + x]
						= PrefilterEnvMapDiffuse(ToDir(face, x, y, w), env_map, width);
					++ processed_texels;
				}
			}
		}

		w = width;
		for (uint32_t mip = 0; mip < num_mipmaps; ++ mip)
		{
			out_data_block[face * num_mipmaps + mip].resize(w * w * sizeof(half) * 4);
			out_data[face * num_mipmaps + mip].data = &out_data_block[face * num_mipmaps + mip][0];
			out_data[face * num_mipmaps + mip].row_pitch = w * sizeof(half) * 4;
			out_data[face * num_mipmaps + mip].slice_pitch = w * out_data[face * num_mipmaps + mip].row_pitch;

			ConvertFromABGR32F(EF_ABGR16F, &prefilted_data[face * num_mipmaps + mip][0], w * w,
				&out_data_block[face * num_mipmaps + mip][0]);

			w = std::max<uint32_t>(1U, w / 2);
		}
	}

	void PrefilterCube(std::string const & in_file, std::string const & out_file)
	{
		Texture::TextureType in_type;
		uint32_t in_width, in_height, in_depth;
		uint32_t in_num_mipmaps;
		uint32_t in_array_size;
		ElementFormat in_format;
		std::vector<ElementInitData> in_data;
		std::vector<uint8_t> in_data_block;
		LoadTexture(in_file, in_type, in_width, in_height, in_depth, in_num_mipmaps, in_array_size, in_format, in_data, in_data_block);

		uint32_t total_texels = 0;
		uint32_t out_num_mipmaps = 1;
		{
			uint32_t w = in_width;
			while (w > 8)
			{
				++ out_num_mipmaps;

				w = std::max<uint32_t>(1U, w / 2);
				total_texels += w * w * 6;
			}
		}

		uint32_t w = in_width;
		std::vector<std::vector<Color>> prefilted_data(out_num_mipmaps * 6);
		for (uint32_t face = 0; face < 6; ++ face)
		{
			prefilted_data[face * out_num_mipmaps].resize(w * w);
			
			uint8_t const * src = static_cast<uint8_t const *>(in_data[face * in_num_mipmaps].data);
			for (uint32_t y = 0; y < w; ++ y)
			{
				ConvertToABGR32F(in_format, src, w, &prefilted_data[face * out_num_mipmaps][y * w]);
				src += in_data[face * in_num_mipmaps].row_pitch;
			}
		}

		std::vector<ElementInitData> out_data(out_num_mipmaps * 6);
		std::vector<std::vector<half>> out_data_block(out_num_mipmaps * 6);

		atomic<uint32_t> processed_texels(0);

		thread_pool tp(1, 6);
		std::vector<joiner<void>> joiners(6);
		PrefilterCubeFaceParam param;
		param.prefilted_data = &prefilted_data;
		param.out_data = &out_data;
		param.out_data_block = &out_data_block;
		param.width = in_width;
		param.num_mipmaps = out_num_mipmaps;
		param.processed_texels = &processed_texels;
		for (size_t face = 0; face < joiners.size(); ++ face)
		{
			joiners[face] = tp(std::bind(PrefilterCubeFace, std::ref(param), static_cast<uint32_t>(face)));
		}

		for (;;)
		{
			cout << '\r';
			cout.precision(2);
			cout << "Processing " << fixed << processed_texels / static_cast<float>(total_texels) * 100 << " %     ";
			if (processed_texels == total_texels)
			{
				break;
			}

			KlayGE::Sleep(1000);
		}
		cout << endl;

		for (size_t face = 0; face < joiners.size(); ++ face)
		{
			joiners[face]();
		}

		SaveTexture(out_file, in_type, in_width, in_height, in_depth, out_num_mipmaps, in_array_size, EF_ABGR16F, out_data);
	}

	void PrefilterCubeGPU(std::string const & in_file, std::string const & out_file)
	{
		TexturePtr in_tex = SyncLoadTexture(in_file, EAH_GPU_Read | EAH_Immutable);
		uint32_t in_width = in_tex->Width(0);

		uint32_t out_num_mipmaps = 1;
		{
			uint32_t w = in_width;
			while (w > 8)
			{
				++ out_num_mipmaps;

				w = std::max<uint32_t>(1U, w / 2);
			}
		}

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		PostProcessPtr diff_pp = SyncLoadPostProcess("PrefilterCube.ppml", "PrefilterCubeDiffuse");
		PostProcessPtr spec_pp = SyncLoadPostProcess("PrefilterCube.ppml", "PrefilterCubeSpecular");
		diff_pp->InputPin(0, in_tex);
		spec_pp->InputPin(0, in_tex);

		TexturePtr out_tex = rf.MakeTextureCube(in_width, out_num_mipmaps, 1, EF_ABGR16F, 1, 0, EAH_GPU_Write, nullptr);

		for (int face = 0; face < 6; ++ face)
		{
			in_tex->CopyToSubTextureCube(*out_tex, 0, static_cast<Texture::CubeFaces>(face), 0, 0, 0, in_width, in_width,
				0, static_cast<Texture::CubeFaces>(face), 0, 0, 0, in_width, in_width);

			for (uint32_t level = 1; level < out_num_mipmaps - 1; ++ level)
			{
				float roughness = static_cast<float>(out_num_mipmaps - 2 - level) / (out_num_mipmaps - 2);
				roughness = pow(8192.0f, roughness);

				spec_pp->OutputPin(0, out_tex, level, 0, face);
				spec_pp->SetParam(0, face);
				spec_pp->SetParam(1, roughness);
				spec_pp->Apply();
			}

			{
				diff_pp->OutputPin(0, out_tex, out_num_mipmaps - 1, 0, face);
				diff_pp->SetParam(0, face);
				diff_pp->Apply();
			}
		}

		SaveTexture(out_tex, out_file);
	}
}

class PrefilterCubeApp : public KlayGE::App3DFramework
{
public:
	PrefilterCubeApp()
		: App3DFramework("PrefilterCube")
	{
		ResLoader::Instance().AddPath("../../Tools/media/PrefilterCube");
	}

	virtual void DoUpdateOverlay() override
	{
	}

	virtual uint32_t DoUpdate(uint32_t /*pass*/) override
	{
		return URV_Finished;
	}
};

int main(int argc, char* argv[])
{
	Context::Instance().LoadCfg("KlayGE.cfg");
	ContextCfg context_cfg = Context::Instance().Config();
	context_cfg.graphics_cfg.hide_win = true;
	context_cfg.graphics_cfg.hdr = false;
	context_cfg.graphics_cfg.color_grading = false;
	context_cfg.graphics_cfg.gamma = false;
	Context::Instance().Config(context_cfg);

	PrefilterCubeApp app;
	app.Create();

	using namespace KlayGE;

	if (argc < 2)
	{
		cout << "Usage: PrefilterCube xxx.dds [xxx_filtered.dds]" << endl;
		return 1;
	}

	std::string input(argv[1]);
	std::string output;
	if (argc >= 3)
	{
		output = argv[2];
	}
	else
	{
		filesystem::path output_path(argv[1]);
#ifdef KLAYGE_TS_LIBRARY_FILESYSTEM_V2_SUPPORT
		output = output_path.stem() + "_filtered.dds";
#else
		output = output_path.stem().string() + "_filtered.dds";
#endif
	}

	Timer timer;

	RenderDeviceCaps const & caps = Context::Instance().RenderFactoryInstance().RenderEngineInstance().DeviceCaps();
	if (caps.max_shader_model < ShaderModel(4, 0))
	{
		PrefilterCube(input, output);
	}
	else
	{
		PrefilterCubeGPU(input, output);
	}

	cout << timer.elapsed() << " s" << endl;
	cout << "Filtered cube map is saved into " << output << endl;

	return 0;
}
