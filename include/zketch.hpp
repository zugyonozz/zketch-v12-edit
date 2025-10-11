#pragma once
#include "renderer.hpp"
#include "inputsystem.hpp"
#include "slider.hpp"
#include "button.hpp"
#include "textbox.hpp"
#include "inputbox.hpp"

namespace zketch {
	void zketch_init() noexcept {
		AppRegistry::RegisterWindowClass() ;
		EventSystem::Init() ;
	}
}