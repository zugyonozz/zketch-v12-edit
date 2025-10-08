#pragma once 
#include "widget.hpp"

namespace zketch {
	class TextBox : public Widget<TextBox> {
		friend class Widget<TextBox> ;
		
    private:
        std::wstring text_ ;
        Font font_ ;
        Color text_color_ = rgba(50, 50, 50, 255) ;
        Color bg_color_ = rgba(250, 250, 250, 255) ;
        std::function<void(Canvas*, const TextBox&)> drawing_logic_ ;

		void UpdateImpl() noexcept {
            if (!drawing_logic_) {
				return ;
			}
			
            if (!IsValid()) {
				return ;
			}

            drawing_logic_(canvas_.get(), *this) ;
        }

    public:
        TextBox(const RectF& bound, const std::wstring& text, const Font& font) noexcept : text_(text), font_(font) {
            bound_ = bound ;
            canvas_ = std::make_unique<Canvas>() ;
            canvas_->Create(bound_.GetSize()) ;
            
            SetDrawingLogic([](Canvas* canvas, const TextBox& textbox){
                Renderer render ;
                if (!render.Begin(*canvas)) return ;
				render.Clear(Transparent) ;
                render.DrawString(
                    textbox.GetText(), 
                    {5, 5}, 
                    textbox.GetTextColor(), 
                    textbox.GetFont()
                ) ;

                render.End() ;
            }) ;
        }

        void SetText(const std::wstring& text) noexcept {
            if (text_ != text) {
                text_ = text ;
                update_ = true ;
            }
        }
        
        void SetTextColor(const Color& color) noexcept {
            text_color_ = color ;
            update_ = true ;
        }
        
        void SetDrawingLogic(std::function<void(Canvas*, const TextBox&)> drawing_logic) noexcept {
            drawing_logic_ = std::move(drawing_logic) ;
            update_ = true ;
        }

        const std::wstring& GetText() const noexcept { return text_ ; }
        const Font& GetFont() const noexcept { return font_ ; }
        const Color& GetTextColor() const noexcept { return text_color_ ; }
        const Color& GetBackgroundColor() const noexcept { return bg_color_ ; }
    } ;
}