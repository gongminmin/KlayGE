// Pkt.cpp
// KlayGE 文件打包类
// Ver 2.2.0
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
// LZSS压缩算法的作者是Haruhiko Okumura
//
// 2.2.0
// 统一使用istream作为资源标示符 (2004.10.26)
// 使用boost::crc来计算crc32 (2004.10.28)
//
// 2.1.3
// 修正了CRC错误的bug
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

#include <cassert>
#include <string>
#include <algorithm>
#include <vector>
#include <sstream>
#include <fstream>

#include <KlayGE/LZSS/LZSS.hpp>

#include <boost/crc.hpp>
#include <boost/filesystem/operations.hpp>

namespace
{
	using namespace KlayGE;

	class LZSS
	{
	public:
		void Encode(std::ostream& out, std::istream& in)
		{
			std::istreambuf_iterator<char> inEnd;
			std::istreambuf_iterator<char> inIter(in);
			std::ostreambuf_iterator<char> outIter(out);

			int lastMatchLength;
			uint8_t c;
			std::vector<uint8_t> codeBuf(17, 0);		// codeBuf[1..16] saves eight units of code, and
												// codeBuf[0] works as eight flags, "1" representing that the unit
												// is an unencoded letter (1 byte), "0" a position-and-length pair
												// (2 bytes).  Thus, eight units require at most 16 bytes of code.
			std::vector<uint8_t>::iterator codeBufPtr = codeBuf.begin() + 1;

			uint8_t mask = 1;

			this->InitTree();					// initialize trees
			uint32_t s(0);
			uint32_t r(N - F);
			std::fill_n(textBuf_, r, ' ');		// Clear the buffer with
												// any character that will appear often.

			for (int i = 1; i <= F; ++ i)
			{
				this->InsertNode(r - i);	// Insert the F strings,
											// each of which begins with one or more 'space' characters.  Note
											// the order in which these strings are inserted.  This way,
											// degenerate trees will be less likely to occur.
			}
			this->InsertNode(r);			// Finally, insert the whole string just read.  The
											// global variables matchLength and matchPosition are set.

			int len(0);
			while ((inIter != inEnd) && (len < F))
			{
				c = *inIter;
				++ inIter;

				textBuf_[r + len] = c;			// Read F bytes into the last F bytes of the buffer

				++ len;
			}

			do
			{
				if (matchLength_ > len)
				{
					matchLength_ = len;		// matchLength
											// may be spuriously long near the end of text.
				}

				if (matchLength_ <= THRESHOLD)
				{
					matchLength_ = 1;				// Not long enough match.  Send one byte.
					codeBuf[0] |= mask;				// 'send one byte' flag
					*codeBufPtr = textBuf_[r];		// Send uncoded.
					++ codeBufPtr;
				}
				else
				{
					*codeBufPtr = static_cast<uint8_t>(matchPosition_);
					++ codeBufPtr;
					*codeBufPtr = static_cast<uint8_t>(((matchPosition_ >> 4) & 0xF0)
						| (matchLength_ - (THRESHOLD + 1)));		// Send position and
																	// length pair. Note matchLength > THRESHOLD.
					++ codeBufPtr;
				}

				mask <<= 1;
				if (0 == mask)
				{
					// Shift mask left one bit.
					std::copy(codeBuf.begin(), codeBufPtr, outIter);	// Send at most 8 units of
																		// code together
					codeBuf[0] = 0;
					codeBufPtr = codeBuf.begin() + 1;
					mask = 1;
				}
				lastMatchLength = matchLength_;

				int i(0);
				while ((inIter != inEnd) && (i < lastMatchLength))
				{
					c = *inIter;
					++ inIter;

					this->DeleteNode(s);		// Delete old strings and
					textBuf_[s] = c;			// read new bytes
					if (s < F - 1)
					{
						textBuf_[s + N] = c;	// If the position is
												// near the end of buffer, extend the buffer to make
												// string comparison easier.
					}
					s = (s + 1) & (N - 1);
					r = (r + 1) & (N - 1);

					// Since this is a ring buffer, increment the position
					// modulo N.
					this->InsertNode(r);	// Register the string in textBuf[r..r+F-1]

					++ i;
				}

				while (i < lastMatchLength)
				{
					this->DeleteNode(s);
					s = (s + 1) & (N - 1);
					r = (r + 1) & (N - 1);
					if (-- len)
					{
						this->InsertNode(r);		// buffer may not be empty.
					}

					++ i;
				}
			} while (len > 0);	// until length of string to be processed is zero

			if (codeBufPtr - codeBuf.begin() > 1)
			{
				// Send remaining code
				std::copy(codeBuf.begin(), codeBufPtr, outIter);
			}
		}

	private:
		// initialize trees
		void InitTree()
		{
			// For i = 0 to N - 1, rson[i] and lson[i] will be the right and
			// left children of node i.  These nodes need not be initialized.
			// Also, dad[i] is the parent of node i.  These are initialized to
			// NIL (= N), which stands for 'not used.'
			// For i = 0 to 255, rson[N + i + 1] is the root of the tree
			// for strings that begin with character i.  These are initialized
			// to NIL.  Note there are 256 trees.

			std::fill_n(rson_ + N + 1, 256, NIL);
			std::fill_n(dad_, N, NIL);
		}

		void InsertNode(uint32_t r)
		{
			// Inserts string of length F, textBuf[r..r+F-1], into one of the
			// trees (textBuf[r]'th tree) and returns the longest-match position
			// and length via the global variables matchPosition and matchLength.
			// If matchLength = F, then removes the old node in favor of the new
			// one, because the old one will be deleted sooner.
			// Note r plays double role, as tree node and position in buffer.

			uint32_t cmp(1);
			uint8_t* key(&textBuf_[r]);
			uint32_t p(N + 1 + key[0]);
			rson_[r] = lson_[r] = NIL;
			matchLength_ = 0;

			for (;;)
			{
				if (cmp >= 0)
				{
					if (rson_[p] != NIL)
					{
						p = rson_[p];
					}
					else
					{
						rson_[p] = r;
						dad_[r] = p;
						return;
					}
				}
				else
				{
					if (lson_[p] != NIL)
					{
						p = lson_[p];
					}
					else
					{
						lson_[p] = r;
						dad_[r] = p;
						return;
					}
				}

				int i(1);
				while ((i < F) && (0 == (cmp = key[i] - textBuf_[p + i])))
				{
					++ i;
				}
				if (i > matchLength_)
				{
					matchPosition_ = p;
					if ((matchLength_ = i) >= F)
					{
						break;
					}
				}
			}

			dad_[r]  = dad_[p];
			lson_[r] = lson_[p];
			rson_[r] = rson_[p];
			dad_[lson_[p]] = r;
			dad_[rson_[p]] = r;
			if (rson_[dad_[p]] == p)
			{
				rson_[dad_[p]] = r;
			}
			else
			{
				lson_[dad_[p]] = r;
			}
			dad_[p] = NIL;  // remove p
		}

		// deletes node p from tree
		void DeleteNode(uint32_t p)
		{
			uint32_t q;

			if (NIL == dad_[p])
			{
				return;  // not in tree
			}
			if (NIL == rson_[p])
			{
				q = lson_[p];
			}
			else
			{
				if (NIL == lson_[p])
				{
					q = rson_[p];
				}
				else
				{
					q = lson_[p];
					if (rson_[q] != NIL)
					{
						do
						{
							q = rson_[q];
						} while (rson_[q] != NIL);

						rson_[dad_[q]] = lson_[q];
						dad_[lson_[q]] = dad_[q];
						lson_[q] = lson_[p];
						dad_[lson_[p]] = q;
					}
					
					rson_[q] = rson_[p];
					dad_[rson_[p]] = q;
				}
			}

			dad_[q] = dad_[p];
			if (rson_[dad_[p]] == p)
			{
				rson_[dad_[p]] = q;
			}
			else
			{
				lson_[dad_[p]] = q;
			}
			dad_[p] = NIL;
		}

	private:
		static int const N = 4096;			// size of ring buffer
		static int const F = 18;			// upper limit for matchLength
		static int const THRESHOLD = 2;		// encode string into position and length
		static int const NIL = N;			// index for root of binary search trees

		unsigned char textBuf_[N + F - 1];		// ring buffer of size N, 
												// with extra F-1 bytes to facilitate string comparison
		int matchPosition_, matchLength_;		// of longest match.  These are
												// set by the InsertNode() procedure.
		int lson_[N + 1], rson_[N + 257];		// left & right children &
		int dad_[N + 1];						// parents -- These constitute binary search trees.
	};

	// 输出目录表
	/////////////////////////////////////////////////////////////////////////////////
	void WriteDirTable(std::ostream& out, KlayGE::DirTable const & dirTable)
	{
		for (DirTable::const_iterator iter = dirTable.begin(); iter != dirTable.end(); ++ iter)
		{
			FileDes const & fd(iter->second);

			std::string const & fileName(iter->first);
			uint32_t const temp(static_cast<uint32_t>(fileName.length()));
			out.write(reinterpret_cast<const char*>(&temp), sizeof(temp));
			out.write(&fileName[0], temp);

			out.write(reinterpret_cast<const char*>(&fd), sizeof(fd));
		}
	}

	// 根据树型结构打包目录
	/////////////////////////////////////////////////////////////////////////////////
	void Compress(std::ostream& outFile, DirTable& dirTable, std::string const & rootName,
		std::vector<std::string> const & files)
	{
		uint32_t curPos(0);

		for (std::vector<std::string>::const_iterator iter = files.begin(); iter != files.end(); ++ iter)
		{
			boost::crc_32_type crc32;

			std::ifstream in((rootName + '/' + *iter).c_str(), std::ios_base::binary);
			assert(in);

			do
			{
				char buf[1024];
				in.read(buf, sizeof(buf));
				crc32.process_bytes(buf, in.gcount());
			} while (in);
			in.clear();

			FileDes fd;
			fd.crc32 = crc32.checksum();
			fd.DeComLength = static_cast<uint32_t>(in.tellg());
			in.seekg(0);

			std::stringstream out;
			Pkt::Encode(out, in);
			
			std::istream* p = &out;
			if (out.tellp() >= in.tellg())
			{
				fd.attr = FA_UnCompressed;
				p = &in;
			}

			p->seekg(0, std::ios_base::end);
			fd.length	= static_cast<uint32_t>(p->tellg());
			fd.start	= curPos;
			curPos += fd.length;

			dirTable.insert(std::make_pair(*iter, fd));

			p->seekg(0);
			std::vector<char> temp(fd.length);
			p->read(&temp[0], temp.size());
			outFile.write(&temp[0], temp.size());
		}
	}

	// 遍历目录，得出树型结构
	/////////////////////////////////////////////////////////////////////////////////
	std::vector<std::string> FindFiles(std::string const & rootName, std::string const & pathName)
	{
		using namespace boost::filesystem;

		std::vector<std::string> ret;

		path findPath(rootName + '/' + pathName, native);
		if (exists(findPath))
		{
			for (directory_iterator iter(findPath); iter != directory_iterator(); ++ iter)
			{
				std::string const & fileName(iter->leaf());

				if (is_directory(*iter))
				{
					std::vector<std::string> sub(FindFiles(rootName, pathName + fileName + '/'));
					ret.insert(ret.end(), sub.begin(), sub.end());
				}
				else
				{
					ret.push_back(pathName + fileName);
				}
			}
		}

		return ret;
	}
}

namespace KlayGE
{
	// 翻译路径名，把'\'转化成'/'
	/////////////////////////////////////////////////////////////////////////////////
	std::string& TransPathName(std::string& out, std::string const & in)
	{
		out = in;
		for (std::string::iterator iter = out.begin(); iter != out.end(); ++ iter)
		{
			if ('\\' == *iter)
			{
				*iter = '/';
			}
		}

		return out;
	}

	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	Pkt::Pkt()
	{
	}

	// 用LZSS编码
	/////////////////////////////////////////////////////////////////////////////////
	void Pkt::Encode(std::ostream& out, std::istream& in)
	{
		LZSS lzss;
		lzss.Encode(out, in);
	}

	// 目录打包
	/////////////////////////////////////////////////////////////////////////////////
	void Pkt::Pack(std::string const & dirName, std::ostream& pktFile)
	{
		std::string rootName;
		TransPathName(rootName, dirName);

		std::vector<std::string> files(FindFiles(rootName, ""));

		DirTable dirTable;
		std::stringstream tmpFile;
		Compress(tmpFile, dirTable, rootName, files);

		std::stringstream dt;
		WriteDirTable(dt, dirTable);

		std::stringstream dtCom;
		dt.seekg(0);
		Encode(dtCom, dt);

		PktHeader mag;
		mag.magic			= MakeFourCC<'p', 'k', 't', ' '>::value;
		mag.ver				= 3;
		mag.DTStart			= sizeof(mag);
		mag.DTLength		= static_cast<uint32_t>(dtCom.tellp());
		mag.DTDeComLength	= static_cast<uint32_t>(dt.tellg());
		mag.FIStart			= mag.DTStart + mag.DTLength;

		pktFile.seekp(0);
		pktFile.write(reinterpret_cast<char*>(&mag), sizeof(mag));

		std::copy(std::istreambuf_iterator<char>(dtCom), std::istreambuf_iterator<char>(),
			std::ostreambuf_iterator<char>(pktFile));
		std::copy(std::istreambuf_iterator<char>(tmpFile), std::istreambuf_iterator<char>(),
			std::ostreambuf_iterator<char>(pktFile));
	}
}
