/**
 * @file XMLDom.hpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KFL, a subproject of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

#ifndef KFL_XML_DOM_HPP
#define KFL_XML_DOM_HPP

#pragma once

#include <iosfwd>
#include <vector>

#include <KFL/Noncopyable.hpp>

namespace KlayGE
{
	class XMLAttribute;
	class XMLNode;

	enum class XMLNodeType
	{
		Document,
		Element,
		Data,
		CData,
		Comment,
		Declaration,
		Doctype,
		PI
	};

	class XMLDocument
	{
		KLAYGE_NONCOPYABLE(XMLDocument);

	public:
		XMLDocument() noexcept;

		XMLNode* RootNode() const;
		void RootNode(std::unique_ptr<XMLNode> new_node);

		std::unique_ptr<XMLNode> CloneNode(XMLNode const& node);
		std::unique_ptr<XMLAttribute> CloneAttrib(XMLAttribute const& attrib);

		std::unique_ptr<XMLNode> AllocNode(XMLNodeType type, std::string_view name);
		std::unique_ptr<XMLAttribute> AllocAttrib(std::string_view name);
		std::unique_ptr<XMLAttribute> AllocAttribBool(std::string_view name, bool value);
		std::unique_ptr<XMLAttribute> AllocAttribInt(std::string_view name, int32_t value);
		std::unique_ptr<XMLAttribute> AllocAttribUInt(std::string_view name, uint32_t value);
		std::unique_ptr<XMLAttribute> AllocAttribFloat(std::string_view name, float value);
		std::unique_ptr<XMLAttribute> AllocAttribString(std::string_view name, std::string_view value);

	private:
		std::unique_ptr<XMLNode> root_;
	};

	class XMLNode final
	{
		KLAYGE_NONCOPYABLE(XMLNode);

	public:
		explicit XMLNode(XMLNodeType type);

		std::string_view Name() const;
		void Name(std::string_view name);

		XMLNodeType Type() const;

		XMLNode* Parent() const;
		void Parent(XMLNode* parent);

		XMLAttribute* FirstAttrib(std::string_view name) const;
		XMLAttribute* NextAttrib(XMLAttribute const& attrib, std::string_view name) const;
		XMLAttribute* LastAttrib(std::string_view name) const;
		XMLAttribute* FirstAttrib() const;
		XMLAttribute* NextAttrib(XMLAttribute const& attrib) const;
		XMLAttribute* LastAttrib() const;

		XMLAttribute* Attrib(std::string_view name) const;

		bool TryConvertAttrib(std::string_view name, bool& val, bool default_val) const;
		bool TryConvertAttrib(std::string_view name, int32_t& val, int32_t default_val) const;
		bool TryConvertAttrib(std::string_view name, uint32_t& val, uint32_t default_val) const;
		bool TryConvertAttrib(std::string_view name, float& val, float default_val) const;

		bool AttribBool(std::string_view name, bool default_val) const;
		int32_t AttribInt(std::string_view name, int32_t default_val) const;
		uint32_t AttribUInt(std::string_view name, uint32_t default_val) const;
		float AttribFloat(std::string_view name, float default_val) const;
		std::string_view AttribString(std::string_view name, std::string_view default_val) const;

		XMLNode* FirstNode(std::string_view name) const;
		XMLNode* LastNode(std::string_view name) const;
		XMLNode* FirstNode() const;
		XMLNode* LastNode() const;

		XMLNode* PrevSibling(std::string_view name) const;
		XMLNode* NextSibling(std::string_view name) const;
		XMLNode* PrevSibling() const;
		XMLNode* NextSibling() const;

		void InsertAfterNode(XMLNode const& location, std::unique_ptr<XMLNode> new_node);
		void InsertAfterAttrib(XMLAttribute const& location, std::unique_ptr<XMLAttribute> new_attr);
		void AppendNode(std::unique_ptr<XMLNode> new_node);
		void AppendAttrib(std::unique_ptr<XMLAttribute> new_attr);

		void RemoveNode(XMLNode const& node);
		void RemoveAttrib(XMLAttribute const& attr);

		void ClearChildren();
		void ClearAttribs();

		bool TryConvertValue(bool& val) const;
		bool TryConvertValue(int32_t& val) const;
		bool TryConvertValue(uint32_t& val) const;
		bool TryConvertValue(float& val) const;

		bool ValueBool() const;
		int32_t ValueInt() const;
		uint32_t ValueUInt() const;
		float ValueFloat() const;
		std::string_view ValueString() const;

		void Value(bool value);
		void Value(int32_t value);
		void Value(uint32_t value);
		void Value(float value);
		void Value(std::string_view value);

	private:
		XMLNode* parent_{};

		XMLNodeType type_;
		std::string name_;
		std::string value_;

		std::vector<std::unique_ptr<XMLNode>> children_;
		std::vector<std::unique_ptr<XMLAttribute>> attrs_;
	};

	class XMLAttribute final
	{
		KLAYGE_NONCOPYABLE(XMLAttribute);

	public:
		XMLAttribute();

		std::string_view Name() const;
		void Name(std::string_view name);

		XMLNode* Parent() const;
		void Parent(XMLNode* parent);

		XMLAttribute* NextAttrib(std::string_view name) const;
		XMLAttribute* NextAttrib() const;

		bool TryConvertValue(bool& val) const;
		bool TryConvertValue(int32_t& val) const;
		bool TryConvertValue(uint32_t& val) const;
		bool TryConvertValue(float& val) const;

		bool ValueBool() const;
		int32_t ValueInt() const;
		uint32_t ValueUInt() const;
		float ValueFloat() const;
		std::string_view ValueString() const;

		void Value(bool value);
		void Value(int32_t value);
		void Value(uint32_t value);
		void Value(float value);
		void Value(std::string_view value);

	private:
		XMLNode* parent_{};

		std::string name_;
		std::string value_;
	};

	std::unique_ptr<XMLDocument> LoadXml(ResIdentifier& source);
	void SaveXml(XMLDocument const& dom, std::ostream& os);
} // namespace KlayGE

#endif // KFL_XML_DOM_HPP
