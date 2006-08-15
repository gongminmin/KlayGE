// D3D9Resource.hpp
// KlayGE D3D9资源类 头文件
// Ver 2.4.0
// 版权所有(C) 龚敏敏, 2005
// Homepage: http://klayge.sourceforge.net
//
// 2.4.0
// 初次建立 (2005.3.2)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _D3D9RESOURCE_HPP
#define _D3D9RESOURCE_HPP

#define KLAYGE_LIB_NAME KlayGE_RenderEngine_D3D9
#include <KlayGE/config/auto_link.hpp>

#include <boost/smart_ptr.hpp>

namespace KlayGE
{
	class D3D9Resource
	{
	public:
		D3D9Resource()
			: reseted_(true)
		{
		}

		virtual ~D3D9Resource()
		{
		}

		void OnLostDevice()
		{
			if (reseted_)
			{
				this->DoOnLostDevice();
				reseted_ = false;
			}
		}
		void OnResetDevice()
		{
			if (!reseted_)
			{
				this->DoOnResetDevice();
				reseted_ = true;
			}
		}

	private:
		virtual void DoOnLostDevice() = 0;
		virtual void DoOnResetDevice() = 0;

	private:
		bool reseted_;
	};

	typedef boost::shared_ptr<D3D9Resource> D3D9ResourcePtr;
}

#endif			// _D3D9RESOURCE_HPP
