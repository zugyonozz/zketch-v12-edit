#include "window copy.hpp"
#include "inputsystem.hpp"
#include "slider copy.hpp"
#include "button copy.hpp"
#include "textbox copy.hpp"

namespace zketch {
	void zketch_init() noexcept {
		AppRegistry::RegisterWindowClass() ;
		EventSystem::Initialize() ;
	}
}

int main() {
    zketch::zketch_init();
    
    zketch::Window window("Test Window", 800, 600);
    zketch::Button button({100, 100, 200, 50}, L"Click Me");
    zketch::Slider slider(zketch::Slider::Horizontal, {100, 200, 300, 20}, {20, 30});
    zketch::TextBox textbox({100, 300, 400, 100}, L"Hello World", zketch::Font());
    
    button.SetOnClick([]() {
        zketch::logger::info("Button clicked!");
    });
    
    window.Show();
    
    zketch::InputSystem input;
    
    while (zketch::Application) {
        zketch::Event e;
        while (zketch::PollEvent(e)) {
            if (e == zketch::EventType::Close) {
                window.Quit();
            }
            
            if (e.IsMouseEvent()) {
                auto pos = e.GetMousePosition();
                zketch::PointF posf{static_cast<float>(pos.x), static_cast<float>(pos.y)};
                
                button.OnHover(posf);
                slider.OnHover(posf);
                
                if (e.GetMouseState() == zketch::MouseState::Down) {
                    button.OnPress(posf);
                    slider.OnPress(posf);
                }
                
                if (e.GetMouseState() == zketch::MouseState::Up) {
                    button.OnRelease(posf);
                    slider.OnRelease();
                }
                
                slider.OnDrag(posf);
            }
        }
        
        window.Display(); // Final blit - NO FLICKER!
        
        input.Update();
    }
    
    return 0;
}