// D3D9RenderEffect.hpp
// KlayGE D3D9渲染效果类 头文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2003-2006
// Homepage: http://klayge.sourceforge.net
//
// 3.4.0
// 增加了D3D9RenderEffectInclude (2006.7.12)
//
// 3.0.0
// 优化了Sampler设置 (2005.9.7)
//
// 2.5.0
// 去掉了Clone (2005.4.16)
//
// 2.4.0
// 改为派生自D3D9Resource (2005.3.3)
//
// 2.3.0
// 增加了OnLostDevice和OnResetDevice (2005.2.23)
//
// 2.0.4
// 增加了D3D9RenderTechnique (2004.3.16)
//
// 2.0.3
// 修改了SetTexture的参数 (2004.3.6)
// 增加了SetMatrixArray/GetMatrixArray (2004.3.11)
//
// 2.0.0
// 初次建立 (2003.8.15)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9RENDEREFFECT_HPP
#define _D3D9RENDEREFFECT_HPP

#define KLAYGE_LIB_NAME KlayGE_RenderEngine_D3D9
#include <KlayGE/config/auto_link.hpp>

#include <KlayGE/PreDeclare.hpp>

#include <boost/smart_ptr.hpp>

#include <d3dx9effect.h>

#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/D3D9/D3D9Typedefs.hpp>
#include <KlayGE/D3D9/D3D9Resource.hpp>

namespace KlayGE
{
	class D3D9RenderEffect;
	class D3D9RenderEffectParameterSampler;

	typedef boost::shared_ptr<D3D9RenderEffect> D3D9RenderEffectPtr;

	
	struct D3D9RenderParameterDesc
	{
		RenderEffectParameterPtr param;

		uint32_t register_set;

		uint32_t register_index;
		uint32_t register_count;

		uint32_t rows;
		uint32_t columns;
	};

	// 渲染效果
	//////////////////////////////////////////////////////////////////////////////////
	class D3D9RenderEffect : public RenderEffect, public D3D9Resource
	{
	public:

	private:
		void DoOnLostDevice()
		{
		}
		void DoOnResetDevice()
		{
		}

		RenderTechniquePtr MakeRenderTechnique();
	};

	class D3D9RenderTechnique : public RenderTechnique
	{
	public:
		explicit D3D9RenderTechnique(RenderEffect& effect)
			: RenderTechnique(effect)
		{
		}

	private:
		RenderPassPtr MakeRenderPass(uint32_t index);

		void DoBegin(uint32_t flags);
		void DoEnd();
	};

	class D3D9RenderPass : public RenderPass
	{
	public:
		D3D9RenderPass(RenderEffect& effect, uint32_t index)
			: RenderPass(effect, index)
		{
		}

		void DoRead();

		void Begin();
		void End();

	private:
		ID3D9VertexShaderPtr vertex_shader_;
		ID3D9PixelShaderPtr pixel_shader_;

		std::vector<D3D9RenderParameterDesc> param_descs_[2];

		uint32_t bool_start_[2];
		uint32_t int_start_[2];
		uint32_t float_start_[2];
		std::vector<BOOL> bool_registers_[2];
		std::vector<int> int_registers_[2];
		std::vector<float> float_registers_[2];

	private:
		void compile_shader(ID3DXBuffer*& code, ID3DXConstantTable*& constant_table,
					std::string const & profile, std::string const & name, std::string const & text);
		void create_vertex_shader(ID3DXConstantTable*& ct,
					std::string const & profile, std::string const & name, std::string const & text);
		void create_pixel_shader(ID3DXConstantTable*& ct,
					std::string const & profile, std::string const & name, std::string const & text);

		void shader(std::string& profile, std::string& name, std::string& func, std::string const & type) const;
	};
}

#endif		// _D3D9RENDEREFFECT_HPP
