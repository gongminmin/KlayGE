// MemFile.cpp
// KlayGE 内存文件类 实现文件
// Ver 2.0.0
// 版权所有(C) 龚敏敏, 2003
// Homepage: http://www.enginedev.com
//
// 2.0.0
// 初次建立 (2003.8.3)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Memory.hpp>
#include <KlayGE/Engine.hpp>

#include <cassert>

#include <KlayGE/MemFile/MemFile.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	MemFile::MemFile()
				: curPos_(0)
	{
	}

	MemFile::MemFile(const void* data, size_t length)
				: curPos_(0)
	{
		this->Open(data, length);
	}

	MemFile::MemFile(const MemFile& rhs)
				: chunkData_(rhs.chunkData_),
					curPos_(rhs.curPos_)
	{
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	MemFile::~MemFile()
	{
		this->Close();
	}

	// 打开文件
	/////////////////////////////////////////////////////////////////////////////////
	void MemFile::Open(const void* data, size_t length)
	{
		this->Close();

		chunkData_.resize(length);
		if (data != NULL)
		{
			Engine::MemoryInstance().Cpy(&chunkData_[0], data, length);
		}

		this->Length(length);
		curPos_ = 0;
	}

	// 关闭文件
	/////////////////////////////////////////////////////////////////////////////////
	void MemFile::Close()
	{
		curPos_ = 0;

		chunkData_.clear();
		std::vector<U8>().swap(chunkData_);
	}

	// 克隆一个与对象相同的指针
	/////////////////////////////////////////////////////////////////////////////////
	VFilePtr MemFile::Clone() const
	{
		return VFilePtr(new MemFile(*this));
	}

	// 重载=
	/////////////////////////////////////////////////////////////////////////////////
	MemFile& MemFile::operator=(const MemFile& rhs)
	{
		MemFile(rhs).Swap(*this);
		return *this;
	}

	// 获取文件长度
	/////////////////////////////////////////////////////////////////////////////////
	size_t MemFile::Length()
	{
		return chunkData_.size();
	}

	// 设置文件长度
	/////////////////////////////////////////////////////////////////////////////////
	void MemFile::Length(size_t newLen)
	{
		chunkData_.resize(newLen);
	}

	// 把数据写入文件
	/////////////////////////////////////////////////////////////////////////////////
	size_t MemFile::Write(const void* data, size_t count)
	{
		assert(data != NULL);

		if (count > (this->Length() - curPos_))
		{
			this->Length(count + curPos_);
		}

		Engine::MemoryInstance().Cpy(&chunkData_[curPos_], data, count);
		curPos_ += count;

		return count;
	}

	// 从文件读出数据
	/////////////////////////////////////////////////////////////////////////////////
	size_t MemFile::Read(void* data, size_t count)
	{
		assert(data != NULL);

		if (count > (this->Length() - curPos_))
		{
			count = this->Length() - curPos_;
		}

		Engine::MemoryInstance().Cpy(data, &chunkData_[curPos_], count);
		curPos_ += count;

		return count;
	}

	// 从文件拷贝数据到当前文件
	/////////////////////////////////////////////////////////////////////////////////
	size_t MemFile::CopyFrom(VFile& src, size_t size)
	{
		std::vector<U8> data(size);
		return this->Write(&data[0], src.Read(&data[0], data.size()));
	}

	// 把文件指针移到指定位置
	/////////////////////////////////////////////////////////////////////////////////
	size_t MemFile::Seek(size_t offset, SeekMode from)
	{
		switch (from)
		{
		case SM_Begin:
			curPos_ = offset;
			break;

		case SM_End:
			curPos_ = this->Length() - 1 - offset;
			break;

		case SM_Current:
			curPos_ += offset;
			break;
		}

		return curPos_;
	}

	// 过去文件指针位置
	/////////////////////////////////////////////////////////////////////////////////
	size_t MemFile::Tell()
	{
		return curPos_;
	}

	// 交换两个内存文件对象
	/////////////////////////////////////////////////////////////////////////////////
	void MemFile::Swap(MemFile& rhs)
	{
		std::swap(chunkData_, rhs.chunkData_);
		std::swap(curPos_, rhs.curPos_);
	}
}
