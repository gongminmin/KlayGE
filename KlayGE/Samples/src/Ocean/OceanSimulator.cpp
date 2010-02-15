#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/FrameBuffer.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/RenderEngine.hpp>

#include "OceanSimulator.hpp"

namespace
{
	using namespace KlayGE;

	float const HALF_SQRT_2 = 0.7071068f;
	float const GRAV_ACCEL = 9.8f;	// The acceleration of gravity, m/s^2

	int const BLOCK_SIZE_X = 16;
	int const BLOCK_SIZE_Y = 16;

	// Generating gaussian random number with mean 0 and standard deviation 1.
	float Gauss()
	{
		float u1 = rand() / static_cast<float>(RAND_MAX);
		float u2 = rand() / static_cast<float>(RAND_MAX);
		if (u1 < 1e-6f)
		{
			u1 = 1e-6f;
		}
		return sqrt(-2 * log(u1)) * cos(2 * PI * u2);
	}

	// Phillips Spectrum
	// K: normalized wave vector, W: wind direction, v: wind velocity, a: amplitude constant
	float Phillips(float2 K, float2 W, float v, float a, float dir_depend)
	{
		// largest possible wave from constant wind of velocity v
		float l = v * v / GRAV_ACCEL;
		// damp out waves with very small length w << l
		float w = l / 1000;

		float Ksqr = MathLib::dot(K, K);
		float Kcos = MathLib::dot(K, W);
		float phillips = a * exp(-1 / (l * l * Ksqr)) / (Ksqr * Ksqr * Ksqr) * (Kcos * Kcos);

		// filter out waves moving opposite to wind
		if (Kcos < 0)
		{
			phillips *= dir_depend;
		}

		// damp out waves with very small length w << l
		return phillips * exp(-Ksqr * w * w);
	}
}

namespace KlayGE
{
	OceanSimulator::OceanSimulator(OceanParameter& params)
		: param_(params)
	{
		param_.wind_dir = MathLib::normalize(param_.wind_dir);

		RenderFactory& rf = Context::Instance().RenderFactoryInstance();

		// Height map H(0)
		int height_map_size = (params.dmap_dim + 4) * (params.dmap_dim + 1);
		std::vector<float2> h0_data(height_map_size);
		std::vector<float> omega_data(height_map_size);
		this->InitHeightMap(h0_data, omega_data);

		ElementInitData init_data;

		// For filling the buffer with zeroes.
		std::vector<char> zero_data(3 * params.dmap_dim * params.dmap_dim * sizeof(float) * 2, 0);

		// RW buffer allocations
		// H0
		init_data.row_pitch = h0_data.size() * sizeof(h0_data[0]);
		init_data.data = &h0_data[0];
		h0_buffer_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Unordered | EAH_GPU_Structured, &init_data, EF_GR32F);

		// Notice: The following 3 should be half sized buffer due to conjugate symmetric input. But we use full
		// spectrum buffer due to the CS4.0 restriction.

		// Put H(t), Dx(t) and Dy(t) into one buffer
		ht_buffer_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Unordered | EAH_GPU_Structured, &init_data, EF_GR32F);
		ht_buffer_->Resize(3 * params.dmap_dim * params.dmap_dim * sizeof(float) * 2);

		// omega
		init_data.row_pitch = omega_data.size() * sizeof(omega_data[0]);
		init_data.data = &omega_data[0];
		omega_buffer_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Unordered | EAH_GPU_Structured, &init_data, EF_R32F);

		// Notice: The following 3 should be real number data. But here we use the complex numbers and C2C FFT
		// instead due to the CS4.0 restriction.
		// Put Dz, Dx and Dy into one buffer
		dxyz_buffer_ = rf.MakeVertexBuffer(BU_Dynamic, EAH_GPU_Read | EAH_GPU_Unordered | EAH_GPU_Structured, &init_data, EF_GR32F);
		dxyz_buffer_->Resize(3 * params.dmap_dim * params.dmap_dim * sizeof(float) * 2);

		displacement_tex_ = rf.MakeTexture2D(params.dmap_dim, params.dmap_dim, 1, 1, EF_ABGR32F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		gradient_tex_ = rf.MakeTexture2D(params.dmap_dim, params.dmap_dim, 0, 1, EF_ABGR16F, 1, 0, EAH_GPU_Read | EAH_GPU_Write, NULL);
		gradient_tex_->BuildMipSubLevels();

		displacement_fb_ = rf.MakeFrameBuffer();
		displacement_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*displacement_tex_, 0, 0));
		gradient_fb_ = rf.MakeFrameBuffer();
		gradient_fb_->Attach(FrameBuffer::ATT_Color0, rf.Make2DRenderView(*gradient_tex_, 0, 0));

		RenderEffectPtr effect_cs = rf.LoadEffect("OceanSimulatorCS.fxml");
		RenderEffectPtr effect_vsps = rf.LoadEffect("OceanSimulatorVSPS.fxml");
		update_spectrum_tech_ = effect_cs->TechniqueByName("UpdateSpectrum");
		update_displacement_tech_ = effect_vsps->TechniqueByName("UpdateDisplacement");
		gen_gradient_folding_tech_ = effect_vsps->TechniqueByName("GenGradientFolding");

		quad_layout_ = rf.MakeRenderLayout();
		quad_layout_->TopologyType(RenderLayout::TT_TriangleStrip);

		float3 xyzs[] =
		{
			float3(-1, +1, 0),
			float3(+1, +1, 0),
			float3(-1, -1, 0),
			float3(+1, -1, 0)
		};
		init_data.row_pitch = sizeof(xyzs);
		init_data.slice_pitch = 0;
		init_data.data = xyzs;
		quad_vb_ = rf.MakeVertexBuffer(BU_Static, EAH_GPU_Read, &init_data);
		quad_layout_->BindVertexStream(quad_vb_, boost::make_tuple(vertex_element(VEU_Position, 0, EF_BGR32F)));

		// Constant buffers
		uint32_t actual_dim = param_.dmap_dim;
		uint32_t input_width = actual_dim + 4;
		// We use full sized data here. The value "output_width" should be actual_dim/2+1 though.
		uint32_t output_width = actual_dim;
		uint32_t output_height = actual_dim;
		uint32_t dtx_offset = actual_dim * actual_dim;
		uint32_t dty_offset = actual_dim * actual_dim * 2;

		*(effect_cs->ParameterByName("actual_dim")) = actual_dim;
		*(effect_cs->ParameterByName("in_width")) = input_width;
		*(effect_cs->ParameterByName("out_width")) = output_width;
		*(effect_cs->ParameterByName("out_height")) = output_height;
		*(effect_cs->ParameterByName("dx_addr_offset")) = dtx_offset;
		*(effect_cs->ParameterByName("dy_addr_offset")) = dty_offset;
		*(effect_cs->ParameterByName("input_h0")) = h0_buffer_;
		*(effect_cs->ParameterByName("input_omega")) = omega_buffer_;

		time_param_ = effect_cs->ParameterByName("time");

		*(effect_vsps->ParameterByName("out_width")) = output_width;
		*(effect_vsps->ParameterByName("out_height")) = output_height;
		*(effect_vsps->ParameterByName("dx_addr_offset")) = dtx_offset;
		*(effect_vsps->ParameterByName("dy_addr_offset")) = dty_offset;
		*(effect_vsps->ParameterByName("choppy_scale")) = param_.choppy_scale;
		*(effect_vsps->ParameterByName("grid_len")) = param_.dmap_dim / param_.patch_length;
		*(effect_vsps->ParameterByName("input_dxyz")) = dxyz_buffer_;
		*(effect_vsps->ParameterByName("displacement_tex")) = displacement_tex_;

		fft_create_plan(&fft_plan_, params.dmap_dim, params.dmap_dim, 3);
	}

	OceanSimulator::~OceanSimulator()
	{
		fft_destroy_plan(&fft_plan_);
	}

	// Initialize the vector field.
	// wlen_x: width of wave tile, in meters
	// wlen_y: length of wave tile, in meters
	void OceanSimulator::InitHeightMap(std::vector<float2>& out_h0, std::vector<float>& out_omega)
	{
		float2 K;

		// initialize random generator.
		srand(0);

		for (int i = 0; i <= param_.dmap_dim; i++)
		{
			// K is wave-vector, range [-|DX/W, |DX/W], [-|DY/H, |DY/H]
			K.y() = (-param_.dmap_dim / 2.0f + i) * (2 * PI / param_.patch_length);

			for (int j = 0; j <= param_.dmap_dim; j++)
			{
				K.x() = (-param_.dmap_dim / 2.0f + j) * (2 * PI / param_.patch_length);

				float phil = ((K.x() == 0) && (K.y() == 0)) ? 0 : sqrt(Phillips(K, param_.wind_dir, param_.wind_speed, param_.wave_amplitude, param_.wind_dependency)) * HALF_SQRT_2;
				out_h0[i * (param_.dmap_dim + 4) + j] = phil * float2(Gauss(), Gauss());

				// The angular frequency is following the dispersion relation:
				//            out_omega^2 = g*k
				// So the equation of Gerstner wave is:
				//            x = x0 - K/k * A * sin(dot(K, x0) - sqrt(g * k) * t), x is a 2D vector.
				//            z = A * cos(dot(K, x0) - sqrt(g * k) * t)
				// Gerstner wave shows that a point on a simple sinusoid wave is doing a uniform circular
				// motion with the center at (x0, y0, z0), radius at A, and the circular plane is parallel
				// to vector K.
				out_omega[i * (param_.dmap_dim + 4) + j] = sqrt(GRAV_ACCEL * MathLib::length(K));
			}
		}
	}

	void OceanSimulator::Update(float time)
	{
		RenderEngine& re = Context::Instance().RenderFactoryInstance().RenderEngineInstance();

		// ---------------------------- H(0) -> H(t), D(x, t), D(y, t) --------------------------------

		re.BindUABuffers(std::vector<GraphicsBufferPtr>(1, ht_buffer_));

		*time_param_ = time * param_.time_scale;

		uint32_t group_count_x = (param_.dmap_dim + BLOCK_SIZE_X - 1) / BLOCK_SIZE_X;
		uint32_t group_count_y = (param_.dmap_dim + BLOCK_SIZE_Y - 1) / BLOCK_SIZE_Y;
		re.Dispatch(*update_spectrum_tech_, group_count_x, group_count_y, 1);

		re.BindUABuffers(std::vector<GraphicsBufferPtr>(0));


		// ------------------------------------ Perform FFT -------------------------------------------
		fft_c2c(&fft_plan_, dxyz_buffer_, ht_buffer_);

		// --------------------------------- Wrap Dx, Dy and Dz ---------------------------------------
		// Push RT
		FrameBufferPtr old_fb = re.CurFrameBuffer();
		re.BindFrameBuffer(displacement_fb_);

		re.Render(*update_displacement_tech_, *quad_layout_);


		// ----------------------------------- Generate Normal ----------------------------------------
		// Set RT
		re.BindFrameBuffer(gradient_fb_);

		re.Render(*gen_gradient_folding_tech_, *quad_layout_);

		// Pop RT
		re.BindFrameBuffer(old_fb);

		gradient_tex_->BuildMipSubLevels();
	}

	TexturePtr const & OceanSimulator::DisplacementTex() const
	{
		return displacement_tex_;
	}

	TexturePtr const & OceanSimulator::GradientTex() const
	{
		return gradient_tex_;
	}

	OceanParameter const & OceanSimulator::Parameters() const
	{
		return param_;
	}
}
