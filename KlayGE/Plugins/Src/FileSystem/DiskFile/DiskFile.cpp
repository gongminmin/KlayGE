// DiskFile.cpp
// KlayGE 磁盘文件类 实现文件
// Ver 2.1.2
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
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
	DiskFile::DiskFile()
			: openMode_(OM_Unknown)
	{
	}

	DiskFile::DiskFile(std::string const & fileName, OpenMode openMode)
			: openMode_(OM_Unknown)
	{
		this->Open(fileName, openMode);
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	DiskFile::~DiskFile()
	{
		this->Close();
	}

	// 打开文件
	/////////////////////////////////////////////////////////////////////////////////
	bool DiskFile::Open(std::string const & fileName, OpenMode openMode)
	{
		this->Close();

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
		openMode_ = openMode;

		file_.open(fileName.c_str(), mode);

		return !file_.fail();
	}

	// 关闭文件
	/////////////////////////////////////////////////////////////////////////////////
	void DiskFile::Close()
	{
		if (file_.is_open())
		{
			file_.close();
		}
		file_.clear();
	}

	// 获取文件长度
	/////////////////////////////////////////////////////////////////////////////////
	size_t DiskFile::Length()
	{
		assert(file_.is_open());

		size_t curPos(this->Tell());
		size_t len(this->Seek(0, SM_End));
		this->Seek(curPos, SM_Begin);

		return len;
	}

	// 设置文件长度
	/////////////////////////////////////////////////////////////////////////////////
	void DiskFile::Length(size_t newLen)
	{
		assert(file_.is_open());

		this->Seek(newLen, SM_Begin);

		short eof(EOF);
		this->Write(&eof, sizeof(eof));
	}

	// 把数据写入文件
	/////////////////////////////////////////////////////////////////////////////////
	size_t DiskFile::Write(void const * data, size_t count)
	{
		assert(file_.is_open());
		assert(data != NULL);

		file_.write(static_cast<char const *>(data),
			static_cast<std::streamsize>(count));

		return count;
	}

	// 从文件读出数据
	/////////////////////////////////////////////////////////////////////////////////
	size_t DiskFile::Read(void* data, size_t count)
	{
		assert(file_.is_open());
		assert(data != NULL);

		if (this->Tell() + count > this->Length())
		{
			count = this->Length() - this->Tell();
		}

		file_.read(static_cast<char*>(data),
			static_cast<std::streamsize>(count));

		return count;
	}

	// 从文件拷贝数据到当前文件
	/////////////////////////////////////////////////////////////////////////////////
	size_t DiskFile::CopyFrom(VFile& src, size_t size)
	{
		std::vector<U8> data(size);
		size = src.Read(&data[0], data.size());
		return this->Write(&data[0], size);
	}

	// 把文件指针移到指定位置
	/////////////////////////////////////////////////////////////////////////////////
	size_t DiskFile::Seek(size_t offset, SeekMode from)
	{
		assert(file_.is_open());

		std::ios_base::seekdir seekFrom(std::ios_base::beg);
		switch (from)
		{
		case SM_Begin:
			seekFrom = std::ios_base::beg;
			break;

		case SM_End:
			seekFrom = std::ios_base::end;
			break;

		case SM_Current:
			seekFrom = std::ios_base::cur;
			break;
		}

		if (OM_Read == openMode_)
		{
			file_.seekg(static_cast<std::istream::off_type>(offset), seekFrom);
		}
		else
		{
			file_.seekp(static_cast<std::ostream::off_type>(offset), seekFrom);
		}

		return this->Tell();
	}

	// 过去文件指针位置
	/////////////////////////////////////////////////////////////////////////////////
	size_t DiskFile::Tell()
	{
		assert(file_.is_open());
		assert(!file_.fail());

		if (OM_Read == openMode_)
		{
			return file_.tellg();
		}
		else
		{
			return file_.tellp();
		}
	}

	// 把数据立刻写入文件
	/////////////////////////////////////////////////////////////////////////////////
	void DiskFile::Flush()
	{
		assert(file_.is_open());

		file_.flush();
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
