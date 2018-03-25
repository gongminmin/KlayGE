#include <KlayGE/KlayGE.hpp>
#include <KlayGE/App3D.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Texture.hpp>

#include "KlayGETests.hpp"

using namespace testing;

namespace KlayGE
{
	class KlayGETestsApp : public App3DFramework
	{
	public:
		KlayGETestsApp()
			: App3DFramework("KlayGETests")
		{
			ResLoader::Instance().AddPath("../../Tests/media");
		}

		virtual void DoUpdateOverlay() override
		{
		}

		virtual uint32_t DoUpdate(uint32_t pass) override
		{
			KFL_UNUSED(pass);
			return URV_Finished;
		}
	};

	class KlayGETestEnvironment : public testing::Environment
	{
	public:
		void SetUp() override
		{
			Context::Instance().LoadCfg("KlayGE.cfg");
			ContextCfg context_cfg = Context::Instance().Config();
			context_cfg.graphics_cfg.hide_win = true;
			context_cfg.graphics_cfg.hdr = false;
			context_cfg.graphics_cfg.color_grading = false;
			context_cfg.graphics_cfg.gamma = false;
			Context::Instance().Config(context_cfg);

			app_ = MakeSharedPtr<KlayGETestsApp>();
			app_->Create();
		}

		void TearDown() override
		{
			app_.reset();

			Context::Destroy();
		}

	private:
		std::shared_ptr<App3DFramework> app_;
	};

	bool CompareBuffer(GraphicsBuffer& buff0, uint32_t buff0_offset,
		GraphicsBuffer& buff1, uint32_t buff1_offset,
		uint32_t num_elems, float tolerance)
	{
		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		GraphicsBufferPtr buff0_cpu;
		GraphicsBufferPtr buff1_cpu;

		GraphicsBuffer* buff0_cpu_ptr;
		if (buff0.AccessHint() & EAH_CPU_Read)
		{
			buff0_cpu_ptr = &buff0;
		}
		else
		{
			buff0_cpu = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, buff0.Size(), nullptr);
			buff0_cpu_ptr = buff0_cpu.get();
			buff0.CopyToBuffer(*buff0_cpu_ptr);
		}

		GraphicsBuffer* buff1_cpu_ptr;
		if (buff1.AccessHint() & EAH_CPU_Read)
		{
			buff1_cpu_ptr = &buff1;
		}
		else
		{
			buff1_cpu = rf.MakeVertexBuffer(BU_Static, EAH_CPU_Read, buff1.Size(), nullptr);
			buff1_cpu_ptr = buff1_cpu.get();
			buff1.CopyToBuffer(*buff1_cpu_ptr);
		}

		bool match = true;
		{
			GraphicsBuffer::Mapper buff0_mapper(*buff0_cpu_ptr, BA_Read_Only);
			float const * buff0_p = reinterpret_cast<float const *>(buff0_mapper.Pointer<uint8_t>() + buff0_offset);

			GraphicsBuffer::Mapper buff1_mapper(*buff1_cpu_ptr, BA_Read_Only);
			float const * buff1_p = reinterpret_cast<float const *>(buff1_mapper.Pointer<uint8_t>() + buff1_offset);

			for (uint32_t i = 0; i < num_elems; ++ i)
			{
				if (abs(buff0_p[i] - buff1_p[i]) > tolerance)
				{
					match = false;
					break;
				}
			}
		}

		return match;
	}

	bool Compare2D(Texture& tex0, uint32_t tex0_array_index, uint32_t tex0_level, uint32_t tex0_x_offset, uint32_t tex0_y_offset,
		Texture& tex1, uint32_t tex1_array_index, uint32_t tex1_level, uint32_t tex1_x_offset, uint32_t tex1_y_offset,
		uint32_t width, uint32_t height, float tolerance)
	{
		BOOST_ASSERT(1 == tex0.SampleCount());
		BOOST_ASSERT(1 == tex1.SampleCount());

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		ElementFormat const tex0_fmt = tex0.Format();
		ElementFormat const tex1_fmt = tex1.Format();

		TexturePtr tex0_cpu = rf.MakeTexture2D(width, height, 1, 1, tex0_fmt, 1, 0, EAH_CPU_Read);
		tex0.CopyToSubTexture2D(*tex0_cpu, 0, 0, 0, 0, width, height,
			tex0_array_index, tex0_level, tex0_x_offset, tex0_y_offset, width, height);

		TexturePtr tex1_cpu = rf.MakeTexture2D(width, height, 1, 1, tex1_fmt, 1, 0, EAH_CPU_Read);
		tex1.CopyToSubTexture2D(*tex1_cpu, 0, 0, 0, 0, width, height,
			tex1_array_index, tex1_level, tex1_x_offset, tex1_y_offset, width, height);

		bool match = true;
		{
			uint32_t const tex0_elem_size = NumFormatBytes(tex0_fmt);
			uint32_t const tex1_elem_size = NumFormatBytes(tex1_fmt);

			Texture::Mapper tex0_mapper(*tex0_cpu, 0, 0, TMA_Read_Only, 0, 0, width, height);
			uint8_t const * tex0_p = tex0_mapper.Pointer<uint8_t>();
			uint32_t const tex0_row_pitch = tex0_mapper.RowPitch();

			Texture::Mapper tex1_mapper(*tex1_cpu, 0, 0, TMA_Read_Only, 0, 0, width, height);
			uint8_t const * tex1_p = tex1_mapper.Pointer<uint8_t>();
			uint32_t const tex1_row_pitch = tex1_mapper.RowPitch();

			for (uint32_t y = 0; (y < height) && match; ++ y)
			{
				for (uint32_t x = 0; x < width; ++ x)
				{
					Color tex0_clr;
					ConvertToABGR32F(tex0_fmt, tex0_p + y * tex0_row_pitch + x * tex0_elem_size, 1, &tex0_clr);

					Color tex1_clr;
					ConvertToABGR32F(tex1_fmt, tex1_p + y * tex1_row_pitch + x * tex1_elem_size, 1, &tex1_clr);

					if ((abs(tex0_clr.r() - tex1_clr.r()) > tolerance) || (abs(tex0_clr.g() - tex1_clr.g()) > tolerance)
						|| (abs(tex0_clr.b() - tex1_clr.b()) > tolerance) || (abs(tex0_clr.a() - tex1_clr.a()) > tolerance))
					{
						match = false;
						break;
					}
				}
			}
		}
		if (!match)
		{
			auto const * test_info = testing::UnitTest::GetInstance()->current_test_info();
			auto test_name = std::string(test_info->test_case_name()) + '_' + test_info->name();

			SaveTexture(tex0_cpu, test_name + "_tex0.dds");
			SaveTexture(tex1_cpu, test_name + "_tex1.dds");
		}

		return match;
	}
}

int main(int argc, char** argv)
{
	InitGoogleTest(&argc, argv);
	AddGlobalTestEnvironment(new KlayGE::KlayGETestEnvironment);

	int ret_val = RUN_ALL_TESTS();
	if (ret_val != 0)
	{
		getchar();
	}

	return ret_val;
}
