// RenderStateObject.hpp
// KlayGE 渲染状态对象类 头文件
// Ver 3.7.0
// 版权所有(C) 龚敏敏, 2006-2008
// Homepage: http://klayge.sourceforge.net
//
// 3.7.0
// 把RenderStateObject拆成三部分 (2008.6.29)
//
// 3.5.0
// 初次建立 (2006.11.1)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERSTATEOBJECT_HPP
#define _RENDERSTATEOBJECT_HPP

#include <KlayGE/PreDeclare.hpp>

#include <boost/array.hpp>

namespace KlayGE
{
	inline uint32_t
	float_to_uint32(float v)
	{
		union
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
		union
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
		CM_Clockwise,
		CM_AntiClockwise
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
		ABF_Src_Alpha_Sat
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
		SOP_Increment,
		// Decrease the stencil value by 1, clamping at 0
		SOP_Decrement,
		// Invert the bits of the stencil buffer
		SOP_Invert
	};

	enum ColorMask
	{
		CMASK_Red   = 1UL << 3,
		CMASK_Green = 1UL << 2,
		CMASK_Blue  = 1UL << 1,
		CMASK_Alpha = 1UL << 0,
		CMASK_All   = CMASK_Red | CMASK_Green | CMASK_Blue | CMASK_Alpha
	};

#ifdef KLAYGE_PLATFORM_WINDOWS
#pragma pack(push, 1)
#endif
	struct RasterizerStateDesc
	{
		PolygonMode			polygon_mode;
		ShadeMode			shade_mode;
		CullMode			cull_mode;
		float				polygon_offset_factor;
		float				polygon_offset_units;
		bool				scissor_enable;
		bool				multisample_enable;

		RasterizerStateDesc();

		friend bool operator<(RasterizerStateDesc const & lhs, RasterizerStateDesc const & rhs);
	};

	struct DepthStencilStateDesc
	{
		bool				depth_enable;
		bool				depth_write_mask;
		CompareFunction		depth_func;

		bool				front_stencil_enable;
		CompareFunction		front_stencil_func;
		uint16_t			front_stencil_ref;
		uint16_t			front_stencil_read_mask;
		uint16_t			front_stencil_write_mask;
		StencilOperation	front_stencil_fail;
		StencilOperation	front_stencil_depth_fail;
		StencilOperation	front_stencil_pass;

		bool				back_stencil_enable;
		CompareFunction		back_stencil_func;
		uint16_t			back_stencil_ref;
		uint16_t			back_stencil_read_mask;
		uint16_t			back_stencil_write_mask;
		StencilOperation	back_stencil_fail;
		StencilOperation	back_stencil_depth_fail;
		StencilOperation	back_stencil_pass;

		DepthStencilStateDesc();

		friend bool operator<(DepthStencilStateDesc const & lhs, DepthStencilStateDesc const & rhs);
	};

	struct BlendStateDesc
	{
		bool				alpha_to_coverage_enable;
		bool				independent_blend_enable;

		boost::array<bool, 8>				blend_enable;
		boost::array<BlendOperation, 8>		blend_op;
		boost::array<AlphaBlendFactor, 8>	src_blend;
		boost::array<AlphaBlendFactor, 8>	dest_blend;
		boost::array<BlendOperation, 8>		blend_op_alpha;
		boost::array<AlphaBlendFactor, 8>	src_blend_alpha;
		boost::array<AlphaBlendFactor, 8>	dest_blend_alpha;
		boost::array<uint8_t, 8>			color_write_mask;

		BlendStateDesc();

		friend bool operator<(BlendStateDesc const & lhs, BlendStateDesc const & rhs);
	};
#ifdef KLAYGE_PLATFORM_WINDOWS
#pragma pack(pop)
#endif

	class RasterizerStateObject
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

		static RasterizerStateObjectPtr NullObject();

		virtual void SetStates(RasterizerStateObject const & current) = 0;

	protected:
		RasterizerStateDesc desc_;
	};

	class DepthStencilStateObject
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

		static DepthStencilStateObjectPtr NullObject();

		virtual void SetStates(DepthStencilStateObject const & current) = 0;

	protected:
		DepthStencilStateDesc desc_;
	};

	class BlendStateObject
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

		static BlendStateObjectPtr NullObject();

		virtual void SetStates(BlendStateObject const & current) = 0;

	protected:
		BlendStateDesc desc_;
	};
}

#endif			// _RENDERSTATEOBJECT_HPP
