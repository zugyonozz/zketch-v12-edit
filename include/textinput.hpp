#pragma once
#include "widget.hpp"

namespace zketch {
	
	class TextInput : Widget<TextInput> {
		friend class Widget<TextInput> ;

	private :
		uint8_t cursor_interval_ ;
		RectF text_region_ ;
		PointF cursor_region_ ;
		std::wstring served_ ;
		std::wstring data_ ;
		std::unique_ptr<Font> font_ ;
		std::unique_ptr<Canvas> text_canvas_ ;
		std::unique_ptr<Canvas> cursor_canvas_ ;

	public :
		TextInput(const RectF& bound, const Font& font, uint8_t cursor_interval = 3) noexcept : 
		font_(std::make_unique<Font>()), cursor_interval_(cursor_interval) {
			text_canvas_ = std::make_unique<Canvas>() ;
			cursor_canvas_ = std::make_unique<Canvas>() ;
			bound_ = bound ;
			text_region_ = {bound_.GetPos() + 5, bound_.GetSize() - 5} ;
			cursor_region_ = text_region_.GetPos() + PointF{2.0f, 0.0f} ;
			
		}
	} ;
}