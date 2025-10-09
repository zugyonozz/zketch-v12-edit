#pragma once
#include "widget.hpp"

namespace zketch {
	
	class TextInput : Widget<TextInput> {
		friend class Widget<TextInput> ;

	private :
		uint8_t cursor_interval_ ;
		size_t cursor_index_ ;
		PointF origin_ ;
		std::wstring text_ ;
		Font font_ ;
		std::unique_ptr<Canvas> text_canvas_ ;
		std::unique_ptr<Canvas> cursor_canvas_ ;
		std::function<void(Canvas*, const TextInput&)> drawing_logic_ ;

	public :
		TextInput(const RectF& bound, const Font& font, uint8_t cursor_interval = 3) noexcept {
			canvas_ = std::make_unique<Canvas>() ;
			text_canvas_ = std::make_unique<Canvas>() ;
			cursor_canvas_ = std::make_unique<Canvas>() ;

			bound_ = bound ;
			cursor_interval_ = cursor_interval ;
			cursor_index_ = 0 ;
			font_ = font ;
			origin_ = {0, bound_.y + (font_.GetHeight() / 2)} ;
			canvas_->Create(bound_.GetSize()) ;
			text_canvas_->Create(bound.GetSize()) ;
			cursor_canvas_->Create({font_.GetHeight(), 2}) ;

			SetDrawingLogic([](Canvas* canvas, const TextInput& textinput){
				Renderer render ;

				if (!render.Begin(*canvas)) {
					return ;
				}

				render.Clear(Transparent) ;

				render.FillRectRounded(
                    textinput.bound_, 
                    rgba(200, 200, 200, 1),
                    3.0f
                ) ;
                
                render.DrawString(
                    textinput.text_, 
                    {0, 0}, 
                    rgba(60, 60, 60, 1), 
                    textinput.font_
                ) ;

                render.End() ;
			}) ;
		}

		// menggunkan uint8_t karena sesuai konteks, cursor kedip tidak mungkin lama2
		// tapi butuh fix agar sesuai dengan waktu
		void SetCursorInterval(uint8_t ms) noexcept { cursor_interval_ = ms ; }
		void SetCursorIndex(size_t index) noexcept { cursor_index_ = index ; }
		void SetOrigin(const PointF& origin) noexcept { origin_ = origin ; }
		void SetNextIndex() noexcept { ++cursor_index_ ; }
		void SetPrevIndex() noexcept { ++cursor_index_ ; }
		void Insert(wchar_t c) noexcept { text_.insert(text_.begin() + cursor_index_, c) ; }
		void Pop() noexcept { text_.erase(cursor_index_, 1) ; }

		void SetDrawingLogic(std::function<void(Canvas*, const TextInput&)> drawing_logic) noexcept {
			drawing_logic_ = std::move(drawing_logic) ;
			update_ = true ;
		}

		const Font& GetFont() const noexcept { return font_ ; }
		const uint8_t GetCursorIndex() const noexcept { return cursor_index_ ; }
		const PointF& GetOrigin() const noexcept { return origin_ ; }
		const std::wstring& GetText() const noexcept { return text_ ; }
	} ;
}