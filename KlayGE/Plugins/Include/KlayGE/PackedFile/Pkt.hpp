// Pkt.hpp
// KlayGE 打包文件读取类 头文件
// Ver 2.1.0
// 版权所有(C) 龚敏敏, 2003-2004
// Homepage: http://klayge.sourceforge.net
// LZSS压缩算法的作者是 Haruhiko Okumura
//
// 2.1.0
// 简化了目录表的表示法 (2004.4.14)
//
// 2.0.0
// 初次建立 (2003.9.18)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#ifndef _PKT_HPP
#define _PKT_HPP

#include <KlayGE/VFile.hpp>
#include <KlayGE/MapVector.hpp>

#pragma comment(lib, "KlayGE_FileSystem_PackedFile.lib")

namespace KlayGE
{
	#ifdef _MSC_VER
		#pragma pack(push, 1)
	#endif

	struct FileDes
	{
		U32			start;
		U32			length;
		U32			DeComLength;
		U32			crc32;
		U8			attr;
	};

	struct PktHeader
	{
		U32		magic;
		U32		ver;
		U32		DTStart;
		U32		DTLength;
		U32		DTDeComLength;
		U32		FIStart;
	};

	#ifdef _MSC_VER
		#pragma pack(pop)
	#endif

	typedef MapVector<String, FileDes> DirTable;

	enum FileAttrib
	{
		FA_UnCompressed = 1,
	};

	// 翻译路径名
	/////////////////////////////////////////////////////////////////////////////////
	String& TransPathName(String& out, const String& in);

	// 文件打包
	/////////////////////////////////////////////////////////////////////////////////
	class Pkt
	{
	public:
		static void Encode(VFile& Out, VFile& In);

		void Pack(const String& dirName, VFile& pktFile);

		Pkt();

	private:
		Pkt(const Pkt& rhs);
		Pkt& operator=(const Pkt& rhs);
	};


	// 打包文件读取
	/////////////////////////////////////////////////////////////////////////////////
	class UnPkt
	{
	public:
		static void Decode(VFile& Out, VFile& In);

		void Open(const VFilePtr& pktFile);
		void Close();
		
		void LocateFile(const String& pathName);

		size_t CurFileSize() const;
		size_t CurFileCompressedSize() const;

		bool ReadCurFile(void* data);
		void ReadCurFileCompressed(void* data);

		UnPkt();
		~UnPkt();

	private:
		VFilePtr	file_;

		DirTable	dirTable_;
		DirTable::iterator	curFile_;

		PktHeader	mag_;

		UnPkt(const UnPkt& rhs);
		UnPkt& operator=(const UnPkt& rhs);
	};
}

#endif		// _UNPKT_HPP
