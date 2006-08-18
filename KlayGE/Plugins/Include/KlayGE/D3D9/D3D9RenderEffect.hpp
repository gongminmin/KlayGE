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
#include <KlayGE/MapVector.hpp>

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

	// 渲染效果
	//////////////////////////////////////////////////////////////////////////////////
	class D3D9RenderEffect : public RenderEffect, public D3D9Resource
	{
	public:
		explicit D3D9RenderEffect(std::string const & srcData);

		ID3DXEffectPtr const & D3DXEffect() const
			{ return d3dx_effect_; }

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

		std::string DoNameBySemantic(std::string const & semantic);
		RenderEffectParameterPtr DoParameterByName(std::string const & name);

		RenderTechniquePtr MakeRenderTechnique(uint32_t n);

	private:
		ID3DXEffectPtr d3dx_effect_;
	};

	class D3D9RenderTechnique : public RenderTechnique
	{
	public:
		D3D9RenderTechnique(RenderEffect& effect, std::string const & name, D3DXHANDLE tech);

		bool Validate();

	private:
		RenderPassPtr MakeRenderPass(uint32_t n);

		uint32_t DoBegin(uint32_t flags);
		void DoEnd();

	private:
		D3DXHANDLE tech_;
	};

	class D3D9RenderPass : public RenderPass
	{
	public:
		D3D9RenderPass(RenderEffect& effect, uint32_t index, D3DXHANDLE pass);

		void Begin();
		void End();

	private:
		D3DXHANDLE pass_;

		ID3DXConstantTablePtr constant_table_[2];
		MapVector<RenderEffectParameterPtr, uint32_t> samplers_[2];
	};

	class D3D9RenderEffectParameterBool : public RenderEffectParameterConcrete<bool>, public D3D9Resource
	{
	public:
		D3D9RenderEffectParameterBool(RenderEffect& effect, std::string const & name)
			: RenderEffectParameterConcrete<bool>(effect, name)
		{
		}

	private:
		void DoFlush(bool const & value);

	private:
		virtual void DoOnLostDevice()
		{
		}
		virtual void DoOnResetDevice()
		{
		}

	private:
		D3D9RenderEffectParameterBool(D3D9RenderEffectParameterBool const & rhs);
		D3D9RenderEffectParameterBool& operator=(D3D9RenderEffectParameterBool const & rhs);
	};

	class D3D9RenderEffectParameterInt : public RenderEffectParameterConcrete<int>, public D3D9Resource
	{
	public:
		D3D9RenderEffectParameterInt(RenderEffect& effect, std::string const & name)
			: RenderEffectParameterConcrete<int>(effect, name)
		{
		}

	private:
		void DoFlush(int const & value);

	private:
		virtual void DoOnLostDevice()
		{
		}
		virtual void DoOnResetDevice()
		{
		}

	private:
		D3D9RenderEffectParameterInt(D3D9RenderEffectParameterInt const & rhs);
		D3D9RenderEffectParameterInt& operator=(D3D9RenderEffectParameterInt const & rhs);
	};

	class D3D9RenderEffectParameterFloat : public RenderEffectParameterConcrete<float>, public D3D9Resource
	{
	public:
		D3D9RenderEffectParameterFloat(RenderEffect& effect, std::string const & name)
			: RenderEffectParameterConcrete<float>(effect, name)
		{
		}

	private:
		void DoFlush(float const & value);

	private:
		virtual void DoOnLostDevice()
		{
		}
		virtual void DoOnResetDevice()
		{
		}

	private:
		D3D9RenderEffectParameterFloat(D3D9RenderEffectParameterFloat const & rhs);
		D3D9RenderEffectParameterFloat& operator=(D3D9RenderEffectParameterFloat const & rhs);
	};

	class D3D9RenderEffectParameterVector2 : public RenderEffectParameterConcrete<float2>, public D3D9Resource
	{
	public:
		D3D9RenderEffectParameterVector2(RenderEffect& effect, std::string const & name)
			: RenderEffectParameterConcrete<float2>(effect, name)
		{
		}

	private:
		void DoFlush(float2 const & value);

	private:
		virtual void DoOnLostDevice()
		{
		}
		virtual void DoOnResetDevice()
		{
		}

	private:
		D3D9RenderEffectParameterVector2(D3D9RenderEffectParameterVector2 const & rhs);
		D3D9RenderEffectParameterVector2& operator=(D3D9RenderEffectParameterVector2 const & rhs);
	};

	class D3D9RenderEffectParameterVector3 : public RenderEffectParameterConcrete<float3>, public D3D9Resource
	{
	public:
		D3D9RenderEffectParameterVector3(RenderEffect& effect, std::string const & name)
			: RenderEffectParameterConcrete<float3>(effect, name)
		{
		}

	private:
		void DoFlush(float3 const & value);

	private:
		virtual void DoOnLostDevice()
		{
		}
		virtual void DoOnResetDevice()
		{
		}

	private:
		D3D9RenderEffectParameterVector3(D3D9RenderEffectParameterVector3 const & rhs);
		D3D9RenderEffectParameterVector3& operator=(D3D9RenderEffectParameterVector3 const & rhs);
	};

	class D3D9RenderEffectParameterVector4 : public RenderEffectParameterConcrete<float4>, public D3D9Resource
	{
	public:
		D3D9RenderEffectParameterVector4(RenderEffect& effect, std::string const & name)
			: RenderEffectParameterConcrete<float4>(effect, name)
		{
		}

	private:
		void DoFlush(float4 const & value);

	private:
		virtual void DoOnLostDevice()
		{
		}
		virtual void DoOnResetDevice()
		{
		}

	private:
		D3D9RenderEffectParameterVector4(D3D9RenderEffectParameterVector4 const & rhs);
		D3D9RenderEffectParameterVector4& operator=(D3D9RenderEffectParameterVector4 const & rhs);
	};

	class D3D9RenderEffectParameterMatrix4 : public RenderEffectParameterConcrete<float4x4>, public D3D9Resource
	{
	public:
		D3D9RenderEffectParameterMatrix4(RenderEffect& effect, std::string const & name)
			: RenderEffectParameterConcrete<float4x4>(effect, name)
		{
		}

	private:
		void DoFlush(float4x4 const & value);

	private:
		void DoOnLostDevice()
		{
		}
		void DoOnResetDevice()
		{
		}

	private:
		D3D9RenderEffectParameterMatrix4(D3D9RenderEffectParameterMatrix4 const & rhs);
		D3D9RenderEffectParameterMatrix4& operator=(D3D9RenderEffectParameterMatrix4 const & rhs);
	};

	class D3D9RenderEffectParameterSampler : public RenderEffectParameterConcrete<SamplerPtr>, public D3D9Resource
	{
	public:
		D3D9RenderEffectParameterSampler(RenderEffect& effect, std::string const & name)
			: RenderEffectParameterConcrete<SamplerPtr>(effect, name)
		{
		}

	private:
		void DoFlush(SamplerPtr const & value);

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

	private:
		D3D9RenderEffectParameterSampler(D3D9RenderEffectParameterSampler const & rhs);
		D3D9RenderEffectParameterSampler& operator=(D3D9RenderEffectParameterSampler const & rhs);
	};

	class D3D9RenderEffectParameterBoolArray : public RenderEffectParameterConcrete<std::vector<bool> >, public D3D9Resource
	{
	public:
		D3D9RenderEffectParameterBoolArray(RenderEffect& effect, std::string const & name)
			: RenderEffectParameterConcrete<std::vector<bool> >(effect, name)
		{
		}

	private:
		void DoFlush(std::vector<bool> const & value);

	private:
		virtual void DoOnLostDevice()
		{
		}
		virtual void DoOnResetDevice()
		{
		}

	private:
		D3D9RenderEffectParameterBoolArray(D3D9RenderEffectParameterBoolArray const & rhs);
		D3D9RenderEffectParameterBoolArray& operator=(D3D9RenderEffectParameterBoolArray const & rhs);
	};

	class D3D9RenderEffectParameterIntArray : public RenderEffectParameterConcrete<std::vector<int> >, public D3D9Resource
	{
	public:
		D3D9RenderEffectParameterIntArray(RenderEffect& effect, std::string const & name)
			: RenderEffectParameterConcrete<std::vector<int> >(effect, name)
		{
		}

	private:
		void DoFlush(std::vector<int> const & value);

	private:
		virtual void DoOnLostDevice()
		{
		}
		virtual void DoOnResetDevice()
		{
		}

	private:
		D3D9RenderEffectParameterIntArray(D3D9RenderEffectParameterIntArray const & rhs);
		D3D9RenderEffectParameterIntArray& operator=(D3D9RenderEffectParameterIntArray const & rhs);
	};

	class D3D9RenderEffectParameterFloatArray : public RenderEffectParameterConcrete<std::vector<float> >, public D3D9Resource
	{
	public:
		D3D9RenderEffectParameterFloatArray(RenderEffect& effect, std::string const & name)
			: RenderEffectParameterConcrete<std::vector<float> >(effect, name)
		{
		}

	private:
		void DoFlush(std::vector<float> const & value);

	private:
		virtual void DoOnLostDevice()
		{
		}
		virtual void DoOnResetDevice()
		{
		}

	private:
		D3D9RenderEffectParameterFloatArray(D3D9RenderEffectParameterFloatArray const & rhs);
		D3D9RenderEffectParameterFloatArray& operator=(D3D9RenderEffectParameterFloatArray const & rhs);
	};

	class D3D9RenderEffectParameterVector4Array : public RenderEffectParameterConcrete<std::vector<float4> >, public D3D9Resource
	{
	public:
		D3D9RenderEffectParameterVector4Array(RenderEffect& effect, std::string const & name)
			: RenderEffectParameterConcrete<std::vector<float4> >(effect, name)
		{
		}

	private:
		void DoFlush(std::vector<float4> const & value);

	private:
		virtual void DoOnLostDevice()
		{
		}
		virtual void DoOnResetDevice()
		{
		}

	private:
		D3D9RenderEffectParameterVector4Array(D3D9RenderEffectParameterVector4Array const & rhs);
		D3D9RenderEffectParameterVector4Array& operator=(D3D9RenderEffectParameterVector4Array const & rhs);
	};

	class D3D9RenderEffectParameterMatrix4Array : public RenderEffectParameterConcrete<std::vector<float4x4> >, public D3D9Resource
	{
	public:
		D3D9RenderEffectParameterMatrix4Array(RenderEffect& effect, std::string const & name)
			: RenderEffectParameterConcrete<std::vector<float4x4> >(effect, name)
		{
		}

	private:
		void DoFlush(std::vector<float4x4> const & value);

	private:
		virtual void DoOnLostDevice()
		{
		}
		virtual void DoOnResetDevice()
		{
		}

	private:
		D3D9RenderEffectParameterMatrix4Array(D3D9RenderEffectParameterMatrix4Array const & rhs);
		D3D9RenderEffectParameterMatrix4Array& operator=(D3D9RenderEffectParameterMatrix4Array const & rhs);
	};

	class D3D9RenderEffectInclude : public ID3DXInclude
	{
	public:
        virtual ~D3D9RenderEffectInclude()
        {
        }

		STDMETHOD(Open)(D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName,
			LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes);
		STDMETHOD(Close)(LPCVOID pData);
	};
}

#endif		// _D3D9RENDEREFFECT_HPP
