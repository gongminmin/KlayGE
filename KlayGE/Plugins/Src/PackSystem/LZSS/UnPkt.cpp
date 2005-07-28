// Unpkt.cpp
// KlayGE 打包文件读取类 实现文件
// Ver 2.2.0
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
//
// 2.2.0
// 统一使用istream作为资源标示符 (2004.10.26)
// 使用boost::crc来计算crc32 (2004.10.28)
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

#include <string>
#include <vector>
#include <sstream>

#include <boost/assert.hpp>
#pragma warning(disable: 4127 4800)
#include <boost/pool/pool_alloc.hpp>
#pragma warning(disable: 4244 4245)
#include <boost/crc.hpp>

#include <KlayGE/LZSS/LZSS.hpp>

namespace
{
	using namespace KlayGE;

	class LZSS
	{
	public:
		void Decode(std::ostream& out, std::istream& in)
		{
			std::ostreambuf_iterator<char> outIter(out);

			uint32_t r(N - F);
			std::fill_n(textBuf_, r, ' ');

			uint32_t flags(0);
			uint8_t c;
			for (;;)
			{
				if (0 == ((flags >>= 1) & 256))
				{
					in.read(reinterpret_cast<char*>(&c), sizeof(c));
					if (in.fail())
					{
						break;
					}

					flags = c | 0xFF00;		// uses higher byte cleverly
											// to count eight
				}
				if (flags & 1)
				{
					in.read(reinterpret_cast<char*>(&c), sizeof(c));
					if (in.fail())
					{
						break;
					}

					*outIter = c;
					++ outIter;

					textBuf_[r] = c;
					++ r;
					r &= (N - 1);
				}
				else
				{
					in.read(reinterpret_cast<char*>(&c), sizeof(c));
					if (in.fail())
					{
						break;
					}
					uint32_t c1(c);

					in.read(reinterpret_cast<char*>(&c), sizeof(c));
					if (in.fail())
					{
						break;
					}
					uint32_t c2(c);

					c1 |= ((c2 & 0xF0) << 4);
					c2 = (c2 & 0x0F) + THRESHOLD;
					for (uint32_t k = 0; k <= c2; ++ k)
					{
						c = textBuf_[(c1 + k) & (N - 1)];
						
						*outIter = c;
						++ outIter;

						textBuf_[r] = c;
						++ r;
						r &= (N - 1);
					}
				}
			}

			in.clear();
		}

	private:
		static int const N = 4096;			// size of ring buffer
		static int const F = 18;			// upper limit for match_length
		static int const THRESHOLD = 2;		// encode string into position and length
											// if match_length is greater than this
		static int const NIL = N;			// index for root of binary search trees


		uint8_t textBuf_[N + F - 1];				// ring buffer of size N, 
											// with extra F-1 bytes to facilitate string comparison
	};

	// 读入目录表
	/////////////////////////////////////////////////////////////////////////////////
	void ReadDirTable(DirTable& dirTable, std::istream& input)
	{
		for (;;)
		{
			uint32_t len;
			input.read(reinterpret_cast<char*>(&len), sizeof(len));
			if (input.fail())
			{
				break;
			}

			std::vector<char, boost::pool_allocator<char> > str(len);
			input.read(&str[0], static_cast<std::streamsize>(str.size()));
			if (input.fail())
			{
				break;
			}

			FileDes fd;
			input.read(reinterpret_cast<char*>(&fd), sizeof(fd));
			if (input.fail())
			{
				break;
			}

			dirTable.insert(std::make_pair(std::string(str.begin(), str.end()), fd));
		}

		input.clear();
	}
}

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	UnPkt::UnPkt()
	{
	}

	// LZSS解压
	/////////////////////////////////////////////////////////////////////////////////
	void UnPkt::Decode(std::ostream& out, std::istream& in)
	{
		LZSS lzss;
		lzss.Decode(out, in);
	}

	// 打开打包文件
	/////////////////////////////////////////////////////////////////////////////////
	void UnPkt::Open(boost::shared_ptr<std::istream> const & pktFile)
	{
		file_ = pktFile;

		file_->read(reinterpret_cast<char*>(&mag_), sizeof(mag_));
		Verify(MakeFourCC<'p', 'k', 't', ' '>::value == mag_.magic);
		Verify(3 == mag_.ver);

		file_->seekg(mag_.DTStart);

		std::stringstream dtCom;
		{
			std::vector<char, boost::pool_allocator<char> > temp(mag_.DTLength);
			file_->read(&temp[0], static_cast<std::streamsize>(temp.size()));
			dtCom.write(&temp[0], static_cast<std::streamsize>(temp.size()));
		}

		std::stringstream dt;
		dtCom.seekp(0);
		Decode(dt, dtCom);

		dt.seekp(0);
		ReadDirTable(dirTable_, dt);
	}

	// 在打包文件中定位文件
	/////////////////////////////////////////////////////////////////////////////////
	void UnPkt::LocateFile(std::string const & pathName)
	{
		DirTable::iterator iter = dirTable_.find(pathName);
		BOOST_ASSERT(iter != dirTable_.end());

		curFile_ = iter;
	}

	// 获取当前文件(解压过的)的字节数
	/////////////////////////////////////////////////////////////////////////////////
	size_t UnPkt::CurFileSize() const
	{
		BOOST_ASSERT(curFile_ != dirTable_.end());

		return curFile_->second.DeComLength;
	}

	// 读取当前文件(解压过的)
	/////////////////////////////////////////////////////////////////////////////////
	bool UnPkt::ReadCurFile(void* data)
	{
		BOOST_ASSERT(data != NULL);
		BOOST_ASSERT(curFile_ != dirTable_.end());

		file_->seekg(mag_.FIStart + curFile_->second.start);

		if (curFile_->second.attr & FA_UnCompressed)
		{
			file_->read(static_cast<char*>(data), static_cast<std::streamsize>(this->CurFileSize()));
		}
		else
		{
			std::stringstream chunk;
			{
				std::vector<char, boost::pool_allocator<char> > temp(this->CurFileCompressedSize());
				file_->read(&temp[0], static_cast<std::streamsize>(temp.size()));
				chunk.write(&temp[0], static_cast<std::streamsize>(temp.size()));
			}

			std::stringstream out;
			chunk.seekp(0);
			Decode(out, chunk);

			std::copy(std::istreambuf_iterator<char>(out), std::istreambuf_iterator<char>(),
				static_cast<char*>(data));
		}

		boost::crc_32_type crc32;
		crc32.process_bytes(data, this->CurFileSize());

		if (crc32.checksum() != curFile_->second.crc32)
		{
			return false;	// CRC32 错误
		}

		return true;
	}

	// 获取当前文件(没解压过的)的字节数
	/////////////////////////////////////////////////////////////////////////////////
	size_t UnPkt::CurFileCompressedSize() const
	{
		BOOST_ASSERT(curFile_ != dirTable_.end());

		return curFile_->second.length;
	}

	// 读取当前文件(没解压过的)
	/////////////////////////////////////////////////////////////////////////////////
	void UnPkt::ReadCurFileCompressed(void* data)
	{
		BOOST_ASSERT(data != NULL);
		BOOST_ASSERT(curFile_ != dirTable_.end());

		file_->seekg(mag_.FIStart + curFile_->second.start);
		file_->read(static_cast<char*>(data), static_cast<std::streamsize>(this->CurFileCompressedSize()));
	}
}