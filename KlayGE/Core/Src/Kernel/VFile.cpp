// VFile.cpp
// KlayGE 虚拟文件类 实现文件
// Ver 2.2.0
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.2.0
// 大量代码从子类抽象过来 (2004.10.21)
//
// 2.0.0
// 初次建立 (2003.8.3)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>

#include <istream>
#include <vector>

#include <KlayGE/VFile.hpp>

namespace KlayGE
{
	// 空文件对象类
	/////////////////////////////////////////////////////////////////////////////////
	class NullVFile : public VFile
	{
	public:
		NullVFile()
			: VFile(OM_Unknown)
			{ }

		size_t Write(void const * data, size_t count)
			{ return 0; }
		size_t Read(void* data, size_t count)
			{ return 0; }

		void OnSeek(size_t offset, std::ios_base::seekdir from)
			{ }
	};

	// 返回空对象
	/////////////////////////////////////////////////////////////////////////////////
	VFilePtr VFile::NullObject()
	{
		static VFilePtr obj(new NullVFile);
		return obj;
	}

	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	VFile::VFile(OpenMode openMode)
			: openMode_(openMode)
	{
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	VFile::~VFile()
	{
		this->Close();
	}

	// 测试是否失败
	/////////////////////////////////////////////////////////////////////////////////
	bool VFile::Fail()
	{
		return stream_->fail();
	}

	// 关闭文件
	/////////////////////////////////////////////////////////////////////////////////
	void VFile::Close()
	{
		stream_.reset();
	}

	// 获取文件长度
	/////////////////////////////////////////////////////////////////////////////////
	size_t VFile::Length()
	{
		size_t curPos(this->Tell());
		size_t len(this->Seek(0, SM_End));
		this->Seek(curPos, SM_Begin);

		return len;
	}

	// 从文件拷贝数据到当前文件
	/////////////////////////////////////////////////////////////////////////////////
	size_t VFile::CopyFrom(VFile& src, size_t size)
	{
		std::vector<U8> data(size);
		size = src.Read(&data[0], data.size());
		return this->Write(&data[0], size);
	}

	// 把文件指针移到指定位置
	/////////////////////////////////////////////////////////////////////////////////
	size_t VFile::Seek(size_t offset, SeekMode from)
	{
		assert(stream_);

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

		this->OnSeek(static_cast<std::iostream::off_type>(offset), seekFrom);

		return this->Tell();
	}

	// 获取文件指针位置
	/////////////////////////////////////////////////////////////////////////////////
	size_t VFile::Tell()
	{
		assert(stream_);
		assert(!this->Fail());

		if (OM_Read == openMode_)
		{
			return stream_->tellg();
		}
		else
		{
			return stream_->tellp();
		}
	}

	// 文件指针回零
	/////////////////////////////////////////////////////////////////////////////////
	void VFile::Rewind()
	{
		this->Seek(0, SM_Begin);
	}
}
