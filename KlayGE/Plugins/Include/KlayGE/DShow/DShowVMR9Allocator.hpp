// DShowVRM9Allocator.hpp
// KlayGE DirectShow VRM9分配器类 头文件
// Ver 3.4.0
// 版权所有(C) 龚敏敏, 2006
// Homepage: http://klayge.sourceforge.net
//
// 3.4.0
// 初次建立 (2006.7.15)
//
// 修改记录
/////////////////////////////////////////////////////////////////////

#ifndef _DSHOWVMR9ALLOCATOR_HPP
#define _DSHOWVMR9ALLOCATOR_HPP

#include <KlayGE/PreDeclare.hpp>

#include <d3d9.h>
#include <strmif.h>
#include <vmr9.h>
#include <vector>

#include <boost/thread/mutex.hpp>

#include <KlayGE/D3D9/D3D9Typedefs.hpp>

namespace KlayGE
{
	class DShowVMR9Allocator : public IVMRSurfaceAllocator9, IVMRImagePresenter9
	{
	public:
		enum
		{
			USER_ID = 0xBAFEDCBA
		};

	public:
		explicit DShowVMR9Allocator(HWND wnd);
		virtual~DShowVMR9Allocator();

		// IVMRSurfaceAllocator9
		virtual HRESULT STDMETHODCALLTYPE InitializeDevice( 
				/* [in] */ DWORD_PTR dwUserID,
				/* [in] */ VMR9AllocationInfo *lpAllocInfo,
				/* [out][in] */ DWORD *lpNumBuffers);
	            
		virtual HRESULT STDMETHODCALLTYPE TerminateDevice( 
			/* [in] */ DWORD_PTR dwID);
	    
		virtual HRESULT STDMETHODCALLTYPE GetSurface( 
			/* [in] */ DWORD_PTR dwUserID,
			/* [in] */ DWORD SurfaceIndex,
			/* [in] */ DWORD SurfaceFlags,
			/* [out] */ IDirect3DSurface9 **lplpSurface);
	    
		virtual HRESULT STDMETHODCALLTYPE AdviseNotify( 
			/* [in] */ IVMRSurfaceAllocatorNotify9 *lpIVMRSurfAllocNotify);

		// IVMRImagePresenter9
		virtual HRESULT STDMETHODCALLTYPE StartPresenting( 
			/* [in] */ DWORD_PTR dwUserID);
	    
		virtual HRESULT STDMETHODCALLTYPE StopPresenting( 
			/* [in] */ DWORD_PTR dwUserID);
	    
		virtual HRESULT STDMETHODCALLTYPE PresentImage( 
			/* [in] */ DWORD_PTR dwUserID,
			/* [in] */ VMR9PresentationInfo *lpPresInfo);
	    
		// IUnknown
		virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
			REFIID riid,
			void** ppvObject);

		virtual ULONG STDMETHODCALLTYPE AddRef();
		virtual ULONG STDMETHODCALLTYPE Release();

		TexturePtr PresentTexture();

	protected:
		// a helper function to erase every surface in the vector
		void DeleteSurfaces();
		void CreateDevice();

	private:
		// needed to make this a thread safe object
		boost::mutex	mutex_;
		HWND			wnd_;
		long			ref_count_;

		ID3D9Ptr			d3d_;
		ID3D9DevicePtr		d3d_device_;

		boost::shared_ptr<IVMRSurfaceAllocatorNotify9> vmr_surf_alloc_notify_;
		std::vector<IDirect3DSurface9*>	surfaces_;
		uint32_t cur_surf_index_;

		ID3D9SurfacePtr		cache_surf_;
		TexturePtr			present_tex_;

		D3DPRESENT_PARAMETERS d3dpp_;
	};
}

#endif		// _DSHOWVMR9ALLOCATOR_HPP
