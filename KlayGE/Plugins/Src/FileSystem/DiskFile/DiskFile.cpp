// DiskFile.cpp
// KlayGE 磁盘文件类 实现文件
// Ver 2.0.4
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
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
#include <KlayGE/CommFuncs.hpp>

#include <vector>
#include <cassert>

#include <KlayGE/DiskFile/DiskFile.hpp>

using namespace std;

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	DiskFile::DiskFile()
			: openMode_(OM_Unknown)
	{
	}

	DiskFile::DiskFile(const WString& fileName, OpenMode openMode)
			: openMode_(OM_Unknown)
	{
		this->Open(fileName, openMode);
	}

	DiskFile::DiskFile(const DiskFile& rhs)
			: openMode_(OM_Unknown)
	{
		this->Open(rhs.fileName_, rhs.openMode_);
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	DiskFile::~DiskFile()
	{
		this->Close();
	}

	// 打开文件
	/////////////////////////////////////////////////////////////////////////////////
	void DiskFile::Open(const WString& fileName, OpenMode openMode)
	{
		this->Close();

		ios_base::openmode mode(ios_base::binary);

		switch (openMode)
		{
		case OM_Read:
			mode |= ios_base::in;
			break;

		case OM_Write:
			mode |= ios_base::out;
			break;

		case OM_ReadWrite:
			mode |= ios_base::in | ios_base::out;
			break;
			
		case OM_Create:
			mode |= ios_base::out | ios_base::trunc;
			break;
		}

		String fn;
		Convert(fn, fileName);
		file_ = SharePtr<fstream>(new fstream(fn.c_str(), mode));

		fileName_ = fileName;
		openMode_ = openMode;
	}

	// 关闭文件
	/////////////////////////////////////////////////////////////////////////////////
	void DiskFile::Close()
	{
		if ((file_.Get() != NULL) && file_->is_open())
		{
			file_->close();
			file_ = SharePtr<fstream>();
		}
	}

	// 克隆一个与对象相同的指针
	/////////////////////////////////////////////////////////////////////////////////
	VFilePtr DiskFile::Clone() const
	{
		return VFilePtr(new DiskFile(*this));
	}

	// 重载=
	/////////////////////////////////////////////////////////////////////////////////
	DiskFile& DiskFile::operator=(const DiskFile& rhs)
	{
		DiskFile(rhs).Swap(*this);
		return *this;
	}

	// 获取文件长度
	/////////////////////////////////////////////////////////////////////////////////
	size_t DiskFile::Length()
	{
		assert((file_.Get() != NULL) && file_->is_open());

		size_t curPos(this->Tell());
		size_t len(this->Seek(0, SM_End));
		this->Seek(curPos, SM_Begin);

		return len;
	}

	// 设置文件长度
	/////////////////////////////////////////////////////////////////////////////////
	void DiskFile::Length(size_t newLen)
	{
		assert((file_.Get() != NULL) && file_->is_open());

		this->Seek(newLen, SM_Begin);

		short eof(EOF);
		this->Write(&eof, sizeof(eof));
	}

	// 把数据写入文件
	/////////////////////////////////////////////////////////////////////////////////
	size_t DiskFile::Write(const void* data, size_t count)
	{
		assert((file_.Get() != NULL) && file_->is_open());
		assert(data != NULL);

		file_->write(static_cast<const char*>(data), count);

		return count;
	}

	// 从文件读出数据
	/////////////////////////////////////////////////////////////////////////////////
	size_t DiskFile::Read(void* data, size_t count)
	{
		assert((file_.Get() != NULL) && file_->is_open());
		assert(data != NULL);

		if (this->Tell() + count > this->Length())
		{
			count = this->Length() - this->Tell();
		}

		file_->read(static_cast<char*>(data), count);

		return count;
	}

	// 从文件拷贝数据到当前文件
	/////////////////////////////////////////////////////////////////////////////////
	size_t DiskFile::CopyFrom(VFile& src, size_t size)
	{
		std::vector<U8> data(size);
		return this->Write(&data[0], src.Read(&data[0], data.size()));
	}

	// 把文件指针移到指定位置
	/////////////////////////////////////////////////////////////////////////////////
	size_t DiskFile::Seek(size_t offset, SeekMode from)
	{
		assert((file_.Get() != NULL) && file_->is_open());

		ios_base::seekdir seekFrom(ios_base::beg);
		switch (from)
		{
		case SM_Begin:
			seekFrom = ios_base::beg;
			break;

		case SM_End:
			seekFrom = ios_base::end;
			break;

		case SM_Current:
			seekFrom = ios_base::cur;
			break;
		}

		if (openMode_ & ios_base::in)
		{
			file_->seekg(offset, seekFrom);
		}
		else
		{
			file_->seekp(offset, seekFrom);
		}

		return this->Tell();
	}

	// 过去文件指针位置
	/////////////////////////////////////////////////////////////////////////////////
	size_t DiskFile::Tell()
	{
		assert((file_.Get() != NULL) && file_->is_open());

		if (openMode_ & ios_base::in)
		{
			return file_->tellg();
		}
		else
		{
			return file_->tellp();
		}
	}

	// 把数据立刻写入文件
	/////////////////////////////////////////////////////////////////////////////////
	void DiskFile::Flush()
	{
		assert((file_.Get() != NULL) && file_->is_open());

		file_->flush();
	}

	// 交换两个文件对象
	/////////////////////////////////////////////////////////////////////////////////
	void DiskFile::Swap(DiskFile& rhs)
	{
		std::swap(file_, rhs.file_);
		std::swap(fileName_, rhs.fileName_);
		std::swap(openMode_, rhs.openMode_);
	}
}
