#pragma once
#include "widget.hpp"

namespace zketch {
	class Button : public Widget<Button> {
		friend class Widget<Button> ;

    private:
        bool is_hovered_ = false ;
        bool is_pressed_ = false ;
		std::wstring label_ ;
        std::unique_ptr<Font> font_ ;
        std::function<void(Canvas*, const Button&)> drawing_logic_ ;
        std::function<void()> on_click_ ;

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
        Button(const RectF& bound, const Font& font, const std::wstring& label = L"") noexcept : label_(label) {
            bound_ = bound ;
            canvas_ = std::make_unique<Canvas>() ;
            canvas_->Create(bound_.GetSize()) ;
			font_ = std::make_unique<Font>() ;
			*font_ = font ;

            SetDrawingLogic([&](Canvas* canvas, const Button& button) {
                Renderer render ;
                if (!render.Begin(*canvas)) {
					return ;
				}

				render.Clear(Transparent) ;
                Color button_color ;
                Color border_color ;
                
                if (button.IsPressed()) {
                    button_color = rgba(70, 130, 180, 255) ;
                    border_color = rgba(50, 100, 150, 255) ;
                } else if (button.IsHovered()) {
                    button_color = rgba(100, 149, 237, 255) ;
                    border_color = rgba(70, 119, 207, 255) ;
                } else {
                    button_color = rgba(135, 206, 250, 255) ;
                    border_color = rgba(100, 171, 220, 255) ;
                }
                
                RectF rect = button.GetRelativeBound() ;
                render.FillRectRounded(rect, button_color, 5.0f) ;
                render.DrawRectRounded(rect, border_color, 5.0f, 2.0f) ;
                
                if (!button.GetLabel().empty()) {
                    Color text_color = rgba(255, 255, 255, 255) ;
                    Point text_pos = {static_cast<int32_t>(rect.w / 2 - 30),static_cast<int32_t>(rect.h / 2 - 10)} ;
                    render.DrawString(label, text_pos, text_color, *button.GetFont()) ;
                }

                render.End() ;
            }) ;
        }

        bool OnHover(const PointF& mouse_pos) noexcept {
            bool state = bound_.Contain(mouse_pos) ;
            if (state != is_hovered_) {
                is_hovered_ = state ;
                update_ = true ;
            }
            return state ;
        }

        bool OnPress(const PointF& mouse_pos) noexcept {
            if (bound_.Contain(mouse_pos)) {
                is_pressed_ = true ;
                update_ = true ;
                return true ;
            }
            return false ;
        }

        bool OnRelease(const PointF& mouse_pos) noexcept {
            bool was_pressed = is_pressed_ ;
            is_pressed_ = false ;
            if (was_pressed) {
                update_ = true ;
                if (bound_.Contain(mouse_pos) && on_click_) {
                    on_click_() ;
                }
                return bound_.Contain(mouse_pos) ;
            }
            return false ;
        }

        void SetDrawingLogic(std::function<void(Canvas*, const Button&)> drawing_logic) noexcept {
            drawing_logic_ = std::move(drawing_logic) ;
            update_ = true ;
        }
        
        void SetOnClick(std::function<void()> callback) noexcept {
            on_click_ = std::move(callback) ;
        }
        
        void SetLabel(const std::wstring& label) noexcept {
            if (label_ != label) {
                label_ = label ;
                update_ = true ;
            }
        }

        RectF GetRelativeBound() const noexcept { return {0, 0, bound_.w, bound_.h} ; }
        const std::wstring& GetLabel() const noexcept { return label_ ; }
        Font* GetFont() const noexcept { return font_.get() ; }

        bool IsHovered() const noexcept { return is_hovered_ ; }
        bool IsPressed() const noexcept { return is_pressed_ ; }
    } ;
}