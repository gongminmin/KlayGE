// Unpkt.cpp
// KlayGE 打包文件读取类 实现文件
// Ver 2.1.0
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.1.0
// 简化了目录表的表示法 (2004.4.14)
//
// 2.0.0
// 初次建立 (2003.9.18)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/Crc32.hpp>

#include <cassert>
#include <cctype>
#include <string>
#include <algorithm>

#include <KlayGE/MemFile/MemFile.hpp>
#include <KlayGE/PackedFile/Pkt.hpp>

namespace
{
	using namespace KlayGE;

	U32 const N(4096);				// size of ring buffer
	U32 const F(18);				// upper limit for match_length
	U32 const THRESHOLD(2);			// encode string into position and length
									//   if match_length is greater than this
	U32 const NIL(N);				// index for root of binary search trees


	U8 textBuf[N + F - 1];			// ring buffer of size N, 
									// with extra F-1 bytes to facilitate string comparison

	// 忽略大小写比较字符串
	/////////////////////////////////////////////////////////////////////////////////
	bool IgnoreCaseCompare(std::string const & lhs, std::string const & rhs)
	{
		if (lhs.length() != rhs.length())
		{
			return false;
		}

		U32 i(0);
		while ((i < lhs.length()) && (std::toupper(lhs[i]) == std::toupper(rhs[i])))
		{
			++ i;
		}

		if (i != lhs.length())
		{
			return false;
		}

		return true;
	}

	// 读入目录表
	/////////////////////////////////////////////////////////////////////////////////
	void ReadDirTable(DirTable& dirTable, VFile& input)
	{
		using namespace KlayGE;

		while (input.Tell() != input.Length())
		{
			U32 len;
			input.Read(&len, sizeof(len));
			std::vector<char> str(len);
			input.Read(&str[0], str.size());

			FileDes fd;
			input.Read(&fd, sizeof(fd));
			dirTable.insert(std::make_pair(std::string(str.begin(), str.end()), fd));
		}
	}
}

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	UnPkt::UnPkt()
	{
	}

	// 析构函数
	/////////////////////////////////////////////////////////////////////////////////
	UnPkt::~UnPkt()
	{
		this->Close();
	}

	// LZSS解压
	/////////////////////////////////////////////////////////////////////////////////
	void UnPkt::Decode(VFile& output, VFile& input)
	{
		U32 r(N - F);
		std::fill_n(textBuf, r, ' ');

		U32 flags(0);
		U8 c;
		for (;;)
		{
			if (0 == ((flags >>= 1) & 256))
			{
				if (input.Tell() >= input.Length())
				{
					break;
				}

				input.Read(&c, 1);

				flags = c | 0xFF00;		// uses higher byte cleverly
											// to count eight
			}
			if (flags & 1)
			{
				if (input.Tell() >= input.Length())
				{
					break;
				}

				input.Read(&c, 1);

				output.Write(&c, 1);
				textBuf[r] = c;
				++ r;
				r &= (N - 1);
			}
			else
			{
				if (input.Tell() >= input.Length())
				{
					break;
				}
				input.Read(&c, 1);
				U32 c1(c);

				if (input.Tell() >= input.Length())
				{
					break;
				}
				input.Read(&c, 1);
				U32 c2(c);

				c1 |= ((c2 & 0xF0) << 4);
				c2 = (c2 & 0x0F) + THRESHOLD;
				for (U32 k = 0; k <= c2; ++ k)
				{
					c = textBuf[(c1 + k) & (N - 1)];
					output.Write(&c, 1);
					textBuf[r] = c;
					++ r;
					r &= (N - 1);
				}
			}
		}

		output.Rewind();
	}

	// 打开打包文件
	/////////////////////////////////////////////////////////////////////////////////
	void UnPkt::Open(VFilePtr const & pktFile)
	{
		this->Close();

		file_ = pktFile;

		file_->Read(&mag_, sizeof(mag_));
		Verify(MakeFourCC<'p', 'k', 't', ' '>::value == mag_.magic);
		Verify(3 == mag_.ver);

		file_->Seek(mag_.DTStart, VFile::SM_Begin);

		MemFile dtCom;
		dtCom.CopyFrom(*file_, mag_.DTLength);

		MemFile dt;
		dtCom.Rewind();
		Decode(dt, dtCom);

		dt.Rewind();
		ReadDirTable(dirTable_, dt);
	}

	// 关闭打包文件
	/////////////////////////////////////////////////////////////////////////////////
	void UnPkt::Close()
	{
		file_ = VFilePtr();
	}

	// 在打包文件中定位文件
	/////////////////////////////////////////////////////////////////////////////////
	void UnPkt::LocateFile(std::string const & pathName)
	{
		curFile_ = dirTable_.find(pathName);
	}

	// 获取当前文件(解压过的)的字节数
	/////////////////////////////////////////////////////////////////////////////////
	size_t UnPkt::CurFileSize() const
	{
		assert(curFile_ != dirTable_.end());

		return curFile_->second.DeComLength;
	}

	// 读取当前文件(解压过的)
	/////////////////////////////////////////////////////////////////////////////////
	bool UnPkt::ReadCurFile(void* data)
	{
		assert(data != NULL);
		assert(curFile_ != dirTable_.end());

		file_->Seek(mag_.FIStart + curFile_->second.start, VFile::SM_Begin);

		if (curFile_->second.attr & FA_UnCompressed)
		{
			file_->Read(data, this->CurFileSize());
		}
		else
		{
			MemFile chunk;
			chunk.CopyFrom(*file_, this->CurFileCompressedSize());

			MemFile out;
			chunk.Rewind();
			Decode(out, chunk);

			out.Read(data, out.Length());
		}

		if (Crc32::CrcMem(static_cast<U8*>(data), this->CurFileSize())
			!= curFile_->second.crc32)
		{
			return false;	// CRC32 错误
		}

		return true;
	}

	// 获取当前文件(没解压过的)的字节数
	/////////////////////////////////////////////////////////////////////////////////
	size_t UnPkt::CurFileCompressedSize() const
	{
		return curFile_->second.length;
	}

	// 读取当前文件(没解压过的)
	/////////////////////////////////////////////////////////////////////////////////
	void UnPkt::ReadCurFileCompressed(void* data)
	{
		assert(data != NULL);

		file_->Seek(mag_.FIStart + curFile_->second.start, VFile::SM_Begin);
		file_->Read(data, this->CurFileCompressedSize());
	}
}