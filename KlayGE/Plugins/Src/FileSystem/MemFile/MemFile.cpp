// MemFile.cpp
// KlayGE 内存文件类 实现文件
// Ver 2.0.5
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.0.5
// 改为stringstream实现 (2004.4.9)
//
// 2.0.0
// 初次建立 (2003.8.3)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Memory.hpp>

#include <cassert>
#include <string>
#include <vector>

#include <KlayGE/MemFile/MemFile.hpp>

using namespace std;

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	MemFile::MemFile()
	{
	}

	MemFile::MemFile(const void* data, size_t length)
	{
		this->Open(data, length);
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

		chunkData_.str(string(static_cast<const char*>(data)));
	}

	// 关闭文件
	/////////////////////////////////////////////////////////////////////////////////
	void MemFile::Close()
	{
		chunkData_.clear();
	}

	// 获取文件长度
	/////////////////////////////////////////////////////////////////////////////////
	size_t MemFile::Length()
	{
		size_t curPos(this->Tell());
		size_t len(this->Seek(0, SM_End));
		this->Seek(curPos, SM_Begin);

		return len;
	}

	// 设置文件长度
	/////////////////////////////////////////////////////////////////////////////////
	void MemFile::Length(size_t newLen)
	{
		this->Seek(newLen, SM_Begin);

		short eof(EOF);
		this->Write(&eof, sizeof(eof));
	}

	// 把数据写入文件
	/////////////////////////////////////////////////////////////////////////////////
	size_t MemFile::Write(const void* data, size_t count)
	{
		assert(data != NULL);

		chunkData_.write(static_cast<const char*>(data),
			static_cast<std::streamsize>(count));

		chunkData_.seekg(static_cast<istream::off_type>(count), ios_base::cur);

		return count;
	}

	// 从文件读出数据
	/////////////////////////////////////////////////////////////////////////////////
	size_t MemFile::Read(void* data, size_t count)
	{
		assert(data != NULL);

		if (this->Tell() + count > this->Length())
		{
			count = this->Length() - this->Tell();
		}

		chunkData_.read(static_cast<char*>(data),
			static_cast<std::streamsize>(count));

		chunkData_.seekp(static_cast<istream::off_type>(count), ios_base::cur);

		return count;
	}

	// 从文件拷贝数据到当前文件
	/////////////////////////////////////////////////////////////////////////////////
	size_t MemFile::CopyFrom(VFile& src, size_t size)
	{
		vector<U8, alloc<U8> > data(size);
		size = src.Read(&data[0], data.size());
		return this->Write(&data[0], size);
	}

	// 把文件指针移到指定位置
	/////////////////////////////////////////////////////////////////////////////////
	size_t MemFile::Seek(size_t offset, SeekMode from)
	{
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

		chunkData_.seekg(static_cast<istream::off_type>(offset), seekFrom);
		chunkData_.seekp(static_cast<ostream::off_type>(offset), seekFrom);

		return this->Tell();
	}

	// 过去文件指针位置
	/////////////////////////////////////////////////////////////////////////////////
	size_t MemFile::Tell()
	{
		return chunkData_.tellg();
	}
}
