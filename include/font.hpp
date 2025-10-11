<<<<<<< HEAD
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
=======
#pragma once
#include "fontdump.hpp"

namespace zketch {

	class Font {
		friend class Application ;
		
	private :
		static inline std::unordered_map<std::string, ___FONT_DATA___::__font_data__> g_fonts_ ;

		float scale_ = 0.0f ;
		std::string_view fontname_ ;
		uint8_t style_ = 0 ;

		std::string GetKey() const noexcept {
			std::string key ;
			key.reserve(fontname_.size() + 2) ;
			key.append(fontname_) ;
			key.push_back('|') ;
			key.push_back('0' + style_) ;
			return key ;
		}

	public :
		Font() noexcept = default ;
		
		Font(const Font& o) noexcept : scale_(o.scale_), fontname_(o.fontname_), style_(o.style_) {}

		Font(Font&& o) noexcept : 
		scale_(std::exchange(o.scale_, 0.0f)), 
		fontname_(std::move(o.fontname_)), 
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
				style_ = std::exchange(o.style_, 0) ;
			}
			return *this ;
		}

		Font(const std::string_view& fontname, float size, FontStyle style = FontStyle::Regular) {
			auto it = ___FONT_DUMP___::__font_dump__::___find_font___(g_fonts_, fontname, static_cast<uint8_t>(style)) ;

			if (!it || it->height_ <= 0.0f) {
				#ifdef FONT_DEBUG
					logger::error("Font::Font - Font not found or invalid: ", fontname, " style: ", static_cast<int>(style)) ;
				#endif
				throw ___ERROR_HANDLER___::invalid_type() ;
			}

			scale_ = size / it->height_ ;
			fontname_ = fontname ;
			style_ = static_cast<uint8_t>(style) ;

			#ifdef FONT_DEBUG
				logger::info("Font::Font - Created font: ", fontname, " size: ", size, " style: ", static_cast<int>(style)) ;
			#endif
		}

		void SetFontSize(float newSize) noexcept {
			auto it = g_fonts_.find(GetKey()) ;
			scale_ = newSize / it->second.height_ ;
		}
		
		float GetFontSize() const noexcept { 
			auto it = g_fonts_.find(GetKey()) ;
			return (it != g_fonts_.end()) ? it->second.height_ * scale_ : 0.0f ;
		}

		float GetCharWidthOf(char c) const noexcept {
			auto it = g_fonts_.find(GetKey()) ;
			if (it == g_fonts_.end()) return 0.0f ;
			
			int index = static_cast<int>(c) - ___FONT_DATA___::___CHAR_START___ ;
			if (index < 0 || index >= ___FONT_DATA___::___CHAR_COUNT___) return 0.0f ;
			
			return it->second.char_widths_[index] * scale_ ;
		}

		float GetStringWidth(const std::string_view& str) const noexcept {
			if (str.empty()) return 0.0f ;
			
			float total_width = 0.0f ;
			for (char c : str) {
				total_width += GetCharWidthOf(c) ;
			}
			return total_width ;
		}

		float GetStringWidth(const std::wstring_view& wstr) const noexcept {
			if (wstr.empty()) return 0.0f ;
			return GetStringWidth(WideStringToString(std::wstring(wstr))) ;
		}

		float GetHeight() const noexcept {
			auto it = g_fonts_.find(GetKey()) ;
			return (it != g_fonts_.end()) ? it->second.height_ * scale_ : 0.0f ;
		}

		float GetAscent() const noexcept {
			auto it = g_fonts_.find(GetKey()) ;
			return (it != g_fonts_.end()) ? it->second.ascent_ * scale_ : 0.0f ;
		}

		float GetDescent() const noexcept {
			auto it = g_fonts_.find(GetKey()) ;
			return (it != g_fonts_.end()) ? it->second.descent_ * scale_ : 0.0f ;
		}

		float GetWeight() const noexcept {
			auto it = g_fonts_.find(GetKey()) ;
			return (it != g_fonts_.end()) ? it->second.weight_ : 0.0f ;
		}

		const std::string_view& GetFontName() const noexcept { return fontname_ ; }
		FontStyle GetFontStyle() const noexcept { return static_cast<FontStyle>(style_) ; }
		RectF GetTextBound(const std::string_view& text, const PointF& origin = {}) const noexcept { return {origin.x, origin.y - GetAscent(), GetHeight(), GetStringWidth(text)} ; }
		RectF GetTextBound(const std::wstring& text, const PointF& origin = {}) const noexcept { return {origin.x, origin.y - GetAscent(), GetHeight(), GetStringWidth(text)} ; }

		operator Gdiplus::Font() const noexcept { return Gdiplus::Font(StringToWideString(fontname_).c_str(), GetFontSize(), style_, Gdiplus::UnitPixel) ; }
	} ;
>>>>>>> 015feefd91700e70c7ee430b9505f3ab1ca1e0a6
}