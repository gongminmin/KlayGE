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

#pragma once

#include <string_view>
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

	class XMLNode final
	{
	public:
		explicit XMLNode(XMLNodeType type, std::string_view name);
		XMLNode(XMLNode const& rhs);
		XMLNode(XMLNode&& rhs) noexcept;
		~XMLNode() noexcept;

		XMLNode& operator=(XMLNode const& rhs);
		XMLNode& operator=(XMLNode&& rhs) noexcept;

		std::string_view Name() const noexcept;
		void Name(std::string_view name) noexcept;

		XMLNodeType Type() const noexcept;

		XMLNode const* Parent() const noexcept;
		XMLNode* Parent() noexcept;
		void Parent(XMLNode* parent) noexcept;

		XMLAttribute const* FirstAttrib(std::string_view name) const;
		XMLAttribute* FirstAttrib(std::string_view name);
		XMLAttribute const* NextAttrib(XMLAttribute const& attrib, std::string_view name) const;
		XMLAttribute* NextAttrib(XMLAttribute const& attrib, std::string_view name);
		XMLAttribute const* LastAttrib(std::string_view name) const;
		XMLAttribute* LastAttrib(std::string_view name);
		XMLAttribute const* FirstAttrib() const;
		XMLAttribute* FirstAttrib();
		XMLAttribute const* NextAttrib(XMLAttribute const& attrib) const;
		XMLAttribute* NextAttrib(XMLAttribute const& attrib);
		XMLAttribute const* LastAttrib() const;
		XMLAttribute* LastAttrib();

		XMLAttribute const* Attrib(std::string_view name) const;
		XMLAttribute* Attrib(std::string_view name);

		bool TryConvertAttrib(std::string_view name, bool& val, bool default_val) const;
		bool TryConvertAttrib(std::string_view name, int32_t& val, int32_t default_val) const;
		bool TryConvertAttrib(std::string_view name, uint32_t& val, uint32_t default_val) const;
		bool TryConvertAttrib(std::string_view name, float& val, float default_val) const;

		bool AttribBool(std::string_view name, bool default_val) const;
		int32_t AttribInt(std::string_view name, int32_t default_val) const;
		uint32_t AttribUInt(std::string_view name, uint32_t default_val) const;
		float AttribFloat(std::string_view name, float default_val) const;
		std::string_view AttribString(std::string_view name, std::string_view default_val) const;

		XMLNode const* FirstNode(std::string_view name) const;
		XMLNode* FirstNode(std::string_view name);
		XMLNode const* LastNode(std::string_view name) const;
		XMLNode* LastNode(std::string_view name);
		XMLNode const* FirstNode() const;
		XMLNode* FirstNode();
		XMLNode const* LastNode() const;
		XMLNode* LastNode();

		XMLNode const* PrevSibling(std::string_view name) const;
		XMLNode* PrevSibling(std::string_view name);
		XMLNode const* NextSibling(std::string_view name) const;
		XMLNode* NextSibling(std::string_view name);
		XMLNode const* PrevSibling() const;
		XMLNode* PrevSibling();
		XMLNode const* NextSibling() const;
		XMLNode* NextSibling();

		uint32_t FindChildNodeIndex(XMLNode const& node) const;
		void InsertAt(uint32_t index, XMLNode new_node);

		void InsertAfterNode(XMLNode const& location, XMLNode new_node);
		void InsertAfterAttrib(XMLAttribute const& location, XMLAttribute new_attr);
		void AppendNode(XMLNode new_node);
		void AppendAttrib(XMLAttribute new_attr);

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
		class Impl;
		std::unique_ptr<Impl> pimpl_;
	};

	class XMLAttribute final
	{
	public:
		XMLAttribute(std::string_view name);
		XMLAttribute(std::string_view name, bool value);
		XMLAttribute(std::string_view name, int32_t value);
		XMLAttribute(std::string_view name, uint32_t value);
		XMLAttribute(std::string_view name, float value);
		XMLAttribute(std::string_view name, std::string_view value);
		XMLAttribute(XMLAttribute const& rhs);
		XMLAttribute(XMLAttribute&& rhs) noexcept;
		~XMLAttribute() noexcept;

		XMLAttribute& operator=(XMLAttribute const& rhs);
		XMLAttribute& operator=(XMLAttribute&& rhs) noexcept;

		std::string_view Name() const noexcept;
		void Name(std::string_view name) noexcept;

		XMLNode const* Parent() const noexcept;
		XMLNode* Parent() noexcept;
		void Parent(XMLNode* parent) noexcept;

		XMLAttribute const* NextAttrib(std::string_view name) const;
		XMLAttribute* NextAttrib(std::string_view name);
		XMLAttribute const* NextAttrib() const;
		XMLAttribute* NextAttrib();

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
		class Impl;
		std::unique_ptr<Impl> pimpl_;
	};

	XMLNode LoadXml(ResIdentifier& source);
	void SaveXml(XMLNode const& node, std::ostream& os);
} // namespace KlayGE
