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

#include <KlayGE/VFile.hpp>
#include <KlayGE/PackedFile/Pkt.hpp>

#pragma comment(lib, "KlayGE_FileSystem_PackedFile.lib")

namespace KlayGE
{
	// 把打包文件当普通文件夹读取的适配器
	//		只能读取
	/////////////////////////////////////////////////////////////////////////////////
 	class PackedFile : public VFile
	{
	public:
		PackedFile();
		PackedFile(const WString& pathName);

		bool Open(const WString& pathName);
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

		PackedFile(const PackedFile& rhs);
		PackedFile& operator=(const PackedFile& rhs);
	};
}

#endif			// _PACKEDFILE_HPP
