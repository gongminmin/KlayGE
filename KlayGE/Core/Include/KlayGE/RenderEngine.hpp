// RenderEngine.hpp
// KlayGE 渲染引擎类 实现文件
// Ver 2.0.3
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.0.3
// 去掉了SoftwareBlend (2004.3.10)
//
// 2.0.1
// 去掉了TexBlendMode (2003.10.16)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#ifndef _RENDERENGINE_HPP
#define _RENDERENGINE_HPP

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderTarget.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/MathTypes.hpp>
#include <KlayGE/SharePtr.hpp>

#include <vector>
#include <list>

#pragma comment(lib, "KlayGE_Core.lib")

namespace KlayGE
{
	class RenderEngine
	{
	public:
		enum ShadeOptions
		{
			SO_Flat,
			SO_Gouraud,
			SO_Phong
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

		enum CullMode
		{
			Cull_None,
			Cull_Clockwise,
			Cull_AntiClockwise,
		};

		enum FogMode
		{
			// No fog. Duh.
			Fog_None,
			// Fog density increases  exponentially from the camera (fog = 1/e^(distance * density))
			Fog_Exp,
			// Fog density increases at the square of FOG_EXP, i.e. even quicker (fog = 1/e^(distance * density)^2)
			Fog_Exp2,
			// Fog density increases linearly between the start and end distances
			Fog_Linear
		};

		// Enum describing the ways to generate texture coordinates
		enum TexCoordCalcMethod
		{
			// No calculated texture coordinates
			TCC_None,
			// Environment map based on vertex normals
			TCC_EnvironmentMap,
			// Environment map based on vertex positions
			TCC_EnvironmentMapPlanar,
			TCC_EnvironmentMapReflection,
			TCC_EnvironmentMapNormal
		};

		enum TexFiltering
		{
			TF_None,
			TF_Bilinear,
			TF_Trilinear,
			TF_Anisotropic
		};

		// Texture addressing modes - default is TAM_WRAP.
		enum TexAddressingMode
		{
			// Texture wraps at values over 1.0
			TAM_Wrap,
			// Texture mirrors (flips) at joins over 1.0
			TAM_Mirror,
			// Texture clamps at 1.0
			TAM_Clamp
		};


		// Type of texture blend mode.
		enum TexBlendType
		{
			TBT_Color,
			TBT_Alpha
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

	public:
		typedef std::list<RenderTargetPtr, alloc<RenderTargetPtr> > RenderTargetList;
		typedef RenderTargetList::iterator RenderTargetListIterator;

	public:
		RenderEngine();
		virtual ~RenderEngine();

		virtual const WString& Name() const = 0;

		virtual void StartRendering() = 0;

		void SetRenderEffect(const RenderEffectPtr& effect);
		RenderEffectPtr GetRenderEffect() const
			{ return renderEffect_; }

		virtual void BeginFrame() = 0;
		virtual void Render(const VertexBuffer& vb) = 0;
		virtual void EndFrame() = 0;

		virtual void ClearColor(const Color& clr) = 0;

		virtual void ShadingType(ShadeOptions so) = 0;

		virtual void EnableLighting(bool enabled) = 0;
		virtual void AmbientLight(const Color& col) = 0;

		virtual RenderWindowPtr CreateRenderWindow(const String &name, const RenderWindowSettings& settings) = 0;

		virtual void CullingMode(CullMode mode) = 0;
		virtual void SetMaterial(const Material& mat) = 0;

		virtual void SetLight(U32 index, const Light& lt) = 0;
		virtual void LightEnable(U32 index, bool enable) = 0;

		virtual Matrix4 WorldMatrix() const = 0;
		virtual void WorldMatrix(const Matrix4& mat) = 0;
		// Sets multiple world matrices (vertex blending)
		virtual void WorldMatrices(Matrix4* mats, size_t count) = 0;
		virtual Matrix4 ViewMatrix() = 0;
		virtual void ViewMatrix(const Matrix4 &m) = 0;
		virtual Matrix4 ProjectionMatrix() = 0;
		virtual void ProjectionMatrix(const Matrix4 &m) = 0;

		virtual void DepthBufferDepthTest(bool depthTest) = 0;
		virtual void DepthBufferDepthWrite(bool depthWrite) = 0;
		virtual void DepthBufferFunction(CompareFunction depthFunction) = 0;
		virtual void DepthBias(U16 bias) = 0;

		virtual void Fog(FogMode mode = Fog_None,
			const Color& color = Color(1, 1, 1, 1),
			float expDensity = 1, float linearStart = 0, float linearEnd = 1) = 0;

		// Attaches the passed render target to the render system.
		virtual RenderTargetListIterator AddRenderTarget(const RenderTargetPtr& target);
		// Renders a iterator to the beginning of the RenderTargetList
		virtual RenderTargetListIterator RenderTargetListBegin();
		// Renders a iterator to the one the pass the last one of the RenderTargetList
		virtual RenderTargetListIterator RenderTargetListEnd();
		// Detaches the render target with the passed name from the render system and returns a pointer to it.
		virtual RenderTargetPtr RemoveRenderTarget(RenderTargetListIterator iter);
		virtual void ActiveRenderTarget(RenderTargetListIterator iter);
		const RenderTargetListIterator& ActiveRenderTarget() const
			{ return activeRenderTarget_; }

		virtual void SetTexture(U32 stage, const Texture& texture) = 0;

		// Sets the texture coordinate set to use for a texture unit.
		virtual void TextureCoordSet(U32 stage, int index) = 0;

		// Returns the number of texture units the current output hardware supports.
		virtual U32 MaxTextureStages() = 0;
		// Turns off a texture unit.
		virtual void DisableTextureStage(U32 stage) = 0;

		// Sets a method for automatically calculating texture coordinates for a stage.
		virtual void TextureCoordCalculation(U32 stage, TexCoordCalcMethod m) = 0;
		// Sets the texture addressing mode for a texture unit.
		virtual void TextureAddressingMode(U32 stage, TexAddressingMode tam) = 0;
		// Sets the texture coordinate transformation matrix for a texture unit.
		virtual void TextureMatrix(U32 stage, const Matrix4& mat) = 0;
		// Sets the texture filtering type for a texture unit.
		virtual void TextureFiltering(U32 stage, TexFiltering texFiltering) = 0;
		// Sets the maximal anisotropy for the specified texture unit.
		virtual void TextureAnisotropy(U32 stage, U32 maxAnisotropy) = 0;

		// Returns the number of matrices available to hardware vertex blending for this rendering system
		virtual U32 MaxVertexBlendMatrices();


		// Turns stencil buffer checking on or off. 
		virtual void StencilCheckEnabled(bool enabled) = 0;
		// Determines if this system supports hardware accelerated stencil buffer. 
		virtual bool HasHardwareStencil() = 0;

		// Determines the bit depth of the hardware accelerated stencil buffer, if supported.
		virtual U16 StencilBufferBitDepth() = 0;

		// Sets the stencil test function.
		virtual void StencilBufferFunction(CompareFunction func) = 0;
		// Sets the stencil buffer reference value.
		virtual void StencilBufferReferenceValue(U32 refValue) = 0;
		// Sets the stencil buffer mask value.
		virtual void StencilBufferMask(U32 mask) = 0;
		// Sets the action to perform if the stencil test fails.
		virtual void StencilBufferFailOperation(StencilOperation op) = 0;
		// Sets the action to perform if the stencil test passes, but the depth buffer test fails.
		virtual void StencilBufferDepthFailOperation(StencilOperation op) = 0;
		// Sets the action to perform if both the stencil test and the depth buffer test passes.
		virtual void StencilBufferPassOperation(StencilOperation op) = 0;

	protected:
		RenderTargetList renderTargetList_;
		RenderTargetListIterator activeRenderTarget_;

		RenderEffectPtr renderEffect_;
		UINT renderPasses_;
	};
}

#endif			// _RENDERENGINE_HPP
