// RenderStateObject.hpp
// KlayGE ��Ⱦ״̬������ ͷ�ļ�
// Ver 3.8.0
// ��Ȩ����(C) ������, 2006-2008
// Homepage: http://www.klayge.org
//
// 3.8.0
// ������SamplerStateObject (2008.9.21)
//
// 3.7.0
// ��RenderStateObject��������� (2008.6.29)
//
// 3.5.0
// ���ν��� (2006.11.1)
//
// �޸ļ�¼
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERSTATEOBJECT_HPP
#define _RENDERSTATEOBJECT_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>

#include <array>

#include <KFL/Color.hpp>

namespace KlayGE
{
	inline uint32_t
	float_to_uint32(float v)
	{
		union FNU
		{
			float f;
			uint32_t u;
		} fnu;
		fnu.f = v;
		return fnu.u;
	}
	inline float
	uint32_to_float(uint32_t v)
	{
		union FNU
		{
			uint32_t u;
			float f;
		} unf;
		unf.u = v;
		return unf.f;
	}

	enum PolygonMode
	{
		PM_Point,
		PM_Line,
		PM_Fill
	};

	enum ShadeMode
	{
		SM_Flat,
		SM_Gouraud
	};

	enum CullMode
	{
		CM_None,
		CM_Front,
		CM_Back
	};

	enum BlendOperation
	{
		BOP_Add		= 1,
		BOP_Sub		= 2,
		BOP_Rev_Sub	= 3,
		BOP_Min		= 4,
		BOP_Max		= 5,
	};

	enum AlphaBlendFactor
	{
		ABF_Zero,
		ABF_One,
		ABF_Src_Alpha,
		ABF_Dst_Alpha,
		ABF_Inv_Src_Alpha,
		ABF_Inv_Dst_Alpha,
		ABF_Src_Color,
		ABF_Dst_Color,
		ABF_Inv_Src_Color,
		ABF_Inv_Dst_Color,
		ABF_Src_Alpha_Sat,
		ABF_Blend_Factor,
		ABF_Inv_Blend_Factor,
		ABF_Src1_Alpha,
		ABF_Inv_Src1_Alpha,
		ABF_Src1_Color,
		ABF_Inv_Src1_Color
	};

	enum CompareFunction
	{
		CF_AlwaysFail,
		CF_AlwaysPass,
		CF_Less,
		CF_LessEqual,
		CF_Equal,
		CF_NotEqual,
		CF_GreaterEqual,
		CF_Greater
	};

	// Enum describing the various actions which can be taken onthe stencil buffer
	enum StencilOperation
	{
		// Leave the stencil buffer unchanged
		SOP_Keep,
		// Set the stencil value to zero
		SOP_Zero,
		// Set the stencil value to the reference value
		SOP_Replace,
		// Increase the stencil value by 1, clamping at the maximum value
		SOP_Incr,
		// Decrease the stencil value by 1, clamping at 0
		SOP_Decr,
		// Invert the bits of the stencil buffer
		SOP_Invert,
		// Increase the stencil value by 1, wrap the result if necessary
		SOP_Incr_Wrap,
		// Decrease the stencil value by 1, wrap the result if necessary
		SOP_Decr_Wrap
	};

	enum ColorMask
	{
		CMASK_Red   = 1UL << 0,
		CMASK_Green = 1UL << 1,
		CMASK_Blue  = 1UL << 2,
		CMASK_Alpha = 1UL << 3,
		CMASK_All   = CMASK_Red | CMASK_Green | CMASK_Blue | CMASK_Alpha
	};

	// Sampler addressing modes - default is TAM_Wrap.
	enum TexAddressingMode
	{
		// Texture wraps at values over 1.0
		TAM_Wrap,
		// Texture mirrors (flips) at joins over 1.0
		TAM_Mirror,
		// Texture clamps at 1.0
		TAM_Clamp,
		// Texture coordinates outside the range [0.0, 1.0] are set to the border color.
		TAM_Border
	};

	enum TexFilterOp
	{
		// Dont' use these enum directly
		TFOE_Mip_Point = 0x0,
		TFOE_Mip_Linear = 0x1,
		TFOE_Mag_Point = 0x0,
		TFOE_Mag_Linear = 0x2,
		TFOE_Min_Point = 0x0,
		TFOE_Min_Linear = 0x4,
		TFOE_Anisotropic = 0x08,
		TFOE_Comparison = 0x10,

		// Use these
		TFO_Min_Mag_Mip_Point				= TFOE_Min_Point  | TFOE_Mag_Point  | TFOE_Mip_Point,
		TFO_Min_Mag_Point_Mip_Linear		= TFOE_Min_Point  | TFOE_Mag_Point  | TFOE_Mip_Linear,
		TFO_Min_Point_Mag_Linear_Mip_Point	= TFOE_Min_Point  | TFOE_Mag_Linear | TFOE_Mip_Point,
		TFO_Min_Point_Mag_Mip_Linear		= TFOE_Min_Point  | TFOE_Mag_Linear | TFOE_Mip_Linear,
		TFO_Min_Linear_Mag_Mip_Point		= TFOE_Min_Linear | TFOE_Mag_Point  | TFOE_Mip_Point,
		TFO_Min_Linear_Mag_Point_Mip_Linear	= TFOE_Min_Linear | TFOE_Mag_Point  | TFOE_Mip_Linear,
		TFO_Min_Mag_Linear_Mip_Point		= TFOE_Min_Linear | TFOE_Mag_Linear | TFOE_Mip_Point,
		TFO_Min_Mag_Mip_Linear				= TFOE_Min_Linear | TFOE_Mag_Linear | TFOE_Mip_Linear,
		TFO_Anisotropic						= TFOE_Anisotropic,

		TFO_Cmp_Min_Mag_Mip_Point				= TFOE_Comparison | TFO_Min_Mag_Mip_Point,
		TFO_Cmp_Min_Mag_Point_Mip_Linear		= TFOE_Comparison | TFO_Min_Mag_Point_Mip_Linear,
		TFO_Cmp_Min_Point_Mag_Linear_Mip_Point	= TFOE_Comparison | TFO_Min_Point_Mag_Linear_Mip_Point,
		TFO_Cmp_Min_Point_Mag_Mip_Linear		= TFOE_Comparison | TFO_Min_Point_Mag_Mip_Linear,
		TFO_Cmp_Min_Linear_Mag_Mip_Point		= TFOE_Comparison | TFO_Min_Linear_Mag_Mip_Point,
		TFO_Cmp_Min_Linear_Mag_Point_Mip_Linear	= TFOE_Comparison | TFO_Min_Linear_Mag_Point_Mip_Linear,
		TFO_Cmp_Min_Mag_Linear_Mip_Point		= TFOE_Comparison | TFO_Min_Mag_Linear_Mip_Point,
		TFO_Cmp_Min_Mag_Mip_Linear				= TFOE_Comparison | TFO_Min_Mag_Mip_Linear,
		TFO_Cmp_Anisotropic						= TFOE_Comparison | TFO_Anisotropic
	};
	
	enum LogicOperation
	{
		LOP_Clear,
		LOP_Set,
		LOP_Copy,
		LOP_CopyInverted,
		LOP_Noop,
		LOP_Invert,
		LOP_And,
		LOP_NAnd,
		LOP_Or,
		LOP_NOR,
		LOP_XOR,
		LOP_Equiv,
		LOP_AndReverse,
		LOP_AndInverted,
		LOP_OrReverse,
		LOP_OrInverted
	};

#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(push, 1)
#endif
	struct KLAYGE_CORE_API RasterizerStateDesc
	{
		PolygonMode			polygon_mode;
		ShadeMode			shade_mode;
		CullMode			cull_mode;
		bool				front_face_ccw;
		float				polygon_offset_factor;
		float				polygon_offset_units;
		bool				depth_clip_enable;
		bool				scissor_enable;
		bool				multisample_enable;

		RasterizerStateDesc();

		friend bool operator<(RasterizerStateDesc const & lhs, RasterizerStateDesc const & rhs);
	};

	struct KLAYGE_CORE_API DepthStencilStateDesc
	{
		bool				depth_enable;
		bool				depth_write_mask;
		CompareFunction		depth_func;

		bool				front_stencil_enable;
		CompareFunction		front_stencil_func;
		uint16_t			front_stencil_read_mask;
		uint16_t			front_stencil_write_mask;
		StencilOperation	front_stencil_fail;
		StencilOperation	front_stencil_depth_fail;
		StencilOperation	front_stencil_pass;

		bool				back_stencil_enable;
		CompareFunction		back_stencil_func;
		uint16_t			back_stencil_read_mask;
		uint16_t			back_stencil_write_mask;
		StencilOperation	back_stencil_fail;
		StencilOperation	back_stencil_depth_fail;
		StencilOperation	back_stencil_pass;

		DepthStencilStateDesc();

		friend bool operator<(DepthStencilStateDesc const & lhs, DepthStencilStateDesc const & rhs);
	};

	struct KLAYGE_CORE_API BlendStateDesc
	{
		bool				alpha_to_coverage_enable;
		bool				independent_blend_enable;

		std::array<bool, 8>				blend_enable;
		std::array<bool, 8>				logic_op_enable;
		std::array<BlendOperation, 8>	blend_op;
		std::array<AlphaBlendFactor, 8>	src_blend;
		std::array<AlphaBlendFactor, 8>	dest_blend;
		std::array<BlendOperation, 8>	blend_op_alpha;
		std::array<AlphaBlendFactor, 8>	src_blend_alpha;
		std::array<AlphaBlendFactor, 8>	dest_blend_alpha;
		std::array<LogicOperation, 8>	logic_op;
		std::array<uint8_t, 8>			color_write_mask;

		BlendStateDesc();

		friend bool operator<(BlendStateDesc const & lhs, BlendStateDesc const & rhs);
	};

	struct KLAYGE_CORE_API SamplerStateDesc
	{
		Color border_clr;

		TexAddressingMode addr_mode_u;
		TexAddressingMode addr_mode_v;
		TexAddressingMode addr_mode_w;

		TexFilterOp filter;

		uint8_t max_anisotropy;
		float min_lod;
		float max_lod;
		float mip_map_lod_bias;

		CompareFunction cmp_func;

		SamplerStateDesc();

		friend bool operator<(SamplerStateDesc const & lhs, SamplerStateDesc const & rhs);
	};
#ifdef KLAYGE_HAS_STRUCT_PACK
#pragma pack(pop)
#endif

	class KLAYGE_CORE_API RasterizerStateObject
	{
	public:
		explicit RasterizerStateObject(RasterizerStateDesc const & desc)
			: desc_(desc)
		{
		}

		virtual ~RasterizerStateObject()
		{
		}

		RasterizerStateDesc const & GetDesc() const
		{
			return desc_;
		}

		virtual void Active() = 0;

	protected:
		RasterizerStateDesc desc_;
	};

	class KLAYGE_CORE_API DepthStencilStateObject
	{
	public:
		explicit DepthStencilStateObject(DepthStencilStateDesc const & desc)
			: desc_(desc)
		{
		}

		virtual ~DepthStencilStateObject()
		{
		}

		DepthStencilStateDesc const & GetDesc() const
		{
			return desc_;
		}

		virtual void Active(uint16_t front_stencil_ref, uint16_t back_stencil_ref) = 0;

	protected:
		DepthStencilStateDesc desc_;
	};

	class KLAYGE_CORE_API BlendStateObject
	{
	public:
		explicit BlendStateObject(BlendStateDesc const & desc)
			: desc_(desc)
		{
		}

		virtual ~BlendStateObject()
		{
		}

		BlendStateDesc const & GetDesc() const
		{
			return desc_;
		}

		virtual void Active(Color const & blend_factor, uint32_t sample_mask) = 0;

	protected:
		BlendStateDesc desc_;
	};

	class KLAYGE_CORE_API SamplerStateObject
	{
	public:
		explicit SamplerStateObject(SamplerStateDesc const & desc)
			: desc_(desc)
		{
		}

		virtual ~SamplerStateObject()
		{
		}

		SamplerStateDesc const & GetDesc() const
		{
			return desc_;
		}

	protected:
		SamplerStateDesc desc_;
	};
}

#endif			// _RENDERSTATEOBJECT_HPP
