/**
 * @file WindowAndroid.cpp
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

#ifdef KLAYGE_PLATFORM_ANDROID

#include <android/window.h>

#include <KFL/Math.hpp>
#include <KFL/Util.hpp>

#include <KlayGE/App3D.hpp>
#include <KlayGE/Window.hpp>

namespace KlayGE
{
	Window::Window(std::string const & name, RenderSettings const & settings, void* native_wnd)
		: active_(false), ready_(false), closed_(false), keep_screen_on_(settings.keep_screen_on),
			dpi_scale_(1), effective_dpi_scale_(1), win_rotation_(WR_Identity)
	{
		KFL_UNUSED(name);

		a_window_ = static_cast<ANativeWindow*>(native_wnd);

		android_app* state = Context::Instance().AppState();
		state->userData = this;
		state->onAppCmd = HandleCMD;
		state->onInputEvent = HandleInput;

		while (nullptr == a_window_)
		{
			// Read all pending events.
			int ident;
			int events;
			android_poll_source* source;

			do
			{
				ident = ALooper_pollAll(0, nullptr, &events, reinterpret_cast<void**>(&source));

				// Process this event.
				if (source != nullptr)
				{
					source->process(state, source);
				}

				// Check if we are exiting.
				if (state->destroyRequested != 0)
				{
					return;
				}
			} while ((nullptr == a_window_) && (ident >= 0));
		}

		left_ = settings.left;
		top_ = settings.top;
		width_ = ANativeWindow_getWidth(a_window_);
		height_ = ANativeWindow_getHeight(a_window_);

		if (keep_screen_on_)
		{
			ANativeActivity_setWindowFlags(state->activity, AWINDOW_FLAG_KEEP_SCREEN_ON, 0);
		}
	}

	Window::~Window()
	{
		if (keep_screen_on_)
		{
			android_app* state = Context::Instance().AppState();
			ANativeActivity_setWindowFlags(state->activity, 0, AWINDOW_FLAG_KEEP_SCREEN_ON);
		}
	}

	void Window::HandleCMD(android_app* app, int32_t cmd)
	{
		Window* win = static_cast<Window*>(app->userData);
		switch (cmd)
		{
		case APP_CMD_SAVE_STATE:
			Context::Instance().AppInstance().Suspend();
			break;

		case APP_CMD_RESUME:
			if (win->ready_)
			{
				Context::Instance().AppInstance().Resume();
			}
			break;

		case APP_CMD_INIT_WINDOW:
			win->a_window_ = app->window;
			break;
		
		case APP_CMD_TERM_WINDOW:
			win->OnClose()(*win);
			win->active_ = false;
			win->ready_ = false;
			win->closed_ = true;
			break;

		case APP_CMD_GAINED_FOCUS:
			win->active_ = true;
			win->ready_ = true;
			win->OnActive()(*win, true);
			break;

		case APP_CMD_LOST_FOCUS:
			win->active_ = false;
			win->OnActive()(*win, false);
			break;

		case APP_CMD_WINDOW_RESIZED:
		case APP_CMD_CONTENT_RECT_CHANGED:
			win->left_ = app->contentRect.left;
			win->top_ = app->contentRect.top;
			win->width_ = app->contentRect.right;
			win->height_ = app->contentRect.bottom;
			win->active_ = true;
			win->ready_ = true;
			win->OnSize()(*win, true);
			break;
		}
	}
	
	int32_t Window::HandleInput(android_app* app, AInputEvent* event)
	{
		Window* win = static_cast<Window*>(app->userData);
		int32_t source = AInputEvent_getSource(event);
		switch (AInputEvent_getType(event))
		{
		case AINPUT_EVENT_TYPE_MOTION:
			{
				int32_t action = AMotionEvent_getAction(event);
				int32_t action_code = action & AMOTION_EVENT_ACTION_MASK;
				int32_t pointer_index = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK)
					>> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
				switch (source)
				{
				case AINPUT_SOURCE_MOUSE:
					{
						BOOST_ASSERT(1 == AMotionEvent_getPointerCount(event));

						int2 pt(AMotionEvent_getX(event, pointer_index), AMotionEvent_getY(event, pointer_index));
						int32_t buttons = AMotionEvent_getButtonState(event);
						switch (action_code)
						{
						case AMOTION_EVENT_ACTION_DOWN:
							win->OnMouseDown()(*win, pt, buttons);
							break;

						case AMOTION_EVENT_ACTION_UP:
							win->OnMouseUp()(*win, pt, buttons);
							break;

						case AMOTION_EVENT_ACTION_SCROLL:
							win->OnMouseWheel()(*win, pt,
								AMotionEvent_getAxisValue(event, AMOTION_EVENT_AXIS_VSCROLL, pointer_index));
							break;

						default:
							win->OnMouseMove()(*win, pt);
							break;
						}
					}
					break;

				case AINPUT_SOURCE_TOUCHSCREEN:
					switch (action_code)
					{
					case AMOTION_EVENT_ACTION_DOWN:
					case AMOTION_EVENT_ACTION_POINTER_DOWN:
						win->OnPointerDown()(*win,
							int2(AMotionEvent_getX(event, pointer_index), AMotionEvent_getY(event, pointer_index)),
							AMotionEvent_getPointerId(event, pointer_index) + 1);
						break;

					case AMOTION_EVENT_ACTION_UP:
					case AMOTION_EVENT_ACTION_POINTER_UP:
						win->OnPointerUp()(*win,
							int2(AMotionEvent_getX(event, pointer_index), AMotionEvent_getY(event, pointer_index)),
							AMotionEvent_getPointerId(event, pointer_index) + 1);
						break;

					case AMOTION_EVENT_ACTION_MOVE:
						for (size_t i = 0; i < AMotionEvent_getPointerCount(event); ++i)
						{
							win->OnPointerUpdate()(*win,
								int2(AMotionEvent_getX(event, i), AMotionEvent_getY(event, i)),
								AMotionEvent_getPointerId(event, i) + 1, true);
						}
						break;

					default:
						break;
					}
					break;

				case AINPUT_SOURCE_JOYSTICK:
					{
						for (uint32_t i = 0; i < 8; i++)
						{
							win->OnJoystickAxis()(*win, i, AMotionEvent_getAxisValue(event, i, 0));
						}
					}
					break;

				default:
					break;
				}	
			}
			return 1;

		case AINPUT_EVENT_TYPE_KEY:
			switch (source)
			{
			case AINPUT_SOURCE_KEYBOARD:
				{
					int32_t key_code = AKeyEvent_getKeyCode(event);
					switch (AKeyEvent_getAction(event))
					{
					case AKEY_EVENT_ACTION_DOWN:
						win->OnKeyDown()(*win, key_code);
						break;

					case AKEY_EVENT_ACTION_UP:
						win->OnKeyUp()(*win, key_code);
						break;

					default:
						break;
					}
				}
				break;

			case AINPUT_SOURCE_JOYSTICK:
				win->OnJoystickButtons()(*win, AMotionEvent_getButtonState(event));
				break;

			default:
				break;
			}
			return 1;
		}
		return 0;
	}
}

#endif
