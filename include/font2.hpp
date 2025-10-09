#pragma once
#include "fontdump.hpp"

namespace zketch {

	class Font {
		friend class Application ;
		
	private :
		static inline std::unordered_map<std::string, ___FONT_DATA___::__font_data__> g_fonts_ ;

		float scale_ = 0.0f ;
		std::string fontname_ ;
		uint8_t style_ = 0 ;

		std::string GetKey() const noexcept {
			std::string key ;
			key.reserve(fontname_.size() + 2) ;
			key.append(fontname_) ;
			key.push_back('|') ;
			key.push_back('0' + static_cast<uint8_t>(style_)) ;

			return key ;
		}

	public :
		Font() noexcept = default ;
		Font(const Font& o) noexcept {
			scale_ = o.scale_ ;
			fontname_ = o.fontname_ ;
			style_ = o.style_ ;
		}

		Font(Font&& o) noexcept : 
		scale_(std::exchange(o.scale_, 0.0f)), fontname_(std::move(o.fontname_)), 
		style_(std::exchange(o.style_, 0)) {}

		Font& operator=(const Font& o) noexcept {
			if (this != &o) {
				scale_ = o.scale_ ;
				fontname_ = o.fontname_ ;
				style_ = o.style_ ;
			}

			return *this ;
		}

		Font& operator=(Font&& o) noexcept {
			if (this != &o) {
				scale_ = std::exchange(o.scale_, 0.0f) ;
				fontname_ = std::move(o.fontname_) ;
				style_ = std::exchange(o.style_, style_) ;
			}

			return *this ;
		}

		Font(const std::string_view& fontname, float size, FontStyle style = FontStyle::Regular) {
			auto it = ___FONT_DUMP___::__font_dump__::___find_font___(g_fonts_, fontname, static_cast<uint8_t>(style)) ;

			if (it) {
				scale_ = size / it->height_ ;
				fontname_ = fontname ;
				style_ = static_cast<uint8_t>(style) ;
			} else {
				throw ___ERROR_HANDLER___::invalid_type() ;
			}
		}

		const std::string& GetFontName() const noexcept { return fontname_ ; }
		float GetFontSize() const noexcept { return g_fonts_[GetKey()].height_ * scale_ ; }
		FontStyle GetFontStyle() const noexcept { return static_cast<FontStyle>(style_) ; }

		operator Gdiplus::Font() const noexcept {
			return Gdiplus::Font(StringToWideString(fontname_).c_str(), GetFontSize(), style_, Gdiplus::UnitPixel) ;
		}
	} ;
}