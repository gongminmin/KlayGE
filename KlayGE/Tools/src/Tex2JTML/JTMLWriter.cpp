#include <KlayGE/KlayGE.hpp>

#include "JTMLWriter.hpp"

#include <fstream>

namespace KlayGE
{
	void JTMLWriter::WriteToJTML(std::string const & name)
	{
		ofs_ = std::ofstream(name);
		if (!ofs_)
		{
			return;
		}

		this->PrintHead();
		this->Enter();
		this->BeginJuda();
		this->PushIndent();
		for (size_t i = 0; i < image_records_.size(); ++ i)
		{
			this->PrintImageRecord(image_records_[i]);
		}
		this->PopIndent();
		this->EndJuda();
		ofs_.close();
	}

	void JTMLWriter::ResetData()
	{
		tile_num_ = 0;
		tile_size_ = 0;
		format_ = EF_ABGR8;
		image_records_.clear();
		indent_ = 0;
	}

	std::ostream& JTMLWriter::Out()
	{
		for (int i = 0; i < indent_; ++ i)
		{
			ofs_ << "\t";
		}
		return ofs_;
	}

	std::ostream& JTMLWriter::Enter()
	{
		ofs_ << "\n";
		return ofs_;
	}

	void JTMLWriter::PrintHead()
	{
		this->Out() << "<?xml version='1.0'?>\n";
	}

	void JTMLWriter::BeginJuda()
	{
		this->Out() << "<juda_tex num_tiles=\"" << tile_num_
			<< "\" tile_size=\"" << tile_size_
			<<"\" format=\"" << GetElementFormatName(format_) << "\">\n";
	}

	void JTMLWriter::EndJuda()
	{
		this->Out() << "</juda_tex>\n";
	}

	void JTMLWriter::PrintImageRecord(JTMLImageRecord& jir)
	{
		this->Out() << "<image name=\"" << jir.name_
			<< "\" x=\"" << jir.x_ << "\" y=\"" << jir.y_ << "\" w=\"" << jir.w_ << "\" h=\"" << jir.h_
			<<"\" address_u=\"" << GetTextureAddressTypeName(jir.u_)
			<<"\" address_v=\"" << GetTextureAddressTypeName(jir.v_) << "\"/>\n";
	}
}
