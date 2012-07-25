#ifndef _JTMLWRITER_HPP
#define _JTMLWRITER_HPP

#pragma once

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/RenderStateObject.hpp>

#include <string>
#include <vector>
#include <fstream>

namespace KlayGE
{
	inline char const * GetTextureAddressTypeName(TexAddressingMode type)
	{
		switch(type)
		{
		case TAM_Wrap:
			return "wrap";

		case TAM_Mirror:
			return "mirror";

		case TAM_Clamp:
			return "clamp";

		case TAM_Border:
		default:
			return "broder";
		}
	}

	struct JTMLImageRecord
	{
		std::string name_;
		int x_, y_, h_, w_;
		TexAddressingMode u_, v_;
	};
	typedef std::vector<JTMLImageRecord> JTMLImageRecordArray;

	inline const char* GetElementFormatName(ElementFormat type)
	{
		switch(type)
		{
		case EF_ABGR8:
			return "ABGR8";

		default:
			return NULL;
		}
	}

	class JTMLWriter
	{
	public:
		void WriteToJTML(std::string const & name);
		void SetTileSize(int tile_size)
		{
			tile_size_ = tile_size;
		}
		void SetTileNum(int tile_num)
		{
			tile_num_ = tile_num;
		}
		void SetElementFormat(ElementFormat fmt)
		{
			format_ = fmt;
		}
		void InsertImageRecord(JTMLImageRecord& jir)
		{
			image_records_.push_back(jir);
		}
		void ResetData();
		JTMLImageRecordArray& GetImageRecordArray()
		{
			return image_records_;
		}

		static JTMLWriter& Instance()
		{
			static JTMLWriter writer;
			return writer;
		}
		JTMLWriter()
		{
			this->ResetData();
		}

	private:
		int tile_size_;
		int tile_num_;
		ElementFormat format_;
		JTMLImageRecordArray image_records_;
		std::ofstream ofs_;
		int indent_;

		void PushIndent()
		{
			++ indent_;
		}
		void PopIndent()
		{
			-- indent_;
		}
		std::ostream& Out();
		std::ostream& Enter();
		void PrintHead();
		void BeginJuda();
		void EndJuda();
		void PrintImageRecord(JTMLImageRecord& jir);
	};
}

#endif		// _JTMLWRITER_HPP
