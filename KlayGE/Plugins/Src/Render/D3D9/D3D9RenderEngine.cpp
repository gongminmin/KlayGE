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
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderFactory.hpp>
#include <KlayGE/Viewport.hpp>
#include <KlayGE/GraphicsBuffer.hpp>
#include <KlayGE/RenderLayout.hpp>
#include <KlayGE/FrameBuffer.hpp>
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
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>

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
		cur_frame_buffer_.reset();

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

		FrameBuffer& fb = *this->CurFrameBuffer();
		while (WM_QUIT != msg.message)
		{
			// 如果窗口是激活的，用 PeekMessage()以便我们可以用空闲时间渲染场景
			// 不然, 用 GetMessage() 减少 CPU 占用率
			if (fb.Active())
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
				if (fb.Active())
				{
					Context::Instance().SceneManagerInstance().Update();
					fb.SwapBuffers();
				}
			}
		}
	}

	// 建立渲染窗口
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::CreateRenderWindow(std::string const & name,
		RenderSettings const & settings)
	{
		D3D9RenderWindowPtr win(new D3D9RenderWindow(d3d_, this->ActiveAdapter(),
			name, settings));
		default_frame_buffer_ = win;

		d3dDevice_ = win->D3DDevice();
		Verify(d3dDevice_ != ID3D9DevicePtr());

		this->FillRenderDeviceCaps();
		this->InitRenderStates();

		win->Attach(FrameBuffer::ATT_Color0, D3D9SurfaceRenderViewPtr(new D3D9SurfaceRenderView(win->D3DBackBuffer())));
		if (win->D3DDepthStencilBuffer())
		{
			win->Attach(FrameBuffer::ATT_DepthStencil, D3D9SurfaceRenderViewPtr(new D3D9SurfaceRenderView(win->D3DDepthStencilBuffer())));
		}

		this->BindFrameBuffer(win);

		if (caps_.hw_instancing_support)
		{
			RenderInstance = boost::bind(&D3D9RenderEngine::DoRenderHWInstance, this, _1, _2);
		}
		else
		{
			RenderInstance = boost::bind(&D3D9RenderEngine::DoRenderSWInstance, this, _1, _2);
		}
	}

	void D3D9RenderEngine::InitRenderStates()
	{
		d3dDevice_->SetRenderState(D3DRS_FILLMODE, D3D9Mapping::Mapping(cur_render_state_obj_.polygon_mode));
		d3dDevice_->SetRenderState(D3DRS_SHADEMODE, D3D9Mapping::Mapping(cur_render_state_obj_.shade_mode));
		d3dDevice_->SetRenderState(D3DRS_CULLMODE, D3D9Mapping::Mapping(cur_render_state_obj_.cull_mode));

		// NVIDIA's Transparency Multisampling
		if (S_OK == d3d_->CheckDeviceFormat(D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE,
			static_cast<D3DFORMAT>(MakeFourCC<'A', 'T', 'O', 'C'>::value)))
		{
			if (cur_render_state_obj_.alpha_to_coverage_enable)
			{
				d3dDevice_->SetRenderState(D3DRS_ADAPTIVETESS_Y,
					static_cast<D3DFORMAT>(MakeFourCC<'A', 'T', 'O', 'C'>::value));
			}
			else
			{
				d3dDevice_->SetRenderState(D3DRS_ADAPTIVETESS_Y, D3DFMT_UNKNOWN);
			}
		}
		d3dDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, cur_render_state_obj_.blend_enable);
		d3dDevice_->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, cur_render_state_obj_.blend_enable);		
		d3dDevice_->SetRenderState(D3DRS_BLENDOP, D3D9Mapping::Mapping(cur_render_state_obj_.blend_op));
		d3dDevice_->SetRenderState(D3DRS_SRCBLEND, D3D9Mapping::Mapping(cur_render_state_obj_.src_blend));
		d3dDevice_->SetRenderState(D3DRS_DESTBLEND, D3D9Mapping::Mapping(cur_render_state_obj_.dest_blend));
		d3dDevice_->SetRenderState(D3DRS_BLENDOPALPHA, D3D9Mapping::Mapping(cur_render_state_obj_.blend_op_alpha));
		d3dDevice_->SetRenderState(D3DRS_SRCBLENDALPHA, D3D9Mapping::Mapping(cur_render_state_obj_.src_blend_alpha));
		d3dDevice_->SetRenderState(D3DRS_DESTBLENDALPHA, D3D9Mapping::Mapping(cur_render_state_obj_.dest_blend_alpha));

		d3dDevice_->SetRenderState(D3DRS_ZENABLE, cur_render_state_obj_.depth_enable ? D3DZB_TRUE : D3DZB_FALSE);
		d3dDevice_->SetRenderState(D3DRS_ZWRITEENABLE, cur_render_state_obj_.depth_mask ? D3DZB_TRUE : D3DZB_FALSE);
		d3dDevice_->SetRenderState(D3DRS_ZFUNC, D3D9Mapping::Mapping(cur_render_state_obj_.depth_func));
		d3dDevice_->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, float_to_uint32(cur_render_state_obj_.polygon_offset_factor));
		d3dDevice_->SetRenderState(D3DRS_DEPTHBIAS, float_to_uint32(cur_render_state_obj_.polygon_offset_units));

		if (cur_render_state_obj_.front_stencil_enable && cur_render_state_obj_.back_stencil_enable)
		{
			d3dDevice_->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, true);
		}
		else
		{
			if (cur_render_state_obj_.front_stencil_enable)
			{
				d3dDevice_->SetRenderState(D3DRS_STENCILENABLE, true);
			}
			else
			{
				if (cur_render_state_obj_.back_stencil_enable)
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
		d3dDevice_->SetRenderState(D3DRS_STENCILFUNC, D3D9Mapping::Mapping(cur_render_state_obj_.front_stencil_func));
		d3dDevice_->SetRenderState(D3DRS_STENCILREF, cur_render_state_obj_.front_stencil_ref);
		d3dDevice_->SetRenderState(D3DRS_STENCILMASK, cur_render_state_obj_.front_stencil_mask);
		d3dDevice_->SetRenderState(D3DRS_STENCILFAIL, D3D9Mapping::Mapping(cur_render_state_obj_.front_stencil_fail));
		d3dDevice_->SetRenderState(D3DRS_STENCILZFAIL, D3D9Mapping::Mapping(cur_render_state_obj_.front_stencil_depth_fail));
		d3dDevice_->SetRenderState(D3DRS_STENCILPASS, D3D9Mapping::Mapping(cur_render_state_obj_.front_stencil_pass));
		d3dDevice_->SetRenderState(D3DRS_STENCILWRITEMASK, cur_render_state_obj_.front_stencil_write_mask);

		d3dDevice_->SetRenderState(D3DRS_CCW_STENCILFUNC, D3D9Mapping::Mapping(cur_render_state_obj_.back_stencil_func));	
		d3dDevice_->SetRenderState(D3DRS_STENCILREF, cur_render_state_obj_.back_stencil_ref);
		d3dDevice_->SetRenderState(D3DRS_STENCILMASK, cur_render_state_obj_.back_stencil_ref);
		d3dDevice_->SetRenderState(D3DRS_CCW_STENCILFAIL, D3D9Mapping::Mapping(cur_render_state_obj_.back_stencil_fail));
		d3dDevice_->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3D9Mapping::Mapping(cur_render_state_obj_.back_stencil_depth_fail));
		d3dDevice_->SetRenderState(D3DRS_CCW_STENCILPASS, D3D9Mapping::Mapping(cur_render_state_obj_.back_stencil_pass));
		d3dDevice_->SetRenderState(D3DRS_STENCILWRITEMASK, cur_render_state_obj_.back_stencil_write_mask);

		d3dDevice_->SetRenderState(D3DRS_SCISSORTESTENABLE, cur_render_state_obj_.scissor_enable);

		d3dDevice_->SetRenderState(D3DRS_COLORWRITEENABLE, D3D9Mapping::MappingColorMask(cur_render_state_obj_.color_mask_0));
		d3dDevice_->SetRenderState(D3DRS_COLORWRITEENABLE1, D3D9Mapping::MappingColorMask(cur_render_state_obj_.color_mask_1));
		d3dDevice_->SetRenderState(D3DRS_COLORWRITEENABLE2, D3D9Mapping::MappingColorMask(cur_render_state_obj_.color_mask_2));
		d3dDevice_->SetRenderState(D3DRS_COLORWRITEENABLE3, D3D9Mapping::MappingColorMask(cur_render_state_obj_.color_mask_3));
	}

	// 设置当前渲染状态对象
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::SetStateObjects(RenderStateObject const & rs_obj, ShaderObject const & shader_obj)
	{
		{
			if (cur_render_state_obj_.polygon_mode != rs_obj.polygon_mode)
			{
				d3dDevice_->SetRenderState(D3DRS_FILLMODE, D3D9Mapping::Mapping(rs_obj.polygon_mode));
			}
			if (cur_render_state_obj_.shade_mode != rs_obj.shade_mode)
			{
				d3dDevice_->SetRenderState(D3DRS_SHADEMODE, D3D9Mapping::Mapping(rs_obj.shade_mode));
			}
			if (cur_render_state_obj_.cull_mode != rs_obj.cull_mode)
			{
				d3dDevice_->SetRenderState(D3DRS_CULLMODE, D3D9Mapping::Mapping(rs_obj.cull_mode));
			}			

			if (cur_render_state_obj_.alpha_to_coverage_enable != rs_obj.alpha_to_coverage_enable)
			{
				// NVIDIA's Transparency Multisampling
				if (S_OK == d3d_->CheckDeviceFormat(D3DADAPTER_DEFAULT,
					D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE,
					static_cast<D3DFORMAT>(MakeFourCC<'A', 'T', 'O', 'C'>::value)))
				{
					if (rs_obj.alpha_to_coverage_enable)
					{
						d3dDevice_->SetRenderState(D3DRS_ADAPTIVETESS_Y,
							static_cast<D3DFORMAT>(MakeFourCC<'A', 'T', 'O', 'C'>::value));
					}
					else
					{
						d3dDevice_->SetRenderState(D3DRS_ADAPTIVETESS_Y, D3DFMT_UNKNOWN);
					}
				}
			}
			if (cur_render_state_obj_.blend_enable != rs_obj.blend_enable)
			{
				d3dDevice_->SetRenderState(D3DRS_ALPHABLENDENABLE, rs_obj.blend_enable);
				d3dDevice_->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, rs_obj.blend_enable);		
			}
			if (cur_render_state_obj_.blend_op != rs_obj.blend_op)
			{
				d3dDevice_->SetRenderState(D3DRS_BLENDOP, D3D9Mapping::Mapping(rs_obj.blend_op));
			}
			if (cur_render_state_obj_.src_blend != rs_obj.src_blend)
			{
				d3dDevice_->SetRenderState(D3DRS_SRCBLEND, D3D9Mapping::Mapping(rs_obj.src_blend));
			}
			if (cur_render_state_obj_.dest_blend != rs_obj.dest_blend)
			{
				d3dDevice_->SetRenderState(D3DRS_DESTBLEND, D3D9Mapping::Mapping(rs_obj.dest_blend));
			}
			if (cur_render_state_obj_.blend_op_alpha != rs_obj.blend_op_alpha)
			{
				d3dDevice_->SetRenderState(D3DRS_BLENDOPALPHA, D3D9Mapping::Mapping(rs_obj.blend_op_alpha));
			}
			if (cur_render_state_obj_.src_blend_alpha != rs_obj.src_blend_alpha)
			{
				d3dDevice_->SetRenderState(D3DRS_SRCBLENDALPHA, D3D9Mapping::Mapping(rs_obj.src_blend_alpha));
			}
			if (cur_render_state_obj_.dest_blend_alpha != rs_obj.dest_blend_alpha)
			{
				d3dDevice_->SetRenderState(D3DRS_DESTBLENDALPHA, D3D9Mapping::Mapping(rs_obj.dest_blend_alpha));
			}

			if (cur_render_state_obj_.depth_enable != rs_obj.depth_enable)
			{
				d3dDevice_->SetRenderState(D3DRS_ZENABLE, rs_obj.depth_enable ? D3DZB_TRUE : D3DZB_FALSE);
			}
			if (cur_render_state_obj_.depth_mask != rs_obj.depth_mask)
			{
				d3dDevice_->SetRenderState(D3DRS_ZWRITEENABLE, rs_obj.depth_mask ? D3DZB_TRUE : D3DZB_FALSE);
			}
			if (cur_render_state_obj_.depth_func != rs_obj.depth_func)
			{
				d3dDevice_->SetRenderState(D3DRS_ZFUNC, D3D9Mapping::Mapping(rs_obj.depth_func));
			}
			if (cur_render_state_obj_.polygon_offset_factor != rs_obj.polygon_offset_factor)
			{
				d3dDevice_->SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, float_to_uint32(rs_obj.polygon_offset_factor));
			}
			if (cur_render_state_obj_.polygon_offset_units != rs_obj.polygon_offset_units)
			{
				d3dDevice_->SetRenderState(D3DRS_DEPTHBIAS, float_to_uint32(rs_obj.polygon_offset_units));
			}

			if ((cur_render_state_obj_.front_stencil_enable != rs_obj.front_stencil_enable)
				|| (cur_render_state_obj_.back_stencil_enable != rs_obj.back_stencil_enable))
			{
				if (rs_obj.front_stencil_enable && rs_obj.back_stencil_enable)
				{
					d3dDevice_->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, true);
				}
				else
				{
					if (rs_obj.front_stencil_enable)
					{
						d3dDevice_->SetRenderState(D3DRS_STENCILENABLE, true);
					}
					else
					{
						if (rs_obj.back_stencil_enable)
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
			}
			if (cur_render_state_obj_.front_stencil_func != rs_obj.front_stencil_func)
			{
				d3dDevice_->SetRenderState(D3DRS_STENCILFUNC, D3D9Mapping::Mapping(rs_obj.front_stencil_func));
			}
			if (cur_render_state_obj_.front_stencil_ref != rs_obj.front_stencil_ref)
			{
				d3dDevice_->SetRenderState(D3DRS_STENCILREF, rs_obj.front_stencil_ref);
			}
			if (cur_render_state_obj_.front_stencil_mask != rs_obj.front_stencil_mask)
			{
				d3dDevice_->SetRenderState(D3DRS_STENCILMASK, rs_obj.front_stencil_mask);
			}
			if (cur_render_state_obj_.front_stencil_fail != rs_obj.front_stencil_fail)
			{
				d3dDevice_->SetRenderState(D3DRS_STENCILFAIL, D3D9Mapping::Mapping(rs_obj.front_stencil_fail));
			}
			if (cur_render_state_obj_.front_stencil_depth_fail != rs_obj.front_stencil_depth_fail)
			{
				d3dDevice_->SetRenderState(D3DRS_STENCILZFAIL, D3D9Mapping::Mapping(rs_obj.front_stencil_depth_fail));
			}
			if (cur_render_state_obj_.front_stencil_pass != rs_obj.front_stencil_pass)
			{
				d3dDevice_->SetRenderState(D3DRS_STENCILPASS, D3D9Mapping::Mapping(rs_obj.front_stencil_pass));
			}
			if (cur_render_state_obj_.front_stencil_write_mask != rs_obj.front_stencil_write_mask)
			{
				d3dDevice_->SetRenderState(D3DRS_STENCILWRITEMASK, rs_obj.front_stencil_write_mask);
			}

			if (cur_render_state_obj_.back_stencil_func != rs_obj.back_stencil_func)
			{
				d3dDevice_->SetRenderState(D3DRS_CCW_STENCILFUNC, D3D9Mapping::Mapping(rs_obj.back_stencil_func));	
			}
			if (cur_render_state_obj_.back_stencil_ref != rs_obj.back_stencil_ref)
			{
				d3dDevice_->SetRenderState(D3DRS_STENCILREF, rs_obj.back_stencil_ref);
			}
			if (cur_render_state_obj_.back_stencil_mask != rs_obj.back_stencil_mask)
			{
				d3dDevice_->SetRenderState(D3DRS_STENCILMASK, rs_obj.back_stencil_ref);
			}
			if (cur_render_state_obj_.back_stencil_fail != rs_obj.back_stencil_fail)
			{
				d3dDevice_->SetRenderState(D3DRS_CCW_STENCILFAIL, D3D9Mapping::Mapping(rs_obj.back_stencil_fail));
			}
			if (cur_render_state_obj_.back_stencil_depth_fail != rs_obj.back_stencil_depth_fail)
			{
				d3dDevice_->SetRenderState(D3DRS_CCW_STENCILZFAIL, D3D9Mapping::Mapping(rs_obj.back_stencil_depth_fail));
			}
			if (cur_render_state_obj_.back_stencil_pass != rs_obj.back_stencil_pass)
			{
				d3dDevice_->SetRenderState(D3DRS_CCW_STENCILPASS, D3D9Mapping::Mapping(rs_obj.back_stencil_pass));
			}
			if (cur_render_state_obj_.back_stencil_write_mask != rs_obj.back_stencil_write_mask)
			{
				d3dDevice_->SetRenderState(D3DRS_STENCILWRITEMASK, rs_obj.back_stencil_write_mask);
			}

			if (cur_render_state_obj_.scissor_enable != rs_obj.scissor_enable)
			{
				d3dDevice_->SetRenderState(D3DRS_SCISSORTESTENABLE, rs_obj.scissor_enable);
			}

			if (cur_render_state_obj_.color_mask_0 != rs_obj.color_mask_0)
			{
				d3dDevice_->SetRenderState(D3DRS_COLORWRITEENABLE, D3D9Mapping::MappingColorMask(rs_obj.color_mask_0));
			}
			if (cur_render_state_obj_.color_mask_1 != rs_obj.color_mask_1)
			{
				d3dDevice_->SetRenderState(D3DRS_COLORWRITEENABLE1, D3D9Mapping::MappingColorMask(rs_obj.color_mask_1));
			}
			if (cur_render_state_obj_.color_mask_2 != rs_obj.color_mask_2)
			{
				d3dDevice_->SetRenderState(D3DRS_COLORWRITEENABLE2, D3D9Mapping::MappingColorMask(rs_obj.color_mask_2));
			}
			if (cur_render_state_obj_.color_mask_3 != rs_obj.color_mask_3)
			{
				d3dDevice_->SetRenderState(D3DRS_COLORWRITEENABLE3, D3D9Mapping::MappingColorMask(rs_obj.color_mask_3));
			}

			cur_render_state_obj_ = rs_obj;
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

				Sampler& cur_sampler = cur_samplers_[type][j];
				SamplerPtr sampler = d3d9_shader_obj.Samplers(type)[j];
				if (!sampler || !sampler->texture)
				{
					if (cur_sampler.texture)
					{
						TIF(d3dDevice_->SetTexture(stage, NULL));
						cur_sampler.texture.reset();
					}
				}
				else
				{
					if (cur_sampler.texture != sampler->texture)
					{
						D3D9Texture const & d3d9Tex(*checked_pointer_cast<D3D9Texture>(sampler->texture));
						TIF(d3dDevice_->SetTexture(stage, d3d9Tex.D3DBaseTexture().get()));
						TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_SRGBTEXTURE, IsSRGB(sampler->texture->Format())));
					}

					if (cur_sampler.border_clr != sampler->border_clr)
					{
						TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_BORDERCOLOR,
							D3D9Mapping::MappingToUInt32Color(sampler->border_clr)));
					}

					// Set addressing mode
					if (cur_sampler.addr_mode_u != sampler->addr_mode_u)
					{
						TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_ADDRESSU,
							D3D9Mapping::Mapping(sampler->addr_mode_u)));
					}
					if (cur_sampler.addr_mode_v != sampler->addr_mode_v)
					{
						TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_ADDRESSV,
							D3D9Mapping::Mapping(sampler->addr_mode_v)));
					}
					if (cur_sampler.addr_mode_w != sampler->addr_mode_w)
					{
						TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_ADDRESSW,
							D3D9Mapping::Mapping(sampler->addr_mode_w)));
					}

					if (cur_sampler.filter != sampler->filter)
					{
						switch (sampler->filter)
						{
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
					}

					if (cur_sampler.anisotropy != sampler->anisotropy)
					{
						TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MAXANISOTROPY, sampler->anisotropy));
					}

					if (cur_sampler.max_mip_level != sampler->max_mip_level)
					{
						TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MAXMIPLEVEL, sampler->max_mip_level));
					}

					if (cur_sampler.mip_map_lod_bias != sampler->mip_map_lod_bias)
					{
						TIF(d3dDevice_->SetSamplerState(stage, D3DSAMP_MIPMAPLODBIAS, float_to_uint32(sampler->mip_map_lod_bias)));
					}

					cur_sampler = *sampler;
				}
			}
		}
	}

	// 设置当前渲染目标
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::DoBindFrameBuffer(FrameBufferPtr fb)
	{
		BOOST_ASSERT(d3dDevice_);
		BOOST_ASSERT(fb);
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
	void D3D9RenderEngine::DoRender(RenderTechnique const & tech, RenderLayout const & rl)
	{
		if (rl.InstanceStream() && !rl.UseIndices())
		{
			this->DoRenderSWInstance(tech, rl);
		}
		else
		{
			this->RenderInstance(tech, rl);
		}
	}

	void D3D9RenderEngine::DoRenderSWInstance(RenderTechnique const & tech, RenderLayout const & rl)
	{
		BOOST_ASSERT(d3dDevice_);
		BOOST_ASSERT(rl.NumVertexStreams() != 0);

		uint32_t const num_instance = rl.NumInstance();

		if (num_instance > 1)
		{
			BOOST_ASSERT(rl.InstanceStream());

			RenderFactory& rf = Context::Instance().RenderFactoryInstance();

			RenderLayoutPtr tmp_rl = rf.MakeRenderLayout();
			tmp_rl->TopologyType(rl.TopologyType());
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

			this->RenderRLSWInstance(tech, *tmp_rl);

			for (uint32_t i = 1; i < num_instance; ++ i)
			{
				rl.ExpandInstance(tmp_inst_vs, i);
				this->RenderRLSWInstance(tech, *tmp_rl);
			}
		}
		else
		{
			this->RenderRLSWInstance(tech, rl);
		}
	}

	void D3D9RenderEngine::DoRenderHWInstance(RenderTechnique const & tech, RenderLayout const & rl)
	{
		uint32_t this_num_vertex_stream = rl.NumVertexStreams();
		for (uint32_t i = 0; i < rl.NumVertexStreams(); ++ i)
		{
			GraphicsBufferPtr stream = rl.GetVertexStream(i);

			D3D9VertexBuffer& d3d9vb(*checked_pointer_cast<D3D9VertexBuffer>(stream));
			TIF(d3dDevice_->SetStreamSource(i, d3d9vb.D3D9Buffer().get(), 0, rl.VertexSize(i)));
		}
		if (rl.InstanceStream())
		{
			for (uint32_t i = 0; i < rl.NumVertexStreams(); ++ i)
			{
				TIF(d3dDevice_->SetStreamSourceFreq(i,
					D3DSTREAMSOURCE_INDEXEDDATA | rl.VertexStreamFrequency(i)));
			}

			uint32_t number = rl.NumVertexStreams();
			GraphicsBufferPtr stream = rl.InstanceStream();

			D3D9VertexBuffer& d3d9vb(*checked_pointer_cast<D3D9VertexBuffer>(stream));
			TIF(d3dDevice_->SetStreamSource(number, d3d9vb.D3D9Buffer().get(), 0, rl.InstanceSize()));

			TIF(d3dDevice_->SetStreamSourceFreq(number,
				D3DSTREAMSOURCE_INSTANCEDATA | 1UL));

			++ this_num_vertex_stream;
		}
		else
		{
			for (uint32_t i = 0; i < rl.NumVertexStreams() + 1; ++ i)
			{
				TIF(d3dDevice_->SetStreamSourceFreq(i, 1UL));
			}
		}

		for (uint32_t i = this_num_vertex_stream; i < last_num_vertex_stream_; ++ i)
		{
			TIF(d3dDevice_->SetStreamSource(i, NULL, 0, 0));
			TIF(d3dDevice_->SetStreamSourceFreq(i, 1UL));
		}

		last_num_vertex_stream_ = this_num_vertex_stream;

		this->RenderRL(tech, rl);
	}

	void D3D9RenderEngine::RenderRLSWInstance(RenderTechnique const & tech, RenderLayout const & rl)
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

		this->RenderRL(tech, rl);
	}

	void D3D9RenderEngine::RenderRL(RenderTechnique const & tech, RenderLayout const & rl)
	{
		D3DPRIMITIVETYPE primType;
		uint32_t primCount;
		D3D9Mapping::Mapping(primType, primCount, rl);

		numPrimitivesJustRendered_ += primCount;
		numVerticesJustRendered_ += rl.UseIndices() ? rl.NumIndices() : rl.NumVertices();

		D3D9RenderLayout const & d3d9_rl(*checked_cast<D3D9RenderLayout const *>(&rl));
		TIF(d3dDevice_->SetVertexDeclaration(d3d9_rl.VertexDeclaration().get()));

		uint32_t num_passes = tech.NumPasses();
		if (rl.UseIndices())
		{
			D3D9IndexBuffer& d3dib(*checked_pointer_cast<D3D9IndexBuffer>(rl.GetIndexStream()));
			d3dib.SwitchFormat(rl.IndexStreamFormat());
			d3dDevice_->SetIndices(d3dib.D3D9Buffer().get());

			for (uint32_t i = 0; i < num_passes; ++ i)
			{
				tech.Pass(i)->Apply();

				TIF(d3dDevice_->DrawIndexedPrimitive(primType, 0, 0,
					static_cast<UINT>(rl.NumVertices()), 0, primCount));
			}
		}
		else
		{
			d3dDevice_->SetIndices(NULL);

			for (uint32_t i = 0; i < num_passes; ++ i)
			{
				tech.Pass(i)->Apply();

				TIF(d3dDevice_->DrawPrimitive(primType, 0, primCount));
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

	void D3D9RenderEngine::Resize(uint32_t width, uint32_t height)
	{
		checked_pointer_cast<D3D9RenderWindow>(default_frame_buffer_)->Resize(width, height);
	}

	bool D3D9RenderEngine::FullScreen() const
	{
		return checked_pointer_cast<D3D9RenderWindow>(default_frame_buffer_)->FullScreen();
	}

	void D3D9RenderEngine::FullScreen(bool fs)
	{
		checked_pointer_cast<D3D9RenderWindow>(default_frame_buffer_)->FullScreen(fs);
	}

	// 填充设备能力
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::FillRenderDeviceCaps()
	{
		BOOST_ASSERT(d3dDevice_);

		D3DCAPS9 d3d_caps;
		d3dDevice_->GetDeviceCaps(&d3d_caps);

		caps_ = D3D9Mapping::Mapping(d3d_caps);

		cur_samplers_[ShaderObject::ST_VertexShader].resize(caps_.max_vertex_texture_units);
		cur_samplers_[ShaderObject::ST_PixelShader].resize(caps_.max_texture_units);
	}

	// 响应设备丢失
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::OnLostDevice()
	{
		cur_render_state_obj_ = RenderStateObject();
		BOOST_FOREACH(BOOST_TYPEOF(cur_samplers_)::reference samplers, cur_samplers_)
		{
			BOOST_FOREACH(BOOST_TYPEOF(samplers)::reference sampler, samplers)
			{
				sampler = Sampler();
			}
		}
	}

	// 响应设备复位
	/////////////////////////////////////////////////////////////////////////////////
	void D3D9RenderEngine::OnResetDevice()
	{
		this->BindFrameBuffer(cur_frame_buffer_);
	}
}
