// Pkt.cpp
// KlayGE 文件打包类
// Ver 2.1.3
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
// LZSS压缩算法的作者是Haruhiko Okumura
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
#include <KlayGE/Crc32.hpp>
#include <KlayGE/Util.hpp>
#include <KlayGE/VFile.hpp>
#include <KlayGE/Memory.hpp>

#include <cassert>
#include <string>
#include <algorithm>
#include <vector>

#include <KlayGE/DiskFile/DiskFile.hpp>
#include <KlayGE/MemFile/MemFile.hpp>
#include <KlayGE/PackedFile/Pkt.hpp>

using namespace std;

#include <boost/filesystem/operations.hpp>
using namespace boost::filesystem;

namespace
{
	using namespace KlayGE;

	U32 const N(4096);			// size of ring buffer
	U32 const F(18);			// upper limit for matchLength
	U32 const THRESHOLD(2);		// encode string into position and length
	U32 const NIL(N);			// index for root of binary search trees

	unsigned char textBuf[N + F - 1];		// ring buffer of size N, 
											// with extra F-1 bytes to facilitate string comparison
	int matchPosition, matchLength;			// of longest match.  These are
											// set by the InsertNode() procedure.
	int lson[N + 1], rson[N + 257], dad[N + 1];  // left & right children &
											// parents -- These constitute binary search trees.

	void InitTree()  // initialize trees
	{
		// For i = 0 to N - 1, rson[i] and lson[i] will be the right and
		// left children of node i.  These nodes need not be initialized.
		// Also, dad[i] is the parent of node i.  These are initialized to
		// NIL (= N), which stands for 'not used.'
		// For i = 0 to 255, rson[N + i + 1] is the root of the tree
		// for strings that begin with character i.  These are initialized
		// to NIL.  Note there are 256 trees.

		for (U32 i = N + 1; i <= N + 256; ++ i)
		{
			rson[i] = NIL;
		}

		for (U32 i = 0; i < N; ++ i)
		{
			dad[i] = NIL;
		}
	}

	void InsertNode(U32 r)
		// Inserts string of length F, textBuf[r..r+F-1], into one of the
		// trees (textBuf[r]'th tree) and returns the longest-match position
		// and length via the global variables matchPosition and matchLength.
		// If matchLength = F, then removes the old node in favor of the new
		// one, because the old one will be deleted sooner.
		// Note r plays double role, as tree node and position in buffer.
	{
		U32 cmp(1);
		U8* key(&textBuf[r]);
		U32 p(N + 1 + key[0]);
		rson[r] = lson[r] = NIL;
		matchLength = 0;

		for (;;)
		{
			if (cmp >= 0)
			{
				if (rson[p] != NIL)
				{
					p = rson[p];
				}
				else
				{
					rson[p] = r;
					dad[r] = p;
					return;
				}
			}
			else
			{
				if (lson[p] != NIL)
				{
					p = lson[p];
				}
				else
				{
					lson[p] = r;
					dad[r] = p;
					return;
				}
			}

			int i(1);
			while ((i < F) && ((cmp = key[i] - textBuf[p + i]) == 0))
			{
				++ i;
			}
			if (i > matchLength)
			{
				matchPosition = p;
				if ((matchLength = i) >= F)
				{
					break;
				}
			}
		}

		dad[r]  = dad[p];
		lson[r] = lson[p];
		rson[r] = rson[p];
		dad[lson[p]] = r;
		dad[rson[p]] = r;
		if (rson[dad[p]] == p)
		{
			rson[dad[p]] = r;
		}
		else
		{
			lson[dad[p]] = r;
		}
		dad[p] = NIL;  // remove p
	}

	void DeleteNode(U32 p)  // deletes node p from tree
	{
		U32 q;

		if (NIL == dad[p])
		{
			return;  // not in tree
		}
		if (NIL == rson[p])
		{
			q = lson[p];
		}
		else
		{
			if (NIL == lson[p])
			{
				q = rson[p];
			}
			else
			{
				q = lson[p];
				if (rson[q] != NIL)
				{
					do
					{
						q = rson[q];
					} while (rson[q] != NIL);

					rson[dad[q]] = lson[q];
					dad[lson[q]] = dad[q];
					lson[q] = lson[p];
					dad[lson[p]] = q;
				}
				
				rson[q] = rson[p];
				dad[rson[p]] = q;
			}
		}

		dad[q] = dad[p];
		if (rson[dad[p]] == p)
		{
			rson[dad[p]] = q;
		}
		else
		{
			lson[dad[p]] = q;
		}
		dad[p] = NIL;
	}

	// 输出目录表
	/////////////////////////////////////////////////////////////////////////////////
	void WriteDirTable(KlayGE::VFile& out, KlayGE::DirTable const & dirTable)
	{
		using namespace KlayGE;

		for (DirTable::const_iterator iter = dirTable.begin(); iter != dirTable.end(); ++ iter)
		{
			FileDes const & fd(iter->second);

			std::string const & fileName(iter->first);
			U32 const temp(static_cast<U32>(fileName.length()));
			out.Write(&temp, sizeof(temp));
			out.Write(&fileName[0], temp);

			out.Write(&fd, sizeof(fd));
		}
	}

	// 根据树型结构打包目录
	/////////////////////////////////////////////////////////////////////////////////
	void Compress(KlayGE::VFile& outFile, DirTable& dirTable, std::string const & rootName,
		std::vector<std::string> const & files)
	{
		using namespace KlayGE;

		DiskFile openFile;
		U32 curPos(0);

		for (std::vector<std::string>::const_iterator iter = files.begin(); iter != files.end(); ++ iter)
		{
			openFile.Open(rootName + '/' + *iter, VFile::OM_Read);

			FileDes fd;
			fd.DeComLength = static_cast<U32>(openFile.Length());
			fd.crc32 = Crc32::CrcFile(openFile);
			openFile.Rewind();

			MemFile in;
			in.CopyFrom(openFile, fd.DeComLength);

			openFile.Close();

			MemFile out;
			in.Rewind();
			Pkt::Encode(out, in);
			if (out.Length() >= in.Length())
			{
				fd.attr = FA_UnCompressed;
				out.CopyFrom(in, in.Length());
			}

			fd.length	= static_cast<U32>(out.Length());
			fd.start	= curPos;
			curPos += fd.length;

			dirTable.insert(std::make_pair(*iter, fd));

			out.Rewind();
			outFile.CopyFrom(out, fd.length);
		}
	}

	// 遍历目录，得出树型结构
	/////////////////////////////////////////////////////////////////////////////////
	std::vector<std::string> FindFiles(std::string const & rootName, std::string const & pathName)
	{
		std::vector<std::string> ret;

		path findPath(rootName + '/' + pathName, native);
		if (exists(findPath))
		{
			directory_iterator end_itr;
			for (directory_iterator iter(findPath); iter != end_itr; ++ iter)
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
		for (size_t i = 0; i < out.length(); ++ i)
		{
			if ('\\' == out[i])
			{
				out[i] = '/';
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
	void Pkt::Encode(VFile& out, VFile& in)
	{
		if (0 == in.Length())
		{
			return;  // text of size zero
		}

		int lastMatchLength, codeBufPtr;
		U8 c;
		U8 codeBuf[17], mask;

		InitTree();			// initialize trees
		codeBuf[0] = 0;		// codeBuf[1..16] saves eight units of code, and
							// codeBuf[0] works as eight flags, "1" representing that the unit
							// is an unencoded letter (1 byte), "0" a position-and-length pair
							// (2 bytes).  Thus, eight units require at most 16 bytes of code.
		codeBufPtr = mask = 1;
		U32 s(0);
		U32 r(N - F);
		MemoryLib::Set(textBuf, ' ', r);	// Clear the buffer with
														// any character that will appear often.
		int len(0);
		for (; (len < F) && (in.Tell() != in.Length()); ++ len)
		{
			in.Read(&c, 1);
			textBuf[r + len] = c;  // Read F bytes into the last F bytes of the buffer
		}
		for (int i = 1; i <= F; ++ i)
		{
			InsertNode(r - i);  // Insert the F strings,
								// each of which begins with one or more 'space' characters.  Note
								// the order in which these strings are inserted.  This way,
								// degenerate trees will be less likely to occur.
		}
		InsertNode(r);  // Finally, insert the whole string just read.  The
						// global variables matchLength and matchPosition are set.
		do
		{
			if (matchLength > len)
			{
				matchLength = len;  // matchLength
									// may be spuriously long near the end of text.
			}
			if (matchLength <= THRESHOLD)
			{
				matchLength = 1;		// Not long enough match.  Send one byte.
				codeBuf[0] |= mask;		// 'send one byte' flag
				codeBuf[codeBufPtr ++] = textBuf[r];  // Send uncoded.
			}
			else
			{
				codeBuf[codeBufPtr ++] = static_cast<U8>(matchPosition);
				codeBuf[codeBufPtr ++] = static_cast<U8>(((matchPosition >> 4) & 0xF0)
					| (matchLength - (THRESHOLD + 1)));		// Send position and
															// length pair. Note matchLength > THRESHOLD.
			}
			if (0 == (mask <<= 1))
			{
				// Shift mask left one bit.
				out.Write(codeBuf, codeBufPtr);	// Send at most 8 units of
												// code together
				codeBuf[0] = 0;
				codeBufPtr = mask = 1;
			}
			lastMatchLength = matchLength;

			int i(0);
			for (; (i < lastMatchLength) && (in.Tell() != in.Length()); ++ i)
			{
				in.Read(&c, 1);
				DeleteNode(s);		// Delete old strings and
				textBuf[s] = c;		// read new bytes
				if (s < F - 1)
				{
					textBuf[s + N] = c;		// If the position is
											// near the end of buffer, extend the buffer to make
											// string comparison easier.
				}
				s = (s + 1) & (N - 1);
				r = (r + 1) & (N - 1);
				// Since this is a ring buffer, increment the position
				// modulo N.
				InsertNode(r);	// Register the string in textBuf[r..r+F-1]
			}
			while (i ++ < lastMatchLength)
			{
				// After the end of text,
				DeleteNode(s);					// no need to read, but
				s = (s + 1) & (N - 1);
				r = (r + 1) & (N - 1);
				if (-- len)
				{
					InsertNode(r);		// buffer may not be empty.
				}
			}
		} while (len > 0);	// until length of string to be processed is zero

		if (codeBufPtr > 1)
		{
			// Send remaining code.
			out.Write(codeBuf, codeBufPtr);
		}
	}

	// 目录打包
	/////////////////////////////////////////////////////////////////////////////////
	void Pkt::Pack(std::string const & dirName, VFile& pktFile)
	{
		std::string rootName;
		TransPathName(rootName, dirName);

		std::vector<std::string> files(FindFiles(rootName, ""));

		DirTable dirTable;
		MemFile tmpFile;
		Compress(tmpFile, dirTable, rootName, files);

		MemFile dt;
		WriteDirTable(dt, dirTable);

		MemFile dtCom;
		dt.Rewind();
		Encode(dtCom, dt);

		PktHeader mag;
		mag.magic			= MakeFourCC<'p', 'k', 't', ' '>::value;
		mag.ver				= 3;
		mag.DTStart			= sizeof(mag);
		mag.DTLength		= static_cast<U32>(dtCom.Length());
		mag.DTDeComLength	= static_cast<U32>(dt.Length());
		mag.FIStart			= mag.DTStart + mag.DTLength;

		pktFile.Rewind();
		pktFile.Write(&mag, sizeof(mag));
		dtCom.Rewind();
		pktFile.CopyFrom(dtCom, dtCom.Length());

		tmpFile.Rewind();
		pktFile.CopyFrom(tmpFile, tmpFile.Length());
		tmpFile.Close();
	}
}
