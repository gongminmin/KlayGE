#include <KlayGE/KlayGE.hpp>
#include <KlayGE/Config.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Memory.hpp>
#include <KlayGE/Timer.hpp>
#include <KlayGE/SceneManager.hpp>

#include <KlayGE/Engine.hpp>

namespace KlayGE
{
	App3DFramework* Engine::app_;
	SceneManager*	Engine::sceneManager_;

	RenderFactory*	Engine::renderFactory_;
	AudioFactory*	Engine::audioFactory_;
	InputFactory*	Engine::inputFactory_;

	Engine::Engine()
	{
		sceneManager_ = new SceneManager;
	}

	Engine::~Engine()
	{
		SafeDelete(sceneManager_);
	}

	Engine engine;
}