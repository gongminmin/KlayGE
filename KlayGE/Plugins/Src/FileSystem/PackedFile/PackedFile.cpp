// PackedFile.cpp
// KlayGE 打包文件虚拟适配器类 实现文件
// Ver 2.2.0
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.2.0
// 把大量代码抽象到基类 (2004.10.21)
//
// 2.0.0
// 初次建立 (2003.9.8)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>

#include <cassert>

#include <KlayGE/DiskFile/DiskFile.hpp>
#include <KlayGE/MemFile/MemFile.hpp>
#include <KlayGE/PackedFile/PackedFile.hpp>

namespace KlayGE
{
	PackedFile::PackedFile(std::string const & pathName)
				: VFile(OM_Read)
	{
		std::string::size_type const offset(pathName.rfind(".pkt/"));
		std::string const pktName(pathName.substr(0, offset + 4));
		std::string const fileName(pathName.substr(offset + 5));

		boost::shared_ptr<DiskFile> pktFile(new DiskFile(pktName, VFile::OM_Read));
		if (!pktFile->Fail())
		{
			pktFile_ = pktFile;

			unPkt_.Open(pktFile_);

			stream_ = boost::shared_ptr<std::iostream>(
				new std::stringstream(std::ios_base::in | std::ios_base::binary));

			unPkt_.LocateFile(fileName);

			std::vector<char> data(unPkt_.CurFileSize());
			unPkt_.ReadCurFile(&data[0]);

			stream_->write(&data[0], data.size());
			this->Rewind();
		}
	}

	size_t PackedFile::Read(void* data, size_t count)
	{
		assert(data != NULL);

		if (this->Tell() + count > this->Length())
		{
			count = this->Length() - this->Tell();
		}

		stream_->read(static_cast<char*>(data),
			static_cast<std::streamsize>(count));

		stream_->seekp(static_cast<std::istream::off_type>(count), std::ios_base::cur);

		return count;
	}

	void PackedFile::OnSeek(size_t offset, std::ios_base::seekdir from)
	{
		stream_->seekp(static_cast<std::iostream::off_type>(offset), from);
		stream_->seekg(static_cast<std::iostream::off_type>(offset), from);
	}


	PackedFileResIdentifier::PackedFileResIdentifier(std::string const & fileName)
		: fileName_(fileName)
	{
	}

	VFilePtr PackedFileResIdentifier::Load()
	{
		return VFilePtr(new PackedFile(fileName_));
	};
}
