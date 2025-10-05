#pragma once
#include "canvas.hpp"
#include "event.hpp"

namespace zketch {

	inline LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) ;

	class Application {
		friend inline LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) ;
		friend class Window ;

	private :
		static inline std::unordered_map<HWND, Window*> g_windows_ ;
		static inline bool app_is_runing_ = true ;
	
	public :
		static void QuitProgram() noexcept {
			for (auto& w : g_windows_) {
				if (IsWindow(w.first)) {
					DestroyWindow(w.first) ;
				}
			}

			g_windows_.clear() ;
			app_is_runing_ = false ;
			PostQuitMessage(0) ;

			#ifdef APPLICATION_DEBUG
				logger::info("Application::QuitProgram - PostQuitMessage done.") ;
			#endif
		}

		static bool IsRunning() noexcept {
			return app_is_runing_ ;
		}

	} ;

	namespace AppRegistry {

		static inline HINSTANCE g_hintance_ = GetModuleHandleW(nullptr) ;
		static inline std::string g_window_class_name_ = "zketch_app" ;
		static inline bool window_was_registered = false ;

		static void SetWindowClass(std::string&& windowclassname) noexcept {
			if (!window_was_registered) {
				
				#ifdef APPREGISTRY_DEBUG
					logger::warning("AppRegistry::SetWindowClass - Failed to register window class name, window class name was registered.") ;
				#endif

				return ;
			} 

			g_window_class_name_ = std::move(windowclassname) ;
		}

		static void RegisterWindowClass() {
			if (window_was_registered) {

				#ifdef APPREGISTRY_DEBUG
					logger::warning("AppRegistry::RegisterWindowClass - Failed to register window class name, window class name was registered.") ;
				#endif

				return ;
			}

			WNDCLASSEX wc = {
				sizeof(wc),
				CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
				wndproc,
				0,
				0,
				AppRegistry::g_hintance_,
				LoadIcon(nullptr, IDI_APPLICATION),
				LoadCursor(nullptr, IDC_ARROW),
				nullptr,
				nullptr,
				AppRegistry::g_window_class_name_.c_str(),
				LoadIcon(nullptr, IDI_APPLICATION)
			} ;

			if (!RegisterClassEx(&wc)) {

				#ifdef APPREGISTRY_DEBUG
					logger::error("AppRegistry::RegisterWindowClass - Failed to register window class!") ;
				#endif

				return ;
			}

			#ifdef APPREGISTRY_DEBUG
				logger::info("AppRegistry::RegisterWindowClass - Successfully register window class.") ;
			#endif

			window_was_registered = true ;
		}
	} ;

	class Window {
		friend inline LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) ;
		friend class Renderer ;

	private :
		HWND handle_ = nullptr ;
		std::unique_ptr<Canvas> front_buffer_ ;
		std::unique_ptr<Canvas> back_buffer_ ;

		void CreateCanvas(const Size& size) noexcept {
			if (!front_buffer_) {
				front_buffer_ = std::make_unique<Canvas>() ;
			}

			if (!back_buffer_) {
				back_buffer_ = std::make_unique<Canvas>() ;
			}

			if (!front_buffer_->Create(size)) {

				#ifdef WINDOW_DEBUG
					logger::error("Window::CreateCanvas - failed to create front buffer canvas.") ;
				#endif

				return ;
			}

			if (!back_buffer_->Create(size)) {

				#ifdef WINDOW_DEBUG
					logger::error("Window::CreateCanvas - failed to create back buffer canvas.") ;
				#endif

				return ;
			}

			#ifdef WINDOW_DEBUG
				logger::info("Window::CreateCanvas - Successfully create with size : [", size.x, "x", size.y, "] .") ;
			#endif

		}

		bool IsCanvasValid() const noexcept {
			return front_buffer_->IsValid() && back_buffer_->IsValid() ;
		}

	public :
		Window(const Window&) = delete ;
		Window& operator=(const Window&) = delete ;

		Window(const char* title, int32_t width, int32_t height) noexcept {
			handle_ = CreateWindowEx(
				0,
				AppRegistry::g_window_class_name_.c_str(),
				title,
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				width,
				height,
				nullptr,
				nullptr,
				AppRegistry::g_hintance_,
				nullptr
			) ;

			if (!handle_) {

				#ifdef WINDOW_DEBUG
					logger::error("Window::Window - Failed to create Window, window isn't valid.") ;
				#endif

				return ;
			}

			Application::g_windows_.emplace(handle_, this) ;

			#ifdef WINDOW_DEBUG
				logger::info("Window::Window - Create Window success.") ;
			#endif

			CreateCanvas(GetClientBound().GetSize()) ;
		}

		Window(const char* title, int32_t xpos, int32_t ypos, int32_t width, int32_t height) noexcept {
			handle_ = CreateWindowEx(
				0,
				AppRegistry::g_window_class_name_.c_str(),
				title,
				WS_OVERLAPPEDWINDOW,
				xpos,
				ypos,
				width,
				height,
				nullptr,
				nullptr,
				AppRegistry::g_hintance_,
				nullptr
			) ;

			if (!handle_) {

				#ifdef WINDOW_DEBUG
					logger::error("Window::Window - Failed to create Window, window isn't valid.") ;
				#endif

				return ;
			}

			Application::g_windows_.emplace(handle_, this) ;

			#ifdef WINDOW_DEBUG
				logger::info("Window::Window - Create Window success.") ;
			#endif

			CreateCanvas(GetClientBound().GetSize()) ;
		}

		Window(Window&& o) noexcept : handle_(std::exchange(o.handle_, nullptr)), front_buffer_(std::move(o.front_buffer_)) {

			#ifdef WINDOW_DEBUG
				logger::info("Window::Window - Calling move ctor.") ;
			#endif

			if (handle_) {
				auto it = Application::g_windows_.find(o.handle_) ;
				if (it != Application::g_windows_.end()) 
					Application::g_windows_.erase(it) ;
				Application::g_windows_[handle_] = this ;
			}
		}

		~Window() noexcept {

			#ifdef WINDOW_DEBUG
				logger::info("Window::~Window - Calling window dtor") ;
			#endif

			if (handle_) {
				if (IsWindow(handle_)) {
					DestroyWindow(handle_) ;
				}
				handle_ = nullptr ;

				#ifdef WINDOW_DEBUG
					logger::info("Window::~Window - Success to set hwnd to nullptr.") ;
				#endif

			} else {

				#ifdef WINDOW_DEBUG
					logger::warning("Window::~Window - Failed to set hwnd, hwnd is already nullptr.") ;
				#endif

			}
		}

		Window& operator=(Window&& o) noexcept {

			#ifdef WINDOW_DEBUG
				logger::info("Window::operator= - Calling move assignment.") ;
			#endif

			if (this != &o) {
				handle_ = std::exchange(o.handle_, nullptr) ;
				front_buffer_ = std::move(o.front_buffer_) ;
				
				auto it = Application::g_windows_.find(o.handle_) ;
				if (it != Application::g_windows_.end()) {
					Application::g_windows_.erase(it) ;
				}

				Application::g_windows_[handle_] = this ;
			}

			return *this ;
		}

		void Show() const noexcept {
			if (handle_) {
				ShowWindow(handle_, SW_SHOWDEFAULT) ;
				UpdateWindow(handle_) ;
			}
		}

		void Hide() const noexcept {
			if (handle_) {
				ShowWindow(handle_, SW_HIDE) ; 
			}
		}

		void Minimize() noexcept { 
			if (handle_) {
				ShowWindow(handle_, SW_MINIMIZE) ; 
			}
		}

		void Maximize() noexcept { 
			if (handle_) {
				ShowWindow(handle_, SW_MAXIMIZE) ; 
			}
		}

		void Restore() noexcept { 
			if (handle_) {
				ShowWindow(handle_, SW_RESTORE) ; 
			}
		}

		void Close() noexcept {
			if (handle_) {
				DestroyWindow(handle_) ;
			}
		}

		void Close() const noexcept {
			if (handle_) {

				#ifdef WINDOW_DEBUG
					logger::info("Quit() called for window: ", handle_) ;
				#endif

				DestroyWindow(handle_);
			} else {

				#ifdef WINDOW_DEBUG
					logger::warning("Quit() failed: invalid window handle") ;
				#endif

			}
		}

		void Present() const noexcept {
			if (!front_buffer_ || !front_buffer_->IsValid()) {
				#ifdef WINDOW_DEBUG
					logger::warning("Window::Present - Invalid canvas!") ;
				#endif

				return ;
			}

			HDC hdc = GetDC(handle_) ;
			if (!hdc) {

				#ifdef WINDOW_DEBUG
					logger::warning("Window::Present - Invalid HDC!") ;
				#endif

				return ;
			}

			Gdiplus::Graphics screen(hdc) ;
			screen.SetCompositingMode(Gdiplus::CompositingModeSourceOver) ;
			screen.SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed) ;
			screen.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor) ;
			screen.DrawImage(front_buffer_->GetBitmap(), 0, 0) ;

			ReleaseDC(handle_, hdc) ;
		}

		void SetTitle(const char* title) noexcept {
			if (handle_) {
				SetWindowText(handle_, title) ;
			}
		}

		Rect GetClientBound() const noexcept {
			tagRECT r ;
			GetClientRect(handle_, &r) ;
			return static_cast<Rect>(r) ;
		}

		Rect GetWindowBound() const noexcept {
			tagRECT r ;
			GetWindowRect(handle_, &r) ;
			return static_cast<Rect>(r) ; 
		}

		HWND GetHandle() const noexcept { return handle_ ; }

		bool IsWindowValid() const noexcept { return handle_ ; }
	} ;

	inline LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
		switch (msg) {
			case WM_SIZE : {
				auto it = Application::g_windows_.find(hwnd) ;
				if (it != Application::g_windows_.end()) {
					Application::g_windows_[hwnd]->CreateCanvas({LOWORD(lp), HIWORD(lp)}) ;
				} 
				EventSystem::PushEvent(Event::CreateResizeEvent(hwnd, {LOWORD(lp), HIWORD(lp)})) ;
				break ;
			}

			case WM_CLOSE : 
				EventSystem::PushEvent(Event::CreateNormalEvent(hwnd, EventType::Close)) ;
				return 0 ;

			case WM_DESTROY : 
				if (hwnd) {
					auto it = Application::g_windows_.find(hwnd) ;
					if (it != Application::g_windows_.end()) {
						Application::g_windows_.erase(it) ;

						#ifdef WINDOWPROCEDURE_DEBUG
							logger::info("erased window from g_windows_. current size : ", Application::g_windows_.size()) ;
						#endif

					} else {

						#ifdef WINDOWPROCEDURE_DEBUG
							logger::info("WM_DESTROY for hwnd not found in g_windows_. size: ", Application::g_windows_.size()) ;
						#endif

					}

					#ifdef WINDOWPROCEDURE_DEBUG
						logger::info("current g_windows size : ", Application::g_windows_.size()) ;
					#endif
					
				}

				if (Application::g_windows_.empty()) {
					Application::QuitProgram() ;
				}

				return 0 ;
		}

		return DefWindowProc(hwnd, msg, wp, lp) ;
	}

}