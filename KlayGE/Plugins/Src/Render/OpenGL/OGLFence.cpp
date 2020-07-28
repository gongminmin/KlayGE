/**
 * @file OGLFence.cpp
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
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


#include <KlayGE/KlayGE.hpp>
#include <KFL/Util.hpp>
#include <KFL/Math.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <glloader/glloader.h>

#include <KlayGE/OpenGL/OGLRenderEngine.hpp>
#include <KlayGE/OpenGL/OGLFence.hpp>

namespace KlayGE
{
	OGLFence::OGLFence() = default;

	OGLFence::~OGLFence()
	{
		for (auto& f : fences_)
		{
			glDeleteSync(f.second);
		}
	}

	uint64_t OGLFence::Signal(FenceType ft)
	{
		KFL_UNUSED(ft);

		uint64_t const id = fence_val_;
		fences_[id] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		++ fence_val_;
		return id;
	}

	void OGLFence::Wait(uint64_t id)
	{
		auto iter = fences_.find(id);
		if (iter != fences_.end())
		{
			glFlush();
			glWaitSync(iter->second, 0, GL_TIMEOUT_IGNORED);
			glDeleteSync(iter->second);
			fences_.erase(iter);
		}
	}

	bool OGLFence::Completed(uint64_t id)
	{
		auto iter = fences_.find(id);
		if (iter == fences_.end())
		{
			return true;
		}
		else
		{
			GLint status;
			glGetSynciv(iter->second, GL_SYNC_STATUS, sizeof(status), nullptr, &status);
			return (GL_SIGNALED == status);
		}
	}
}
