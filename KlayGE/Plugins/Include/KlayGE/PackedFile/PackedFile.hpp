// PackedFile.hpp
// KlayGE 打包文件类 头文件
// Ver 2.2.0
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.2.0
// 把大量代码抽象到基类 (2004.10.21)
//
// 2.0.0
// 初次建立 (2003.8.10)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _PACKEDFILE_HPP
#define _PACKEDFILE_HPP

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/VFile.hpp>
#include <KlayGE/PackedFile/Pkt.hpp>
#include <KlayGE/ResLocator.hpp>

#include <boost/utility.hpp>

#pragma comment(lib, "KlayGE_FileSystem_PackedFile.lib")

namespace KlayGE
{
	// 把打包文件当普通文件夹读取的适配器
	//		只能读取
	/////////////////////////////////////////////////////////////////////////////////
 	class PackedFile : boost::noncopyable, public VFile
	{
	public:
		PackedFile(std::string const & pathName);

		size_t Write(void const * /*data*/, size_t /*count*/)
			{ return 0; }
		size_t Read(void* data, size_t count);

		void OnSeek(size_t offset, std::ios_base::seekdir from);

	private:
		UnPkt		unPkt_;
		VFilePtr	pktFile_;
	};

	class PackedFileResIdentifier : public ResIdentifier
	{
	public:
		PackedFileResIdentifier(std::string const & fileName);

		VFilePtr Load();

	private:
		std::string fileName_;
	};
}

#endif			// _PACKEDFILE_HPP
