// MultiresPostProcess.hpp
//////////////////////////////////////////////////////////////////////////////////

#ifndef _MULTIRESPOSTPROCESS_HPP
#define _MULTIRESPOSTPROCESS_HPP

#pragma once

#include <vector>

#include <boost/noncopyable.hpp>

#include <KlayGE/PreDeclare.hpp>
#include <KlayGE/PostProcess.hpp>

namespace KlayGE
{
	class MultiresPostProcess;
	typedef boost::shared_ptr<MultiresPostProcess> MultiresPostProcessPtr;

	class MultiresPostProcess : public PostProcess
	{
	public:
		explicit MultiresPostProcess(std::wstring const & name);
		MultiresPostProcess(std::wstring const & name,
			uint32_t num_levels,
			uint32_t base_level,
			std::vector<std::string> const & param_names,
			std::vector<std::string> const & input_pin_names,
			std::vector<std::string> const & output_pin_names,
			RenderTechniquePtr const & tech);
		virtual ~MultiresPostProcess()
		{
		}

		void ActivateSubLevels(uint32_t num_levels, uint32_t base_level = 0);

	private:
		void CreateVB();
		void CreateMultiresQuads();

	protected:
		uint32_t num_levels_;
		uint32_t base_level_;

		GraphicsBufferPtr pos_tc_vb_;
	};

	MultiresPostProcessPtr LoadMultiresPostProcess(ResIdentifierPtr const & ppml, std::string const & pp_name, uint32_t num_levels, uint32_t base_level = 0);
}

#endif		// _MULTIRESPOSTPROCESS_HPP
