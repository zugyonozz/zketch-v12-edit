#pragma once
#include "unit.hpp"

namespace zketch {

	class Font ;
	bool ValidateFont(const std::string_view& fontname) noexcept ;
	inline void GetCharacterWidths(HDC, Font&) noexcept ;
	std::optional<Font> TryCreateFont(HDC, const std::wstring&, int, bool) noexcept ;
	
	struct DumpOptions ;
	std::optional<std::pair<size_t, size_t>> DumpInstalledFonts(const DumpOptions&) noexcept ;

	// Font class untuk runtime usage (dengan std::string untuk convenience)
	#pragma pack(push, 1)
	class Font {
		friend inline void GetCharacterWidths(HDC, Font&) noexcept ;
		friend std::optional<Font> TryCreateFont(HDC hdc, const std::wstring& fontname, int reqWeight, bool reqItalic) noexcept ;
		friend std::optional<std::pair<size_t, size_t>> DumpInstalledFonts(const DumpOptions&) noexcept ;

	private:
		// Compact character width storage (ASCII 32-126 = 95 chars)
		static constexpr uint8_t CHAR_START_ = 32 ;   // Space
		static constexpr uint8_t CHAR_END_ = 126 ;    // Tilde ~
		static constexpr uint8_t CHAR_COUNT_ = CHAR_END_ - CHAR_START_ + 1 ;  // 95 chars

		std::string fontname_ {} ;
		FontStyle style_ = FontStyle::Regular ;
		float height_ = 0.0f ;
		float weight_ = 0.0f ;
		float ascent_ = 0.0f ;
		float descent_ = 0.0f ;
		std::array<float, CHAR_COUNT_> char_widths_ {} ;
		bool is_valid_ = false ;

	public:
		Font() noexcept = default ;
		Font(const Font& o) noexcept = default ;
		Font& operator=(const Font& o) noexcept = default ;

		Font(const char* fontname, float size, FontStyle style) noexcept : 
			fontname_(fontname), height_(size), style_(style) {}

		Font(Font&& o) noexcept : 
			fontname_(std::move(o.fontname_)), 
			style_(std::exchange(o.style_, FontStyle::Regular)), 
			height_(std::exchange(o.height_, 0.0f)), 
			weight_(std::exchange(o.weight_, 0.0f)), 
			ascent_(std::exchange(o.ascent_, 0.0f)), 
			descent_(std::exchange(o.descent_, 0.0f)), 
			char_widths_(std::move(o.char_widths_)), 
			is_valid_(std::exchange(o.is_valid_, false)) {}

		Font& operator=(Font&& o) noexcept {
			if (this != &o) {
				fontname_ = std::move(o.fontname_) ;
				style_ = std::exchange(o.style_, FontStyle::Regular) ;  // FIX: was declaring local variable
				height_ = std::exchange(o.height_, 0.0f) ;
				weight_ = std::exchange(o.weight_, 0.0f) ;
				ascent_ = std::exchange(o.ascent_, 0.0f) ;
				descent_ = std::exchange(o.descent_, 0.0f) ;
				char_widths_ = std::move(o.char_widths_) ;
				is_valid_ = std::exchange(o.is_valid_, false) ;
			}
			return *this ;
		}

		operator Gdiplus::Font() const noexcept {
			std::wstring cvtres(fontname_.begin(), fontname_.end()) ;
			return Gdiplus::Font(cvtres.c_str(), height_, static_cast<int>(style_), Gdiplus::UnitPixel) ;
		}

		bool IsValid() const noexcept {
			return is_valid_ ;
		}

		void SetFontName(const std::string_view& fontname) noexcept {
			fontname_ = fontname ;
		}

		void SetSizeFont(float fontsize) noexcept {
			if (height_ <= 0.0f || fontsize <= 0.0f) {
				return ;
			}
			
			const float scale = fontsize / height_ ;
			ascent_ *= scale ;
			descent_ *= scale ;
			
			for (auto& w : char_widths_) {
				w *= scale ;
			}
			
			height_ = fontsize ;
		}

		[[nodiscard]] inline std::optional<float> GetCharWidthOf(char c) const noexcept {
			if (c < CHAR_START_ || c > CHAR_END_) {
				return std::nullopt ;
			}
			return char_widths_[c - CHAR_START_] ;
		}

		[[nodiscard]] std::optional<float> GetTextWidth(const std::string_view& text) const noexcept {
			float res = 0.0f ;
			for (const char c : text) {
				const auto w = GetCharWidthOf(c) ;
				if (!w) {
					return std::nullopt ;
				}
				res += *w ;
			}
			return res ;
		}

		[[nodiscard]] float GetTextWidth(const std::string_view& text, float fallbackWidth) const noexcept {
			float res = 0.0f ;
			for (const char c : text) {
				const auto w = GetCharWidthOf(c) ;
				res += w.value_or(fallbackWidth) ;
			}
			return res ;
		}

		[[nodiscard]] std::optional<float> GetTextWidth(const std::string& text) const noexcept {
			return GetTextWidth(static_cast<std::string_view>(text)) ;
		}

		[[nodiscard]] float GetTextWidth(const std::string& text, float fallbackWidth) const noexcept {
			return GetTextWidth(static_cast<std::string_view>(text), fallbackWidth) ;
		}

		[[nodiscard]] std::optional<RectF> GetTextBound(const std::string_view& text, const PointF& origin) const noexcept {
			const auto w = GetTextWidth(text) ;
			if (!w) {
				return std::nullopt ;
			}
			
			RectF res {} ;
			res.x = origin.x ; 
			res.y = origin.y - ascent_ ; 
			res.h = height_ ; 
			res.w = *w ;
			return res ;
		}

		[[nodiscard]] RectF GetTextBound(const std::string_view& text, const PointF& origin, float fallbackWidth) const noexcept {
			RectF res {} ;
			res.x = origin.x ; 
			res.y = origin.y - ascent_ ; 
			res.h = height_ ; 
			res.w = GetTextWidth(text, fallbackWidth) ;
			return res ;
		}

		[[nodiscard]] std::optional<RectF> GetTextBound(const std::string& text, const PointF& origin) const noexcept {
			return GetTextBound(static_cast<std::string_view>(text), origin) ;
		}

		[[nodiscard]] RectF GetTextBound(const std::string& text, const PointF& origin, float fallbackWidth) const noexcept {
			return GetTextBound(static_cast<std::string_view>(text), origin, fallbackWidth) ;
		}

		[[nodiscard]] const std::string& GetFontName() const noexcept { return fontname_ ; }
		[[nodiscard]] float GetFontSize() const noexcept { return height_ ; }
		[[nodiscard]] FontStyle GetFontStyle() const noexcept { return style_ ; }
	} ;
	#pragma pack(pop)

	// WARNING: Binary serialization of Font class is NOT SAFE due to std::string internal pointers
	// For safe binary serialization, use a separate fixed-size struct (see font-ref.hpp FontEntry)
	// This binary format is for quick save/load in same process session only
	#pragma pack(push, 1)
	struct FontBinaryHeader {
		uint32_t magic ;
		uint16_t version ;
		uint16_t entrySize ;
		uint32_t count ;
		uint64_t timestamp ;
		uint64_t reserved1 ;
		uint64_t reserved2 ;
		
		FontBinaryHeader() noexcept : 
			magic(0x534E5446), 
			version(3), 
			entrySize(sizeof(Font)), 
			count(0), 
			timestamp(0), 
			reserved1(0), 
			reserved2(0) {}
		
		[[nodiscard]] bool IsValid() const noexcept {
			return magic == 0x534E5446 && version == 3 && entrySize == sizeof(Font) ;
		}
	} ;
	#pragma pack(pop)

	static_assert(sizeof(FontBinaryHeader) == 36, "Header must be 36 bytes") ;

	struct DumpOptions {
		std::string out_path_csv = "installed_fonts.csv" ;
		std::string out_path_bin = "installed_fonts.bin" ;
		bool write_csv = true ;
		bool write_binary = true ;
		bool write_bom = true ;
		bool write_header = true ;
		size_t reserve_bytes = 1 << 20 ;
		uint32_t charset = DEFAULT_CHARSET ;
	} ;

	inline void Trim(std::string& s) noexcept {
		// Trim left
		auto it = s.begin() ;
		while (it != s.end() && std::isspace(static_cast<unsigned char>(*it))) {
			++it ;
		}
		s.erase(s.begin(), it) ;
		
		// Trim right
		auto rit = s.rbegin() ;
		while (rit != s.rend() && std::isspace(static_cast<unsigned char>(*rit))) {
			++rit ;
		}
		s.erase(rit.base(), s.end()) ;
	}

	inline void AppendCSVField(std::string& out, const std::string_view s) {
		const char* p = s.data() ;
		const char* end = p + s.size() ;
		bool needQuote = false ;
		
		// Check if quoting needed
		for ( ; p != end ; ++p) {
			const char c = *p ;
			if (c == '"' || c == ',' || c == '\n' || c == '\r') {
				needQuote = true ;
				break ;
			}
		}
		
		if (!needQuote) {
			out.append(s.data(), s.size()) ;
			return ;
		}
		
		// Escape and quote
		out.push_back('"') ;
		for (const char c : s) {
			if (c == '"') {
				out.append("\"\"") ;
			} else {
				out.push_back(c) ;
			}
		}
		out.push_back('"') ;
	}

	struct CollectContext {
		std::set<std::wstring> families ;
	} ;

	int CALLBACK CollectFontFamiliesProc(const LOGFONTW* lpelfe, const TEXTMETRICW*, DWORD, LPARAM lParam) {
		if (!lpelfe || !lParam) return 1 ;
		auto* ctx = reinterpret_cast<CollectContext*>(lParam) ;
		ctx->families.insert(lpelfe->lfFaceName) ;
		return 1 ;
	}

	inline void GetCharacterWidths(HDC hdc, Font& entry) noexcept {
		INT widths[Font::CHAR_COUNT_] ;
		
		if (GetCharWidth32W(hdc, Font::CHAR_START_, Font::CHAR_END_, widths)) {
			for (int i = 0 ; i < Font::CHAR_COUNT_ ; ++i) {
				entry.char_widths_[i] = static_cast<float>(widths[i]) ;
			}
		} else {
			// Fallback to zero (indicates error)
			for (int i = 0 ; i < Font::CHAR_COUNT_ ; ++i) {
				entry.char_widths_[i] = 0.0f ;
			}
		}
	}

	[[nodiscard]] std::optional<Font> TryCreateFont(HDC hdc, const std::wstring& fontname, int reqWeight, bool reqItalic) noexcept {
		LOGFONTW lf {} ;
		wcscpy_s(lf.lfFaceName, fontname.c_str()) ;
		lf.lfHeight = -16 ;
		lf.lfWeight = reqWeight ;
		lf.lfItalic = reqItalic ? TRUE : FALSE ;
		lf.lfCharSet = DEFAULT_CHARSET ;
		lf.lfQuality = CLEARTYPE_QUALITY ;
		
		HFONT hFont = CreateFontIndirectW(&lf) ;
		if (!hFont) {
			return std::nullopt ;
		}
		
		HGDIOBJ oldFont = SelectObject(hdc, hFont) ;
		
		wchar_t actualName[256] ;
		const int nameLen = GetTextFaceW(hdc, 256, actualName) ;
		if (nameLen <= 0) {
			SelectObject(hdc, oldFont) ;
			DeleteObject(hFont) ;
			return std::nullopt ;
		}
		
		TEXTMETRICW tm {} ;
		if (!GetTextMetricsW(hdc, &tm)) {
			SelectObject(hdc, oldFont) ;
			DeleteObject(hFont) ;
			return std::nullopt ;
		}
		
		char nameBuffer[512] ;
		const int utf8Len = WideCharToMultiByte(
			CP_UTF8, 0, actualName, nameLen, 
			nameBuffer, sizeof(nameBuffer), nullptr, nullptr) ;
		
		if (utf8Len <= 0) {
			SelectObject(hdc, oldFont) ;
			DeleteObject(hFont) ;
			return std::nullopt ;
		}
		
		std::string name(nameBuffer, utf8Len) ;
		Trim(name) ;
		
		Font entry {} ;
		entry.SetFontName(name) ;
		entry.height_ = static_cast<float>(tm.tmHeight) ;
		entry.ascent_ = static_cast<float>(tm.tmAscent) ;
		entry.descent_ = static_cast<float>(tm.tmDescent) ;
		entry.weight_ = static_cast<float>(tm.tmWeight) ;
		
		const bool isBold = (tm.tmWeight >= FW_BOLD) ;
		const bool isItalic = (tm.tmItalic != 0) ;
		entry.style_ = static_cast<FontStyle>((isBold ? 1 : 0) | (isItalic ? 2 : 0)) ;
		entry.is_valid_ = true ;
		
		GetCharacterWidths(hdc, entry) ;
		
		SelectObject(hdc, oldFont) ;
		DeleteObject(hFont) ;
		
		return entry ;
	}

	[[nodiscard]] inline std::optional<std::pair<size_t, size_t>> DumpInstalledFonts(const DumpOptions& opt) noexcept {
		HDC screen = GetDC(nullptr) ;
		if (!screen) {
			return std::nullopt ;
		}
		
		HDC dc = CreateCompatibleDC(screen) ;
		ReleaseDC(nullptr, screen) ;
		if (!dc) {
			return std::nullopt ;
		}

		#ifdef FONT_DEBUG
			logger::info("DumpInstalledFonts - Collecting font families...") ;
		#endif

		CollectContext collectCtx ;
		LOGFONTW lf {} ;
		lf.lfCharSet = static_cast<BYTE>(opt.charset) ;
		EnumFontFamiliesExW(dc, &lf, CollectFontFamiliesProc, reinterpret_cast<LPARAM>(&collectCtx), 0) ;
		
		const size_t familyCount = collectCtx.families.size() ;

		#ifdef FONT_DEBUG
			logger::info("Found font families", familyCount) ;
		#endif

		std::vector<Font> entries ;
		entries.reserve(familyCount * 4) ;
		
		std::string csvOutput ;
		if (opt.write_csv) {
			csvOutput.reserve(opt.reserve_bytes) ;
			if (opt.write_bom) {
				csvOutput.append("\xEF\xBB\xBF") ;
			}
			if (opt.write_header) {
				csvOutput.append("Name,Height,Ascent,Descent,Weight,Style") ;
				for (int i = Font::CHAR_START_ ; i <= Font::CHAR_END_ ; ++i) {
					csvOutput.push_back(',') ;
					csvOutput.append(std::to_string(i)) ;
				}
				csvOutput.push_back('\n') ;
			}
		}
		
		std::unordered_set<std::string> seen ;
		seen.reserve(familyCount * 4) ;
		
		constexpr std::pair<int, bool> styles[4] = {
			{FW_NORMAL, false},
			{FW_BOLD, false},
			{FW_NORMAL, true},
			{FW_BOLD, true}
		} ;
		
		for (const auto& family : collectCtx.families) {
			for (const auto& [weight, italic] : styles) {
				auto entryOpt = TryCreateFont(dc, family, weight, italic) ;
				if (!entryOpt) continue ;
				
				Font entry = *entryOpt ;
				
				// Create unique key
				std::string key ;
				key.reserve(entry.GetFontName().size() + 2) ;
				key.append(entry.GetFontName()) ;
				key.push_back('|') ;
				key.push_back('0' + static_cast<char>(entry.style_)) ;
				
				if (!seen.insert(key).second) continue ;
				
				entries.push_back(entry) ;
				
				if (opt.write_csv) {
					AppendCSVField(csvOutput, entry.GetFontName()) ;
					csvOutput.push_back(',') ;
					csvOutput.append(std::to_string(entry.height_)) ;
					csvOutput.push_back(',') ;
					csvOutput.append(std::to_string(entry.ascent_)) ;
					csvOutput.push_back(',') ;
					csvOutput.append(std::to_string(entry.descent_)) ;
					csvOutput.push_back(',') ;
					csvOutput.append(std::to_string(entry.weight_)) ;
					csvOutput.push_back(',') ;
					csvOutput.append(std::to_string(static_cast<int>(entry.style_))) ;
					
					// FIX: Proper iteration over char widths
					for (size_t i = 0 ; i < entry.char_widths_.size() ; ++i) {
						csvOutput.push_back(',') ;
						csvOutput.append(std::to_string(entry.char_widths_[i])) ;
					}
					csvOutput.push_back('\n') ;
				}
			}
		}

		DeleteDC(dc) ;

		size_t csvBytes = 0 ;
		if (opt.write_csv && !csvOutput.empty()) {
			std::ofstream csvFile(opt.out_path_csv, std::ios::binary) ;
			if (csvFile) {
				csvFile.write(csvOutput.data(), static_cast<std::streamsize>(csvOutput.size())) ;
				csvBytes = csvOutput.size() ;

				#ifdef FONT_DEBUG
					logger::info("CSV saved", opt.out_path_csv) ;
				#endif
			}
		}

		size_t binBytes = 0 ;
		if (opt.write_binary && !entries.empty()) {
			// WARNING: This writes std::string internal state which contains pointers
			// Only use this for same-session save/load, not for persistent storage
			std::ofstream binFile(opt.out_path_bin, std::ios::binary) ;
			if (binFile) {
				FontBinaryHeader header ;
				header.count = static_cast<uint32_t>(entries.size()) ;
				header.timestamp = static_cast<uint64_t>(time(nullptr)) ;
				
				binFile.write(reinterpret_cast<const char*>(&header), sizeof(header)) ;
				binFile.write(reinterpret_cast<const char*>(entries.data()), 
					static_cast<std::streamsize>(entries.size() * sizeof(Font))) ;
				
				binBytes = sizeof(header) + entries.size() * sizeof(Font) ;

				#ifdef FONT_DEBUG
					logger::info("Binary saved", opt.out_path_bin) ;
					logger::info("Entry size", sizeof(Font)) ;
				#endif
			}
		}

		return std::make_pair(entries.size(), std::max(csvBytes, binBytes)) ;
	}

	// WARNING: LoadFontsFromBinary is NOT SAFE for persistent storage
	// Only use for same-session save/load due to std::string pointer issues
	[[nodiscard]] inline std::optional<std::unordered_map<std::string, Font>> LoadFontsFromBinary(const std::string& filename) noexcept {
		std::ifstream file(filename, std::ios::binary) ;
		if (!file) {
			return std::nullopt ;
		}
		
		file.seekg(0, std::ios::end) ;
		const auto fileSize = file.tellg() ;
		file.seekg(0, std::ios::beg) ;
		
		if (fileSize < static_cast<std::streamoff>(sizeof(FontBinaryHeader))) {
			return std::nullopt ;
		}
		
		FontBinaryHeader header ;
		file.read(reinterpret_cast<char*>(&header), sizeof(header)) ;
		
		if (!file || !header.IsValid()) {
			return std::nullopt ;
		}
		
		const size_t expectedSize = sizeof(FontBinaryHeader) + header.count * sizeof(Font) ;
		if (static_cast<size_t>(fileSize) < expectedSize) {
			return std::nullopt ;
		}
		
		std::unordered_map<std::string, Font> fontMap ;
		fontMap.reserve(header.count) ;
		
		std::vector<Font> entries(header.count) ;
		file.read(reinterpret_cast<char*>(entries.data()), 
			static_cast<std::streamsize>(header.count * sizeof(Font))) ;
		
		if (!file) {
			return std::nullopt ;
		}
		
		for (auto& entry : entries) {
			std::string key = entry.GetFontName() ;
			key.push_back('|') ;
			key.push_back('0' + static_cast<char>(entry.GetFontStyle())) ;
			fontMap.emplace(std::move(key), std::move(entry)) ;
		}

		return fontMap ;
	}

	[[nodiscard]] inline const Font* FindFont(
		const std::unordered_map<std::string, Font>& fontMap, 
		const std::string_view name, 
		FontStyle style = FontStyle::Regular) noexcept 
	{
		std::string key ;
		key.reserve(name.size() + 2) ;
		key.append(name) ;
		key.push_back('|') ;
		key.push_back('0' + static_cast<char>(style)) ;
		
		const auto it = fontMap.find(key) ;
		return (it != fontMap.end()) ? &it->second : nullptr ;
	}

	inline bool CreateDumpFonts(
		const char* filenameWithoutExt, 
		uint32_t reserve_bytes = 1 << 20, 
		bool write_csv = false, 
		bool write_binary = true) noexcept 
	{
		DumpOptions opt ;
		opt.out_path_csv = std::string(filenameWithoutExt) + ".csv" ;
		opt.out_path_bin = std::string(filenameWithoutExt) + ".bin" ;
		opt.write_csv = write_csv ;
		opt.write_binary = write_binary ;
		opt.reserve_bytes = reserve_bytes ;

		const auto res = DumpInstalledFonts(opt) ;
		
		return res.has_value() ;
	}

} // namespace zketch