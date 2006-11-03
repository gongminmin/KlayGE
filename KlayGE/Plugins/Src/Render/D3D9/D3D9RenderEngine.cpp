// D3D9RenderEngine.cpp
// KlayGE D3D9渲染引擎类 实现文件
// Ver 3.0.0
// 版权所有(C) 龚敏敏, 2003-2005
// Homepage: http://klayge.sourceforge.net
//
// 3.0.0
// 去掉了固定流水线 (2005.8.18)
//
// 2.8.0
// 增加了RenderDeviceCaps (2005.7.17)
// 简化了StencilBuffer相关操作 (2005.7.20)
//
// 2.7.0
// 改进了Render (2005.6.16)
// 去掉了TextureCoordSet (2005.6.26)
// TextureAddressingMode, TextureFiltering和TextureAnisotropy移到Texture中 (2005.6.27)
//
// 2.4.0
// 增加了PolygonMode (2005.3.20)
//
// 2.0.4
// 去掉了WorldMatrices (2004.4.3)
//
// 2.0.3
// 优化了Render (2004.2.22)
//
// 2.0.1
// 重构了Render (2003.10.10)
//
// 2.0.0
// 初次建立 (2003.8.15)
//
// 修改记录
//////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Viewport.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/RenderTarget.hpp>
#include <KlayGE/RenderStateObject.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/RenderSettings.hpp>

#include <KlayGE/D3D9/D3D9RenderWindow.hpp>
#include <KlayGE/D3D9/D3D9FrameBuffer.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>
#include <KlayGE/D3D9/D3D9GraphicsBuffer.hpp>
#include <KlayGE/D3D9/D3D9Mapping.hpp>
#include <KlayGE/D3D9/D3D9RenderLayout.hpp>
#include <KlayGE/D3D9/D3D9ShaderObject.hpp>

#include <algorithm>
#include <boost/assert.hpp>
#include <boost/bind.hpp>

#include <KlayGE/D3D9/D3D9RenderEngine.hpp>

#ifdef KLAYGE_COMPILER_MSVC
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d9.lib")
#endif

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	D3D9RenderEngine::D3D9RenderEngine()
		: last_num_vertex_stream_(0)
	{
		// Create our Direct3D object
		d3d_ = MakeCOMPtr(Direct3DCreate9(D3D_SDK_VERSION));
		Verify(d3d_ != ID3D9Ptr());

		adapterList_.Enumerate(d3d_);
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	D3D9RenderEngine::~D3D9RenderEngine()
	{
		render_tech_.reset();
		cur_render_target_.reset();

		d3dDevice_.reset();
		d3d_.reset();
	}

	// 返回渲染系统的名字
	/////////////////////////////////////////////////////////////////////////////////
	std::wstring const & D3D9RenderEngine::Name() const
	{
		static std::wstring const name(L"Direct3D9 Render Engine");
		return name;
	}

	// 获取D3D接口
	/////////////////////////////////////////////////////////////////////////////////
	ID3D9Ptr const & D3D9RenderEngine::D3DObject() const
	{
		return d3d_;
	}

	// 获取D3D Device接口
	/////////////////////////////////////////////////////////////////////////////////
	ID3D9DevicePtr const & D3D9RenderEngine::D3DDevice() const
	{
		return d3dDevice_;
	}

	// 获取D3D适配器列表
	/////////////////////////////////////////////////////////////////////////////////
	D3D9AdapterList const & D3D9RenderEngine::D3DAdapters() const
	{
		return adapterList_;
	}

	// 获取当前适配器
	/////////////////////////////////////////////////////////////////////////////////
	D3D9Adapter const & D3D9RenderEngine::ActiveAdapter() const
	{
		return adapterList_.Adapter(adapterList_.CurrentAdapterIndex());
	}

	// 开始渲染
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::StartRendering()
	{
		bool gotMsg;
		MSG  msg;

		::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

		RenderTarget& renderTarget(*this->CurRenderTarget());
		while (WM_QUIT != msg.message)
		{
			// 如果窗口是激活的，用 PeekMessage()以便我们可以用空闲时间渲染场景
			// 不然, 用 GetMessage() 减少 CPU 占用率
			if (renderTarget.Active())
			{
				gotMsg = (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0);
			}
			else
			{
				gotMsg = (::GetMessage(&msg, NULL, 0, 0) != 0);
			}

			if (gotMsg)
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
			else
			{
				// 在空余时间渲染帧 (没有等待的消息)
				if (renderTarget.Active())
				{
					renderTarget.Update();
				}
			}
		}
	}

	// 清空缓冲区
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::Clear(uint32_t masks, Color const & clr, float depth, int32_t stencil)
	{
		uint32_t flags = 0;
		if (masks & CBM_Color)
		{
			flags |= D3DCLEAR_TARGET;
		}
		if (masks & CBM_Depth)
		{
			flags |= D3DCLEAR_ZBUFFER;
		}
		if (masks & CBM_Stencil)
		{
			flags |= D3DCLEAR_STENCIL;
		}

		TIF(d3dDevice_->Clear(0, NULL, flags,
			D3DCOLOR_COLORVALUE(clr.r(), clr.g(), clr.b(), clr.a()),
			depth, stencil));
	}

	// 建立渲染窗口
	/////////////////////////////////////////////////////////////////////////////////
	RenderWindowPtr D3D9RenderEngine::CreateRenderWindow(std::string const & name,
		RenderSettings const & settings)
	{
		D3D9RenderWindowPtr win(new D3D9RenderWindow(d3d_, this->ActiveAdapter(),
			name, settings));
		default_render_target_ = win;

		d3dDevice_ = win->D3DDevice();
		Verify(d3dDevice_ != ID3D9DevicePtr());

		this->FillRenderDeviceCaps();
		this->BindRenderTarget(win);

		if (caps_.hw_instancing_support)
		{
			RenderInstance = boost::bind(&D3D9RenderEngine::DoRenderHWInstance, this, _1);
		}
		else
		{
			RenderInstance = boost::bind(&D3D9RenderEngine::DoRenderSWInstance, this, _1);
		}

		return win;
	}

	// 设置当前渲染状态对象
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::SetStateObjects(RenderStateObject const & rs_obj, ShaderObject const & shader_obj)
	{
		for (int i = 0; i < RenderStateObject::RST_NUM_RENDER_STATES; ++ i)
		{
			RenderStateObject::RenderStateType rst = static_cast<RenderStateObject::RenderStateType>(i);
			uint32_t state = rs_obj.GetRenderState(rst);
			if (cur_render_state_obj_.GetRenderState(rst) != state)
			{
				switch (rst)
				{
				case RenderStateObject::RST_PolygonMode:
					d3dDevice_->SetRenderState(D3DRS_FILLMODE,
						D3D9Mapping::Mapping(static_cast<RenderStateObject::PolygonMode>(state)));
					break;

				case RenderStateObject::RST_ShadeMode:
					d3dDevice_->SetRenderState(D3DRS_SHADEMODE,
						D3D9Mapping::Mapping(static_cast<RenderStateObject::ShadeMode>(state)));
					break;

				case RenderStateObject::RST_CullMode:
					d3dDevice_->SetRenderState(D3DRS_CULLMODE,
						D3D9Mapping::Mapping(static_cast<RenderStateObject::CullMode>(state)));
					break;

				case RenderStateObject::RST_AlphaToCoverageEnable:
					// NVIDIA's Transparency Multisampling
					if (S_OK == d3d_->CheckDeviceFormat(D3DADAPTER_DEFAULT,
						D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE,
						static_cast<D3DFORMAT>(MakeFourCC<'A', 'T', 'O', 'C'>::value)))
					{
						if (state)
						{
							d3dDevice_->SetRenderState(D3DRS_ADAPTIVETESS_Y,
								static_cast<D3DFORMAT>(MakeFourCC<'A', 'T', 'O', 'C'>::value));
						}
						else
						{
							d3dDevice_->SetRenderState(D3DRS_ADAPTIVETESS_Y, D3DFMT_UNKNOWN);
						}
					}
					break;

				case RenderStateObject::RST_BlendEnable:
					d3dDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, state);
					d3dDevice_->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, state);
					break;

				case RenderStateObject::RST_BlendOp:
					d3dDevice_->SetRenderState(D3DRS_BLENDOP,
						D3D9Mapping::Mapping(static_cast<RenderStateObject::BlendOperation>(state)));
					break;

				case RenderStateObject::RST_SrcBlend:
					d3dDevice_->SetRenderState(D3DRS_SRCBLEND,
						D3D9Mapping::Mapping(static_cast<RenderStateObject::AlphaBlendFactor>(state)));
					break;

				case RenderStateObject::RST_DestBlend:
					d3dDevice_->SetRenderState(D3DRS_DESTBLEND,
						D3D9Mapping::Mapping(static_cast<RenderStateObject::AlphaBlendFactor>(state)));
					break;

				case RenderStateObject::RST_BlendOpAlpha:
					d3dDevice_->SetRenderState(D3DRS_BLENDOPALPHA,
						D3D9Mapping::Mapping(static_cast<RenderStateObject::BlendOperation>(state)));
					break;

				case RenderStateObject::RST_SrcBlendAlpha:
					d3dDevice_->SetRenderState(D3DRS_SRCBLENDALPHA,
						D3D9Mapping::Mapping(static_cast<RenderStateObject::AlphaBlendFactor>(state)));
					break;

				case RenderStateObject::RST_DestBlendAlpha:
					d3dDevice_->SetRenderState(D3DRS_DESTBLENDALPHA,
						D3D9Mapping::Mapping(static_cast<RenderStateObject::AlphaBlendFactor>(state)));
					break;

				case RenderStateObject::RST_DepthEnable:
					d3dDevice_->SetRenderState(D3DRS_ZENABLE, state ? D3DZB_TRUE : D3DZB_FALSE);
					break;

				case RenderStateObject::RST_DepthMask:
					d3dDevice_->SetRenderState(D3DRS_ZWRITEENABLE, state ? D3DZB_TRUE : D3DZB_FALSE);
					break;

				case RenderStateObject::RST_DepthFunc:
					d3dDevice_->SetRenderState(D3DRS_ZFUNC,
						D3D9Mapping::Mapping(static_cast<RenderStateObject::CompareFunction>(state)));
					break;

				case RenderStateObject::RST_PolygonOffsetFactor:
					d3dDevice_->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, state);
					break;

				case RenderStateObject::RST_PolygonOffsetUnits:
					d3dDevice_->SetRenderState(D3DRS_DEPTHBIAS, state);
					break;

				case RenderStateObject::RST_FrontStencilFunc:
					d3dDevice_->SetRenderState(D3DRS_STENCILFUNC,
						D3D9Mapping::Mapping(static_cast<RenderStateObject::CompareFunction>(state)));
					break;

				case RenderStateObject::RST_FrontStencilRef:
					d3dDevice_->SetRenderState(D3DRS_STENCILREF, state);
					break;

				case RenderStateObject::RST_FrontStencilMask:
					d3dDevice_->SetRenderState(D3DRS_STENCILMASK, state);
					break;

				case RenderStateObject::RST_FrontStencilFail:
					d3dDevice_->SetRenderState(D3DRS_STENCILFAIL,
						D3D9Mapping::Mapping(static_cast<RenderStateObject::StencilOperation>(state)));
					break;

				case RenderStateObject::RST_FrontStencilDepthFail:
					d3dDevice_->SetRenderState(D3DRS_STENCILZFAIL,
						D3D9Mapping::Mapping(static_cast<RenderStateObject::StencilOperation>(state)));
					break;

				case RenderStateObject::RST_FrontStencilPass:
					d3dDevice_->SetRenderState(D3DRS_STENCILPASS,
						D3D9Mapping::Mapping(static_cast<RenderStateObject::StencilOperation>(state)));
					break;

				case RenderStateObject::RST_FrontStencilWriteMask:
					d3dDevice_->SetRenderState(D3DRS_STENCILWRITEMASK, state);
					break;

				case RenderStateObject::RST_BackStencilFunc:
					d3dDevice_->SetRenderState(D3DRS_CCW_STENCILFUNC,
						D3D9Mapping::Mapping(static_cast<RenderStateObject::CompareFunction>(state)));
					break;

				case RenderStateObject::RST_BackStencilRef:
					d3dDevice_->SetRenderState(D3DRS_STENCILREF, state);
					break;

				case RenderStateObject::RST_BackStencilMask:
					d3dDevice_->SetRenderState(D3DRS_STENCILMASK, state);
					break;

				case RenderStateObject::RST_BackStencilFail:
					d3dDevice_->SetRenderState(D3DRS_CCW_STENCILFAIL,
						D3D9Mapping::Mapping(static_cast<RenderStateObject::StencilOperation>(state)));
					break;

				case RenderStateObject::RST_BackStencilDepthFail:
					d3dDevice_->SetRenderState(D3DRS_CCW_STENCILZFAIL,
						D3D9Mapping::Mapping(static_cast<RenderStateObject::StencilOperation>(state)));
					break;

				case RenderStateObject::RST_BackStencilPass:
					d3dDevice_->SetRenderState(D3DRS_CCW_STENCILPASS,
						D3D9Mapping::Mapping(static_cast<RenderStateObject::StencilOperation>(state)));
					break;

				case RenderStateObject::RST_BackStencilWriteMask:
					d3dDevice_->SetRenderState(D3DRS_STENCILWRITEMASK, state);
					break;

				case RenderStateObject::RST_FrontStencilEnable:
				case RenderStateObject::RST_BackStencilEnable:
					if (rs_obj.GetRenderState(RenderStateObject::RST_FrontStencilEnable)
						&& rs_obj.GetRenderState(RenderStateObject::RST_BackStencilEnable))
					{
						d3dDevice_->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, true);
					}
					else
					{
						if (rs_obj.GetRenderState(RenderStateObject::RST_FrontStencilEnable))
						{
							d3dDevice_->SetRenderState(D3DRS_STENCILENABLE, true);
						}
						else
						{
							if (rs_obj.GetRenderState(RenderStateObject::RST_BackStencilEnable))
							{
								d3dDevice_->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, true);
								d3dDevice_->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
							}
							else
							{
								d3dDevice_->SetRenderState(D3DRS_STENCILENABLE, false);
								d3dDevice_->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, false);
							}
						}
					}
					break;

				case RenderStateObject::RST_ScissorEnable:
					d3dDevice_->SetRenderState(D3DRS_SCISSORTESTENABLE, state);
					break;

				case RenderStateObject::RST_ColorMask0:
					d3dDevice_->SetRenderState(D3DRS_COLORWRITEENABLE,
						D3D9Mapping::MappingColorMask(state));
					break;

				case RenderStateObject::RST_ColorMask1:
					d3dDevice_->SetRenderState(D3DRS_COLORWRITEENABLE,
						D3D9Mapping::MappingColorMask(state));
					break;

				case RenderStateObject::RST_ColorMask2:
					d3dDevice_->SetRenderState(D3DRS_COLORWRITEENABLE,
						D3D9Mapping::MappingColorMask(state));
					break;

				case RenderStateObject::RST_ColorMask3:
					d3dDevice_->SetRenderState(D3DRS_COLORWRITEENABLE,
						D3D9Mapping::MappingColorMask(state));
					break;
				}

				cur_render_state_obj_.SetRenderState(rst, state);
			}
		}

		D3D9ShaderObject const & d3d9_shader_obj = *checked_cast<D3D9ShaderObject const *>(&shader_obj);

		d3dDevice_->SetVertexShader(d3d9_shader_obj.VertexShader().get());
		d3dDevice_->SetPixelShader(d3d9_shader_obj.PixelShader().get());

		for (size_t i = 0; i < ShaderObject::ST_NumShaderTypes; ++ i)
		{
			ShaderObject::ShaderType type = static_cast<ShaderObject::ShaderType>(i);

			if (!d3d9_shader_obj.BoolRegisters(type).empty())
			{
				if (ShaderObject::ST_VertexShader == type)
				{
					d3dDevice_->SetVertexShaderConstantB(d3d9_shader_obj.BoolStart(type), &d3d9_shader_obj.BoolRegisters(type)[0],
						static_cast<UINT>(d3d9_shader_obj.BoolRegisters(type).size()) / 4);
				}
				else
				{
					d3dDevice_->SetPixelShaderConstantB(d3d9_shader_obj.BoolStart(type), &d3d9_shader_obj.BoolRegisters(type)[0],
						static_cast<UINT>(d3d9_shader_obj.BoolRegisters(type).size()) / 4);
				}
			}
			if (!d3d9_shader_obj.IntRegisters(type).empty())
			{
				if (ShaderObject::ST_VertexShader == type)
				{
					d3dDevice_->SetVertexShaderConstantI(d3d9_shader_obj.IntStart(type), &d3d9_shader_obj.IntRegisters(type)[0],
						static_cast<UINT>(d3d9_shader_obj.IntRegisters(type).size()) / 4);
				}
				else
				{
					d3dDevice_->SetPixelShaderConstantI(d3d9_shader_obj.IntStart(type), &d3d9_shader_obj.IntRegisters(type)[0],
						static_cast<UINT>(d3d9_shader_obj.IntRegisters(type).size()) / 4);
				}
			}
			if (!d3d9_shader_obj.FloatRegisters(type).empty())
			{
				if (ShaderObject::ST_VertexShader == type)
				{
					d3dDevice_->SetVertexShaderConstantF(d3d9_shader_obj.FloatStart(type), &d3d9_shader_obj.FloatRegisters(type)[0],
						static_cast<UINT>(d3d9_shader_obj.FloatRegisters(type).size()) / 4);
				}
				else
				{
					d3dDevice_->SetPixelShaderConstantF(d3d9_shader_obj.FloatStart(type), &d3d9_shader_obj.FloatRegisters(type)[0],
						static_cast<UINT>(d3d9_shader_obj.FloatRegisters(type).size()) / 4);
				}
			}

			for (uint32_t j = 0; j < d3d9_shader_obj.Samplers(type).size(); ++ j)
			{
				uint32_t stage = j;
				if (ShaderObject::ST_VertexShader == type)
				{
					stage += D3DVERTEXTEXTURESAMPLER0;
				}

				SamplerPtr cur_sampler = cur_samplers_[j];
				SamplerPtr sampler = d3d9_shader_obj.Samplers(type)[j];
				if (!sampler || !sampler->GetTexture())
				{
					TIF(d3dDevice_->SetTexture(stage, NULL));

					if (cur_sampler)
					{
						if (cur_sampler->GetTexture() != TexturePtr())
						{
							cur_sampler->SetTexture(TexturePtr());
						}
					}
				}
				else
				{
					TexturePtr texture = sampler->GetTexture();

					if (cur_sampler->GetTexture() != sampler->GetTexture())
					{
						D3D9Texture const & d3d9Tex(*checked_pointer_cast<D3D9Texture>(texture));
						TIF(d3dDevice_->SetTexture(stage, d3d9Tex.D3DBaseTexture().get()));
						TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_SRGBTEXTURE, IsSRGB(texture->Format())));
						cur_sampler->SetTexture(texture);
					}

					if (cur_sampler->BorderColor() != sampler->BorderColor())
					{
						TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_BORDERCOLOR,
							D3D9Mapping::MappingToUInt32Color(sampler->BorderColor())));
						cur_sampler->BorderColor(sampler->BorderColor());
					}

					// Set addressing mode
					if (cur_sampler->AddressingMode(Sampler::TAT_Addr_U) != sampler->AddressingMode(Sampler::TAT_Addr_U))
					{
						TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_ADDRESSU,
							D3D9Mapping::Mapping(sampler->AddressingMode(Sampler::TAT_Addr_U))));
						cur_sampler->AddressingMode(Sampler::TAT_Addr_U, sampler->AddressingMode(Sampler::TAT_Addr_U));
					}
					if (cur_sampler->AddressingMode(Sampler::TAT_Addr_V) != sampler->AddressingMode(Sampler::TAT_Addr_V))
					{
						TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_ADDRESSV,
							D3D9Mapping::Mapping(sampler->AddressingMode(Sampler::TAT_Addr_V))));
						cur_sampler->AddressingMode(Sampler::TAT_Addr_V, sampler->AddressingMode(Sampler::TAT_Addr_V));
					}
					if (cur_sampler->AddressingMode(Sampler::TAT_Addr_W) != sampler->AddressingMode(Sampler::TAT_Addr_W))
					{
						TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_ADDRESSW,
							D3D9Mapping::Mapping(sampler->AddressingMode(Sampler::TAT_Addr_W))));
						cur_sampler->AddressingMode(Sampler::TAT_Addr_W, sampler->AddressingMode(Sampler::TAT_Addr_W));
					}

					if (cur_sampler->Filtering() != sampler->Filtering())
					{
						uint32_t tfc;
						switch (texture->Type())
						{
						case Texture::TT_1D:
							tfc = caps_.texture_1d_filter_caps;
							break;

						case Texture::TT_2D:
							tfc = caps_.texture_2d_filter_caps;
							break;

						case Texture::TT_3D:
							tfc = caps_.texture_3d_filter_caps;
							break;

						case Texture::TT_Cube:
							tfc = caps_.texture_cube_filter_caps;
							break;

						default:
							BOOST_ASSERT(false);
							tfc = 0;
							break;
						}

						Sampler::TexFilterOp filter = sampler->Filtering();
						if (Sampler::TFO_Anisotropic == filter)
						{
							if (0 == (tfc & Sampler::TFO_Anisotropic))
							{
								filter = Sampler::TFO_Trilinear;
							}
						}
						if (Sampler::TFO_Trilinear == filter)
						{
							if (0 == (tfc & Sampler::TFO_Trilinear))
							{
								filter = Sampler::TFO_Bilinear;
							}
						}
						if (Sampler::TFO_Bilinear == filter)
						{
							if (0 == (tfc & Sampler::TFO_Bilinear))
							{
								filter = Sampler::TFO_Point;
							}
						}
						if (Sampler::TFO_Point == filter)
						{
							if (0 == (tfc & Sampler::TFO_Point))
							{
								filter = Sampler::TFO_None;
							}
						}

						// Set filter
						switch (filter)
						{
						case Sampler::TFO_None:
							TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_NONE));
							TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_NONE));
							TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_NONE));
							break;

						case Sampler::TFO_Point:
							TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_POINT));
							TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_POINT));
							TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_POINT));
							break;

						case Sampler::TFO_Bilinear:
							TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
							TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
							TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_POINT));
							break;

						case Sampler::TFO_Trilinear:
							TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR));
							TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
							TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR));
							break;

						case Sampler::TFO_Anisotropic:
							TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC));
							TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR));
							TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR));
							break;
						}

						cur_sampler->Filtering(sampler->Filtering());
					}

					if (cur_sampler->Anisotropy() != sampler->Anisotropy())
					{
						// Set anisotropy
						TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MAXANISOTROPY,
							std::min(sampler->Anisotropy(), caps_.max_texture_anisotropy)));
						cur_sampler->Anisotropy(sampler->Anisotropy());
					}

					if (cur_sampler->MaxMipLevel() != sampler->MaxMipLevel())
					{
						// Set max mip level
						TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MAXMIPLEVEL, sampler->MaxMipLevel()));
						cur_sampler->MaxMipLevel(sampler->MaxMipLevel());
					}

					if (cur_sampler->MipMapLodBias() != sampler->MipMapLodBias())
					{
						// Set mip map lod bias
						TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MIPMAPLODBIAS, float_to_uint32(sampler->MipMapLodBias())));
						cur_sampler->MipMapLodBias(sampler->MipMapLodBias());
					}
				}
			}
		}
	}

	// 设置当前渲染目标
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::DoBindRenderTarget(RenderTargetPtr rt)
	{
		BOOST_ASSERT(d3dDevice_);
		BOOST_ASSERT(rt);
	}

	// 开始一帧
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::BeginFrame()
	{
		BOOST_ASSERT(d3dDevice_);

		TIF(d3dDevice_->BeginScene());
	}

	// 渲染
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::DoRender(RenderLayout const & rl)
	{
		if (rl.InstanceStream() && !rl.UseIndices())
		{
			this->DoRenderSWInstance(rl);
		}
		else
		{
			this->RenderInstance(rl);
		}
	}

	void D3D9RenderEngine::DoRenderSWInstance(RenderLayout const & rl)
	{
		BOOST_ASSERT(d3dDevice_);
		BOOST_ASSERT(rl.NumVertexStreams() != 0);
		
		uint32_t const num_instance = rl.NumInstance();

		if (num_instance > 1)
		{
			BOOST_ASSERT(rl.InstanceStream());

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			RenderLayoutPtr tmp_rl = rf.MakeRenderLayout(rl.Type());
			GraphicsBufferPtr tmp_inst_vs;

			rl.ExpandInstance(tmp_inst_vs, 0);

			for (uint32_t i = 0; i < rl.NumVertexStreams(); ++ i)
			{
				tmp_rl->BindVertexStream(rl.GetVertexStream(i), rl.VertexStreamFormat(i));
			}
			tmp_rl->BindVertexStream(tmp_inst_vs, rl.InstanceStreamFormat());
			if (rl.UseIndices())
			{
				tmp_rl->BindIndexStream(rl.GetIndexStream(), rl.IndexStreamFormat());
			}

			this->RenderRLSWInstance(*tmp_rl);

			for (uint32_t i = 1; i < num_instance; ++ i)
			{
				rl.ExpandInstance(tmp_inst_vs, i);
				this->RenderRLSWInstance(*tmp_rl);
			}
		}
		else
		{
			this->RenderRLSWInstance(rl);
		}
	}

	void D3D9RenderEngine::DoRenderHWInstance(RenderLayout const & rl)
	{
		uint32_t this_num_vertex_stream = rl.NumVertexStreams();
		for (uint32_t i = 0; i < rl.NumVertexStreams(); ++ i)
		{
			GraphicsBufferPtr stream = rl.GetVertexStream(i);

			D3D9VertexBuffer& d3d9vb(*checked_pointer_cast<D3D9VertexBuffer>(stream));
			TIF(d3dDevice_->SetStreamSource(i, d3d9vb.D3D9Buffer().get(), 0, rl.VertexSize(i)));

			TIF(d3dDevice_->SetStreamSourceFreq(i,
				D3DSTREAMSOURCE_INDEXEDDATA | rl.VertexStreamFrequency(i)));
		}
		if (rl.InstanceStream())
		{
			uint32_t number = rl.NumVertexStreams();
			GraphicsBufferPtr stream = rl.InstanceStream();

			D3D9VertexBuffer& d3d9vb(*checked_pointer_cast<D3D9VertexBuffer>(stream));
			TIF(d3dDevice_->SetStreamSource(number, d3d9vb.D3D9Buffer().get(), 0, rl.InstanceSize()));

			TIF(d3dDevice_->SetStreamSourceFreq(number,
				D3DSTREAMSOURCE_INSTANCEDATA | 1UL));

			++ this_num_vertex_stream;
		}

		for (uint32_t i = this_num_vertex_stream; i < last_num_vertex_stream_; ++ i)
		{
			TIF(d3dDevice_->SetStreamSource(i, NULL, 0, 0));
			TIF(d3dDevice_->SetStreamSourceFreq(i, 1UL));
		}

		last_num_vertex_stream_ = this_num_vertex_stream;

		this->RenderRL(rl);
	}

	void D3D9RenderEngine::RenderRLSWInstance(RenderLayout const & rl)
	{
		uint32_t this_num_vertex_stream = rl.NumVertexStreams();
		for (uint32_t i = 0; i < rl.NumVertexStreams(); ++ i)
		{
			GraphicsBufferPtr stream = rl.GetVertexStream(i);

			D3D9VertexBuffer& d3d9vb(*checked_pointer_cast<D3D9VertexBuffer>(stream));
			TIF(d3dDevice_->SetStreamSource(i, d3d9vb.D3D9Buffer().get(), 0, rl.VertexSize(i)));
		}

		for (uint32_t i = this_num_vertex_stream; i < last_num_vertex_stream_; ++ i)
		{
			TIF(d3dDevice_->SetStreamSource(i, NULL, 0, 0));
			TIF(d3dDevice_->SetStreamSourceFreq(i, 1UL));
		}

		last_num_vertex_stream_ = this_num_vertex_stream;

		this->RenderRL(rl);
	}

	void D3D9RenderEngine::RenderRL(RenderLayout const & rl)
	{
		D3DPRIMITIVETYPE primType;
		uint32_t primCount;
		D3D9Mapping::Mapping(primType, primCount, rl);

		numPrimitivesJustRendered_ += primCount;
		numVerticesJustRendered_ += rl.UseIndices() ? rl.NumIndices() : rl.NumVertices();

		D3D9RenderLayout const & d3d9_rl(*checked_cast<D3D9RenderLayout const *>(&rl));
		TIF(d3dDevice_->SetVertexDeclaration(d3d9_rl.VertexDeclaration().get()));

		uint32_t num_passes = render_tech_->NumPasses();
		if (rl.UseIndices())
		{
			D3D9IndexBuffer& d3dib(*checked_pointer_cast<D3D9IndexBuffer>(rl.GetIndexStream()));
			d3dib.SwitchFormat(rl.IndexStreamFormat());
			d3dDevice_->SetIndices(d3dib.D3D9Buffer().get());

			for (uint32_t i = 0; i < num_passes; ++ i)
			{
				RenderPassPtr pass = render_tech_->Pass(i);

				pass->Begin();
				TIF(d3dDevice_->DrawIndexedPrimitive(primType, 0, 0,
					static_cast<UINT>(rl.NumVertices()), 0, primCount));
				pass->End();
			}
		}
		else
		{
			d3dDevice_->SetIndices(NULL);

			for (uint32_t i = 0; i < num_passes; ++ i)
			{
				RenderPassPtr pass = render_tech_->Pass(i);

				pass->Begin();
				TIF(d3dDevice_->DrawPrimitive(primType, 0, primCount));
				pass->End();
			}
		}
	}

	// 结束一帧
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::EndFrame()
	{
		BOOST_ASSERT(d3dDevice_);

		TIF(d3dDevice_->EndScene());
	}

	// 获取模板位数
	/////////////////////////////////////////////////////////////////////////////////
	uint16_t D3D9RenderEngine::StencilBufferBitDepth()
	{
		BOOST_ASSERT(d3dDevice_);

		IDirect3DSurface9* surf;
		D3DSURFACE_DESC surfDesc;
		d3dDevice_->GetDepthStencilSurface(&surf);
		ID3D9SurfacePtr surf_ptr = MakeCOMPtr(surf);
		surf_ptr->GetDesc(&surfDesc);

		if (D3DFMT_D24S8 == surfDesc.Format)
		{
			return 8;
		}
		else
		{
			return 0;
		}
	}

	// 设置剪除矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::ScissorRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		RECT rc = { x, y, width, height };
		d3dDevice_->SetScissorRect(&rc);
	}

	// 填充设备能力
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::FillRenderDeviceCaps()
	{
		BOOST_ASSERT(d3dDevice_);

		D3DCAPS9 d3d_caps;
		d3dDevice_->GetDeviceCaps(&d3d_caps);

		caps_ = D3D9Mapping::Mapping(d3d_caps);

		for (size_t i = 0; i < caps_.max_textures_units; ++ i)
		{
			cur_samplers_.push_back(SamplerPtr(new Sampler));
		}
	}

	// 响应设备丢失
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::OnLostDevice()
	{
	}
	
	// 响应设备复位
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::OnResetDevice()
	{
		this->BindRenderTarget(cur_render_target_);
	}
}
