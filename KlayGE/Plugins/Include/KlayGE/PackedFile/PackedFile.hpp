// PackedFile.hpp
// KlayGE 打包文件类 头文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
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
		PackedFile();
		PackedFile(const std::string& pathName);

		bool Open(const std::string& pathName);
		void Close();

		size_t Length();
		void Length(size_t /*newLen*/)
			{ }

		size_t Write(const void* /*data*/, size_t /*count*/)
			{ return 0; }
		size_t Read(void* data, size_t count);
		size_t CopyFrom(VFile& /*src*/, size_t /*size*/)
			{ return 0; }

		size_t Seek(size_t offset, SeekMode from);
		size_t Tell();

	private:
		UnPkt		unPkt_;
		VFilePtr	file_;

		VFilePtr	pktFile_;
		PackedFile& operator=(const PackedFile& rhs);
	};

	class PackedFileResIdentifier : public ResIdentifier
	{
	public:
		PackedFileResIdentifier(const std::string& fileName);

		VFilePtr Load();

	private:
		std::string fileName_;
	};
}

#endif			// _PACKEDFILE_HPP
