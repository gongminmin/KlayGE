// RenderEffect.hpp
// KlayGE 渲染效果脚本类 头文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 2.8.0
// 增加了Do*函数，使用模板方法模式 (2005.7.24)
// 使用新的自动更新参数的方法 (2005.7.25)
//
// 2.5.0
// 去掉了Clone (2005.4.16)
// SetTechnique的返回类型改为bool (2005.4.25)
//
// 2.1.2
// 增加了Parameter (2004.5.26)
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

#ifndef _RENDEREFFECT_HPP
#define _RENDEREFFECT_HPP

#include <KlayGE/PreDeclare.hpp>
#include <vector>
#include <map>
#include <string>

#pragma warning(disable: 4100 4512)
#include <boost/variant.hpp>
#include <boost/utility.hpp>
#include <boost/operators.hpp>

#include <KlayGE/Math.hpp>

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_Core_d.lib")
#else
	#pragma comment(lib, "KlayGE_Core.lib")
#endif

namespace KlayGE
{
	// 渲染效果
	//////////////////////////////////////////////////////////////////////////////////
	class RenderEffect : boost::equality_comparable<RenderEffect>
	{
	public:
		virtual ~RenderEffect()
			{ }

		static RenderEffectPtr NullObject();

		RenderEffectParameterPtr ParameterByName(std::string const & name);
		RenderEffectParameterPtr ParameterBySemantic(std::string const & semantic);

		virtual bool Validate(std::string const & technique) = 0;
		virtual void SetTechnique(std::string const & technique) = 0;

		uint32_t Begin(uint32_t flags = 0);
		void End();
		virtual void BeginPass(uint32_t passNum) = 0;
		virtual void EndPass() = 0;

		void DirtyParam(std::string const& name);

	private:
		virtual std::string DoNameBySemantic(std::string const & semantic) = 0;
		virtual RenderEffectParameterPtr DoParameterByName(std::string const & name) = 0;

		virtual uint32_t DoBegin(uint32_t flags) = 0;
		virtual void DoEnd() = 0;

	protected:
		typedef std::map<std::string, std::pair<RenderEffectParameterPtr, bool> > params_type;
		params_type params_;
	};

	enum RenderEffectParameterType
	{
		REPT_float,
		REPT_Vector4,
		REPT_Matrix4,
		REPT_int,
		REPT_Texture,

		REPT_float_array,
		REPT_Vector4_array,
		REPT_Matrix4_array,
		REPT_int_array,

		REPT_Unknown,
	};

	class RenderEffectParameter : boost::noncopyable
	{
	public:
		RenderEffectParameter(RenderEffect& effect, std::string const & name);
		virtual ~RenderEffectParameter()
			{ }

		static RenderEffectParameterPtr NullObject();

		RenderEffectParameter& operator=(float value);
		RenderEffectParameter& operator=(Vector4 const & value);
		RenderEffectParameter& operator=(Matrix4 const & value);
		RenderEffectParameter& operator=(int value);
		RenderEffectParameter& operator=(TexturePtr const & tex);

		float ToFloat() const;
		Vector4 const & ToVector4() const;
		Matrix4 const & ToMatrix4() const;
		int ToInt() const;
		TexturePtr const & ToTexture() const;

		void SetFloatArray(float const * value, size_t count);
		void GetFloatArray(float* value, size_t count);
		void SetVector4Array(Vector4 const * value, size_t count);
		void GetVector4Array(Vector4* value, size_t count);
		void SetMatrix4Array(Matrix4 const * matrices, size_t count);
		void GetMatrix4Array(Matrix4* matrices, size_t count);
		void SetIntArray(int const * value, size_t count);
		void GetIntArray(int* value, size_t count);

		void DoFlush();

	private:
		virtual bool DoTestType(RenderEffectParameterType type) = 0;

		virtual void DoFloat(float value) = 0;
		virtual void DoVector4(Vector4 const & value) = 0;
		virtual void DoMatrix4(Matrix4 const & value) = 0;
		virtual void DoInt(int value) = 0;
		virtual void DoTexture(TexturePtr const & value) = 0;

		virtual void DoSetFloatArray(float const * value, size_t count) = 0;
		virtual void DoSetVector4Array(Vector4 const * value, size_t count) = 0;
		virtual void DoSetMatrix4Array(Matrix4 const * matrices, size_t count) = 0;
		virtual void DoSetIntArray(int const * value, size_t count) = 0;

	protected:
		RenderEffect& effect_;

		std::string name_;
		RenderEffectParameterType type_;
		boost::variant<float, Vector4, Matrix4, int, TexturePtr,
			std::vector<float>, std::vector<Vector4>, std::vector<Matrix4>, std::vector<int> > val_;
	};
}

#endif		// _RENDEREFFECT_HPP
