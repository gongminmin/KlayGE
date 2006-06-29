// LZSS.hpp
// KlayGE 打包文件读取类 头文件
// Ver 2.8.0
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
// LZSS压缩算法的作者是 Haruhiko Okumura
//
// 2.8.0
// 支持locale (2005.7.21)
//
// 2.2.0
// 统一使用istream作为资源标示符 (2004.10.26)
//
// 2.1.0
// 简化了目录表的表示法 (2004.4.14)
//
// 2.0.0
// 初次建立 (2003.9.18)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _LZSS_HPP
#define _LZSS_HPP

#include <functional>

#include <KlayGE/MapVector.hpp>

#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#pragma warning(push)
#pragma warning(disable: 4512)
#include <boost/algorithm/string/case_conv.hpp>
#pragma warning(pop)

#ifdef KLAYGE_DEBUG
	#pragma comment(lib, "KlayGE_FileSystem_PackedFile_d.lib")
#else
	#pragma comment(lib, "KlayGE_FileSystem_PackedFile.lib")
#endif

namespace KlayGE
{
#ifdef _MSC_VER
	#pragma pack(push, 1)
#endif

	struct FileDes
	{
		uint32_t		start;
		uint32_t		length;
		uint32_t		DeComLength;
		uint32_t		crc32;
		uint8_t			attr;
	};

	struct PktHeader
	{
		uint32_t		magic;
		uint32_t		ver;
		uint32_t		DTStart;
		uint32_t		DTLength;
		uint32_t		DTDeComLength;
		uint32_t		FIStart;
	};

#ifdef _MSC_VER
	#pragma pack(pop)
#endif

	// 忽略大小写比较字符串
	/////////////////////////////////////////////////////////////////////////////////
	class IgnoreCaseLessThan : public std::binary_function<std::string, std::string, bool>
	{
	public:
		IgnoreCaseLessThan()
			: cur_locale_("")
		{
		}

		bool operator()(std::string const & lhs, std::string const & rhs) const
		{
			using boost::algorithm::to_upper_copy;
			return to_upper_copy(lhs, cur_locale_) < to_upper_copy(rhs, cur_locale_);
		}

	private:
		std::locale cur_locale_;
	};

	typedef MapVector<std::string, FileDes, IgnoreCaseLessThan> DirTable;

	enum FileAttrib
	{
		FA_UnCompressed = 1,
	};

	// 翻译路径名
	/////////////////////////////////////////////////////////////////////////////////
	std::string& TransPathName(std::string& out, std::string const & in);

	// 文件打包
	/////////////////////////////////////////////////////////////////////////////////
	class Pkt : boost::noncopyable
	{
	public:
		Pkt();

		static void Encode(std::ostream& out, std::istream& in);

		void Pack(std::string const & dirName, std::ostream& pktFile);
	};


	// 打包文件读取
	/////////////////////////////////////////////////////////////////////////////////
	class UnPkt : boost::noncopyable
	{
	public:
		UnPkt();

		static void UnPkt::Decode(std::ostream& out, std::istream& in);

		void Open(boost::shared_ptr<std::istream> const & pktFile);
		
		void LocateFile(std::string const & pathName);

		size_t CurFileSize() const;
		size_t CurFileCompressedSize() const;

		bool ReadCurFile(void* data);
		void ReadCurFileCompressed(void* data);

	private:
		boost::shared_ptr<std::istream>	file_;

		DirTable	dirTable_;
		DirTable::iterator	curFile_;

		PktHeader	mag_;
	};
}

#endif		// _LZSS_HPP
