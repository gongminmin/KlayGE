// DiskFile.cpp
// KlayGE 磁盘文件类 实现文件
// Ver 2.2.0
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.2.0
// 把大量代码抽象到基类 (2004.10.21)
//
// 2.1.2
// 修正了Seek的bug (2004.8.11)
//
// 2.0.4
// 修正了Read的bug (2004.3.22)
//
// 2.0.0
// 初次建立 (2003.8.3)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>

#include <vector>
#include <cassert>

#include <KlayGE/DiskFile/DiskFile.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	DiskFile::DiskFile(std::string const & fileName, OpenMode openMode)
				: VFile(openMode)
	{
		std::ios_base::openmode mode(std::ios_base::binary);

		switch (openMode)
		{
		case OM_Read:
			mode |= std::ios_base::in;
			break;

		case OM_Write:
			mode |= std::ios_base::out;
			break;

		case OM_ReadWrite:
			mode |= std::ios_base::in | std::ios_base::out;
			break;
			
		case OM_Create:
			mode |= std::ios_base::out | std::ios_base::trunc;
			break;
		}

		fileName_ = fileName;
		stream_ = boost::shared_ptr<std::iostream>(new std::fstream(fileName_.c_str(), mode));
	}

	// 把数据写入文件
	/////////////////////////////////////////////////////////////////////////////////
	size_t DiskFile::Write(void const * data, size_t count)
	{
		assert(stream_);
		assert(data != NULL);

		stream_->write(static_cast<char const *>(data),
			static_cast<std::streamsize>(count));

		return count;
	}

	// 从文件读出数据
	/////////////////////////////////////////////////////////////////////////////////
	size_t DiskFile::Read(void* data, size_t count)
	{
		assert(stream_);
		assert(data != NULL);

		if (this->Tell() + count > this->Length())
		{
			count = this->Length() - this->Tell();
		}

		stream_->read(static_cast<char*>(data),
			static_cast<std::streamsize>(count));

		return count;
	}

	// 把文件指针移到指定位置
	/////////////////////////////////////////////////////////////////////////////////
	void DiskFile::OnSeek(size_t offset, std::ios_base::seekdir from)
	{
		assert(stream_);

		if (std::ios_base::in == openMode_)
		{
			stream_->seekg(static_cast<std::iostream::off_type>(offset), from);
		}
		else
		{
			stream_->seekp(static_cast<std::iostream::off_type>(offset), from);
		}
	}


	DiskFileResIdentifier::DiskFileResIdentifier(std::string const & fileName)
		: fileName_(fileName)
	{
	}

	VFilePtr DiskFileResIdentifier::Load()
	{
		return VFilePtr(new DiskFile(fileName_, VFile::OM_Read));
	};
}
