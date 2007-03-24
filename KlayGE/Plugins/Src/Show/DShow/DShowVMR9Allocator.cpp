// DShowVRM9Allocator.cpp
// KlayGE DirectShow VRM9分配器类 实现文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.4.0
// 初次建立 (2006.7.15)
//
// 修改记录
/////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/COMPtr.hpp>
#include <KlayGE/ElementFormat.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/Texture.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <d3d9.h>
#include <boost/assert.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable: 4127 4800)
#endif
#include <boost/pool/pool_alloc.hpp>
#ifdef KLAYGE_COMPILER_MSVC
#pragma warning(pop)
#endif

#include <KlayGE/D3D9/D3D9Mapping.hpp>
#include <KlayGE/D3D9/D3D9Texture.hpp>
#include <KlayGE/DShow/DShowVMR9Allocator.hpp>

namespace KlayGE
{
	DShowVMR9Allocator::DShowVMR9Allocator(HWND wnd)
					: ref_count_(1),
						wnd_(wnd),
						cur_surf_index_(0xFFFFFFFF)
	{
		d3d_ = MakeCOMPtr(::Direct3DCreate9(D3D_SDK_VERSION));

		this->CreateDevice();
	}

	DShowVMR9Allocator::~DShowVMR9Allocator()
	{
		this->DeleteSurfaces();
	}

	void DShowVMR9Allocator::CreateDevice()
	{
		D3DDISPLAYMODE dm;
		d3d_->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &dm);

		memset(&d3dpp_, 0, sizeof(d3dpp_));
		d3dpp_.Windowed = true;
		d3dpp_.hDeviceWindow = wnd_;
		d3dpp_.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp_.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

		IDirect3DDevice9* d3d_device;
		d3d_->CreateDevice(D3DADAPTER_DEFAULT,
								D3DDEVTYPE_HAL,
								wnd_,
								D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_PUREDEVICE | D3DCREATE_MULTITHREADED,
								&d3dpp_,
								&d3d_device);
		d3d_device_ = MakeCOMPtr(d3d_device);
	}

	void DShowVMR9Allocator::DeleteSurfaces()
	{
		boost::mutex::scoped_lock lock(mutex_);

		// clear out the private texture
		cache_surf_.reset();

		for (size_t i = 0; i < surfaces_.size(); ++ i) 
		{
			if (surfaces_[i] != NULL)
			{
				surfaces_[i]->Release();
				surfaces_[i] = NULL;
			}
		}
	}


	//IVMRSurfaceAllocator9
	HRESULT DShowVMR9Allocator::InitializeDevice(DWORD_PTR dwUserID,
				VMR9AllocationInfo* lpAllocInfo, DWORD* lpNumBuffers)
	{
		if (dwUserID != USER_ID)
		{
			return S_OK;
		}

		if (NULL == lpNumBuffers)
		{
			return E_POINTER;
		}

		if (!vmr_surf_alloc_notify_)
		{
			return E_FAIL;
		}

		HRESULT hr = S_OK;

		// NOTE:
		// we need to make sure that we create textures because
		// surfaces can not be textured onto a primitive.
		lpAllocInfo->dwFlags |= VMR9AllocFlag_TextureSurface;

		this->DeleteSurfaces();
		surfaces_.resize(*lpNumBuffers);
		hr = vmr_surf_alloc_notify_->AllocateSurfaceHelper(lpAllocInfo, lpNumBuffers, &surfaces_[0]);

		// If we couldn't create a texture surface and 
		// the format is not an alpha format,
		// then we probably cannot create a texture.
		// So what we need to do is create a private texture
		// and copy the decoded images onto it.
		if (FAILED(hr) && !(lpAllocInfo->dwFlags & VMR9AllocFlag_3DRenderTarget))
		{
			this->DeleteSurfaces();            

			lpAllocInfo->dwFlags &= ~VMR9AllocFlag_TextureSurface;
			lpAllocInfo->dwFlags |= VMR9AllocFlag_OffscreenSurface;

			TIF(vmr_surf_alloc_notify_->AllocateSurfaceHelper(lpAllocInfo, lpNumBuffers, &surfaces_[0]));
		}


		RenderFactory& rf = Context::Instance().RenderFactoryInstance();
		present_tex_ = rf.MakeTexture2D(lpAllocInfo->dwWidth, lpAllocInfo->dwHeight, 1, EF_ARGB8);
		present_tex_->Usage(Texture::TU_RenderTarget);

		IDirect3DSurface9* surf;
		d3d_device_->CreateOffscreenPlainSurface(lpAllocInfo->dwWidth, lpAllocInfo->dwHeight,
			D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &surf, NULL);
		cache_surf_ = MakeCOMPtr(surf);

		return S_OK;
	}
	            
	HRESULT DShowVMR9Allocator::TerminateDevice(DWORD_PTR dwID)
	{
		if (dwID != USER_ID)
		{
			return S_OK;
		}

		this->DeleteSurfaces();
		return S_OK;
	}
	    
	HRESULT DShowVMR9Allocator::GetSurface(DWORD_PTR dwUserID,
			DWORD SurfaceIndex, DWORD /*SurfaceFlags*/, IDirect3DSurface9** lplpSurface)
	{
		if (dwUserID != USER_ID)
		{
			*lplpSurface = NULL;
			return S_OK;
		}

		if (NULL == lplpSurface)
		{
			return E_POINTER;
		}

		if (SurfaceIndex >= surfaces_.size()) 
		{
			return E_FAIL;
		}

		boost::mutex::scoped_lock lock(mutex_);

		*lplpSurface = surfaces_[SurfaceIndex];
		(*lplpSurface)->AddRef();
		return S_OK;
	}
	    
	HRESULT DShowVMR9Allocator::AdviseNotify(IVMRSurfaceAllocatorNotify9* lpIVMRSurfAllocNotify)
	{
		boost::mutex::scoped_lock lock(mutex_);

		vmr_surf_alloc_notify_ = MakeCOMPtr(lpIVMRSurfAllocNotify);
		vmr_surf_alloc_notify_->AddRef();

		HMONITOR hMonitor = d3d_->GetAdapterMonitor(D3DADAPTER_DEFAULT);
		TIF(vmr_surf_alloc_notify_->SetD3DDevice(d3d_device_.get(), hMonitor));

		return S_OK;
	}

	HRESULT DShowVMR9Allocator::StartPresenting(DWORD_PTR dwUserID)
	{
		if (dwUserID != USER_ID)
		{
			return S_OK;
		}

		boost::mutex::scoped_lock lock(mutex_);

		if (!d3d_device_)
		{
			return E_FAIL;
		}

		return S_OK;
	}

	HRESULT DShowVMR9Allocator::StopPresenting(DWORD_PTR dwUserID)
	{
		if (dwUserID != USER_ID)
		{
			return S_OK;
		}

		return S_OK;
	}

	HRESULT DShowVMR9Allocator::PresentImage(DWORD_PTR dwUserID, VMR9PresentationInfo* lpPresInfo)
	{
		if (dwUserID != USER_ID)
		{
			return S_OK;
		}

		// parameter validation
		if (NULL == lpPresInfo)
		{
			return E_POINTER;
		}
		else
		{
			if (NULL == lpPresInfo->lpSurf)
			{
				return E_POINTER;
			}
		}

		boost::mutex::scoped_lock lock(mutex_);

		cur_surf_index_ = 0xFFFFFFFF;
		for (uint32_t i = 0; i < surfaces_.size(); ++ i)
		{
			if (surfaces_[i] == lpPresInfo->lpSurf)
			{
				cur_surf_index_ = i;
				break;
			}
		}

		if (D3DERR_DEVICENOTRESET == d3d_device_->TestCooperativeLevel())
		{
			cache_surf_.reset();

			for (size_t i = 0; i < surfaces_.size(); ++ i) 
			{
				if (surfaces_[i] != NULL)
				{
					surfaces_[i]->Release();
					surfaces_[i] = NULL;
				}
			}

			this->CreateDevice();

			HMONITOR hMonitor = d3d_->GetAdapterMonitor(D3DADAPTER_DEFAULT);
			TIF(vmr_surf_alloc_notify_->ChangeD3DDevice(d3d_device_.get(), hMonitor));
		}

		return S_OK;
	}

	// IUnknown
	HRESULT DShowVMR9Allocator::QueryInterface(REFIID riid, void** ppvObject)
	{
		HRESULT hr = E_NOINTERFACE;

		if (NULL == ppvObject)
		{
			hr = E_POINTER;
		} 
		else
		{
			if (IID_IVMRSurfaceAllocator9 == riid)
			{
				*ppvObject = static_cast<IVMRSurfaceAllocator9*>(this);
				this->AddRef();
				hr = S_OK;
			} 
			else
			{
				if (IID_IVMRImagePresenter9 == riid)
				{
					*ppvObject = static_cast<IVMRImagePresenter9*>(this);
					this->AddRef();
					hr = S_OK;
				} 
				else
				{
					if (IID_IUnknown == riid)
					{
						*ppvObject = static_cast<IUnknown*>(static_cast<IVMRSurfaceAllocator9*>(this));
						this->AddRef();
						hr = S_OK;    
					}
				}
			}
		}

		return hr;
	}

	ULONG DShowVMR9Allocator::AddRef()
	{
		return ::InterlockedIncrement(&ref_count_);
	}

	ULONG DShowVMR9Allocator::Release()
	{
		ULONG ret = ::InterlockedDecrement(&ref_count_);
		if (0 == ret)
		{
			delete this;
		}

		return ret;
	}

	TexturePtr DShowVMR9Allocator::PresentTexture()
	{
		boost::mutex::scoped_lock lock(mutex_);

		if (FAILED(d3d_device_->TestCooperativeLevel()))
		{
			return TexturePtr();
		}

		if (cur_surf_index_ < surfaces_.size())
		{
			TIF(d3d_device_->GetRenderTargetData(surfaces_[cur_surf_index_], cache_surf_.get()));

			D3DSURFACE_DESC desc;
			surfaces_[cur_surf_index_]->GetDesc(&desc);

			ElementFormat const ef = D3D9Mapping::MappingFormat(desc.Format);
			uint32_t const line_size = desc.Width * NumFormatBytes(ef);

			D3DLOCKED_RECT d3dlocked_rc;
			cache_surf_->LockRect(&d3dlocked_rc, NULL, D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY);
			uint8_t const * src = static_cast<uint8_t const *>(d3dlocked_rc.pBits);
			if (line_size == static_cast<uint32_t>(d3dlocked_rc.Pitch))
			{
				present_tex_->CopyMemoryToTexture2D(0, src, ef,
					present_tex_->Width(0), present_tex_->Height(0), 0, 0, desc.Width, desc.Height);
			}
			else
			{
				std::vector<uint8_t, boost::pool_allocator<uint8_t> > data(line_size * desc.Height);
				uint8_t* dst = &data[0];
				for (uint32_t y = 0; y < desc.Height; ++ y)
				{
					std::copy(src, src + line_size, dst);
					src += d3dlocked_rc.Pitch;
					dst += line_size;
				}

				present_tex_->CopyMemoryToTexture2D(0, &data[0], ef,
					present_tex_->Width(0), present_tex_->Height(0), 0, 0, desc.Width, desc.Height);
			}
			cache_surf_->UnlockRect();
		}

		return present_tex_;
	}
}
