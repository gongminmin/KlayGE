// D3D9RenderEffect.hpp
// KlayGE D3D9渲染效果类 头文件
// Ver 2.5.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
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

#include <KlayGE/PreDeclare.hpp>
#include <boost/smart_ptr.hpp>

#include <map>

#include <d3dx9effect.h>

#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/D3D9/D3D9Resource.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9_d.lib")
#else
	#pragma comment(lib, "KlayGE_RenderEngine_D3D9.lib")
#endif

namespace KlayGE
{
	class D3D9RenderEffect;
	class D3D9RenderEffectParameterTexture;

	typedef boost::shared_ptr<D3D9RenderEffect> D3D9RenderEffectPtr;

	// 渲染效果
	//////////////////////////////////////////////////////////////////////////////////
	class D3D9RenderEffect : public RenderEffect, public D3D9Resource
	{
	public:
		explicit D3D9RenderEffect(std::string const & srcData);

		boost::shared_ptr<ID3DXEffect> const & D3DXEffect() const
			{ return d3dx_effect_; }

		bool Validate(std::string const & technique);
		void SetTechnique(std::string const & technique);

		void BeginPass(uint32_t passNum);
		void EndPass();

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

		std::string DoNameBySemantic(std::string const & semantic);
		RenderEffectParameterPtr DoParameterByName(std::string const & name);

		uint32_t DoBegin(uint32_t flags);
		void DoEnd();

	private:
		boost::shared_ptr<ID3DXEffect> d3dx_effect_;

		typedef std::vector<boost::weak_ptr<D3D9RenderEffectParameterTexture> > tex_params_type;
		tex_params_type tex_params_;
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

	class D3D9RenderEffectParameterVector4 : public RenderEffectParameterConcrete<Vector4>, public D3D9Resource
	{
	public:
		D3D9RenderEffectParameterVector4(RenderEffect& effect, std::string const & name)
			: RenderEffectParameterConcrete<Vector4>(effect, name)
		{
		}

	private:
		void DoFlush(Vector4 const & value);

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

	class D3D9RenderEffectParameterMatrix4 : public RenderEffectParameterConcrete<Matrix4>, public D3D9Resource
	{
	public:
		D3D9RenderEffectParameterMatrix4(RenderEffect& effect, std::string const & name)
			: RenderEffectParameterConcrete<Matrix4>(effect, name)
		{
		}

	private:
		void DoFlush(Matrix4 const & value);

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

	class D3D9RenderEffectParameterTexture : public RenderEffectParameterConcrete<TexturePtr>, public D3D9Resource
	{
	public:
		D3D9RenderEffectParameterTexture(RenderEffect& effect, std::string const & name)
			: RenderEffectParameterConcrete<TexturePtr>(effect, name)
		{
		}

	private:
		void DoFlush(TexturePtr const & value);

	private:
		void DoOnLostDevice();
		void DoOnResetDevice();

	private:
		D3D9RenderEffectParameterTexture(D3D9RenderEffectParameterTexture const & rhs);
		D3D9RenderEffectParameterTexture& operator=(D3D9RenderEffectParameterTexture const & rhs);
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

	class D3D9RenderEffectParameterVector4Array : public RenderEffectParameterConcrete<std::vector<Vector4> >, public D3D9Resource
	{
	public:
		D3D9RenderEffectParameterVector4Array(RenderEffect& effect, std::string const & name)
			: RenderEffectParameterConcrete<std::vector<Vector4> >(effect, name)
		{
		}

	private:
		void DoFlush(std::vector<Vector4> const & value);

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

	class D3D9RenderEffectParameterMatrix4Array : public RenderEffectParameterConcrete<std::vector<Matrix4> >, public D3D9Resource
	{
	public:
		D3D9RenderEffectParameterMatrix4Array(RenderEffect& effect, std::string const & name)
			: RenderEffectParameterConcrete<std::vector<Matrix4> >(effect, name)
		{
		}

	private:
		void DoFlush(std::vector<Matrix4> const & value);

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
}

#endif		// _D3D9RENDEREFFECT_HPP
