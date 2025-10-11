#pragma once
#include "unit.hpp"

namespace zketch {

	class Font {
		friend class Application ;
		
	private :
		std::wstring_view name_ ;
		float size_ = 0.0f ;
		uint8_t style_ = 0 ;

	public :
		Font() noexcept = default ;
		
		Font(const Font& o) noexcept : size_(o.size_), name_(o.name_), style_(o.style_) {}

		Font(Font&& o) noexcept : 
		name_(std::move(o.name_)), 
		size_(std::exchange(o.size_, 0.0f)) ,
		style_(std::exchange(o.style_, 0)) {}

		Font& operator=(const Font& o) noexcept {
			if (this != &o) {
				name_ = o.name_ ;
				size_ = o.size_ ;
				style_ = o.style_ ;
			}
			return *this ;
		}

		Font& operator=(Font&& o) noexcept {
			if (this != &o) {
				size_ = std::exchange(o.size_, 0.0f) ;
				name_ = std::move(o.name_) ;
				style_ = std::exchange(o.style_, 0) ;
			}
			return *this ;
		}

		Font(const std::wstring_view& fontname, float newSize, FontStyle style = FontStyle::Regular) {
			size_ = newSize ;
			name_ = fontname ;
			style_ = static_cast<uint8_t>(style) ;

			#ifdef FONT_DEBUG
				logger::winfo(L"Font::Font - Created font: ", name_, L" size: ", size_, L" style: ", static_cast<int>(style_)) ;
			#endif
		}

		void SetFontSize(float newSize) noexcept { size_ = newSize ; }
		void SetFontStyle(FontStyle style) noexcept { style_ = static_cast<uint8_t>(style) ; }

		float GetFontSize() const noexcept { return size_ ; }
		const std::wstring_view& GetFontName() const noexcept { return name_ ; }
		FontStyle GetFontStyle() const noexcept { return static_cast<FontStyle>(style_) ; }

		operator Gdiplus::Font() const noexcept { 
			return Gdiplus::Font(name_.data(), GetFontSize(), style_, Gdiplus::UnitPixel) ; 
		}
	} ;
}