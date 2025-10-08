#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <memory>

#ifdef FONTDUMP_DEBUG
    #define FONTDUMP_LOG(msg) std::cout << msg << "\n"
    #define FONTDUMP_LOG_VALUE(label, value) std::cout << label << ": " << value << "\n"
#else
    #define FONTDUMP_LOG(msg) ((void)0)
    #define FONTDUMP_LOG_VALUE(label, value) ((void)0)
#endif

namespace fontdump {

// Font style enum
enum class FontStyle : uint8_t {
    Regular = 0,
    Bold = 1,
    Italic = 2,
    BoldItalic = 3
} ;

// Compact character width storage (ASCII 32-126 = 95 chars)
constexpr int CHAR_START = 32 ;   // Space
constexpr int CHAR_END = 126 ;    // Tilde ~
constexpr int CHAR_COUNT = CHAR_END - CHAR_START + 1 ;  // 95 chars

#pragma pack(push, 1)
struct FontEntry {
    char name[192] ;
    int16_t height ;
    int16_t ascent ;
    int16_t descent ;
    int16_t avgCharWidth ;
    int16_t maxCharWidth ;
    int16_t weight ;
    FontStyle style ;
    uint8_t padding ;
    
    // Character widths for ASCII 32-126 (95 printable chars)
    int16_t charWidths[CHAR_COUNT] ;
    
    inline void SetName(std::string_view str) noexcept {
        const size_t len = (str.size() < sizeof(name)) ? str.size() : sizeof(name) - 1 ;
        std::memcpy(name, str.data(), len) ;
        name[len] = '\0' ;
    }
    
    [[nodiscard]] inline std::string_view GetName() const noexcept {
        return std::string_view(name, std::strlen(name)) ;
    }
    
    [[nodiscard]] inline std::string GetNameStr() const {
        return std::string(name, std::strlen(name)) ;
    }
    
    // Get width of specific character
    [[nodiscard]] inline int16_t GetCharWidth(char c) const noexcept {
        if (c < CHAR_START || c > CHAR_END) {
            return avgCharWidth ;  // Fallback for non-printable
        }
        return charWidths[c - CHAR_START] ;
    }
    
    // Calculate text width (accurate for ASCII text)
    [[nodiscard]] inline int CalculateTextWidth(std::string_view text) const noexcept {
        int width = 0 ;
        for (char c : text) {
            width += GetCharWidth(c) ;
        }
        return width ;
    }
    
    // Calculate text width (for wide string)
    [[nodiscard]] inline int CalculateTextWidth(std::wstring_view text) const noexcept {
        int width = 0 ;
        for (wchar_t wc : text) {
            if (wc < CHAR_START || wc > CHAR_END) {
                width += avgCharWidth ;  // Fallback
            } else {
                width += charWidths[wc - CHAR_START] ;
            }
        }
        return width ;
    }
} ;
#pragma pack(pop)

#pragma pack(push, 1)
struct FontBinaryHeader {
    uint32_t magic ;
    uint16_t version ;
    uint16_t entrySize ;
    uint32_t count ;
    uint64_t timestamp ;
    uint64_t reserved1 ;
    uint64_t reserved2 ;
    
    FontBinaryHeader() noexcept : magic(0x534E5446), version(3), entrySize(sizeof(FontEntry)), count(0), timestamp(0), reserved1(0), reserved2(0) {}
    
    [[nodiscard]] inline bool IsValid() const noexcept {
        return magic == 0x534E5446 && 
               version == 3 && 
               entrySize == sizeof(FontEntry) ;
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
    bool include_char_widths = true ;  // New option
    size_t reserve_bytes = 1 << 20 ;
    UINT charset = DEFAULT_CHARSET ;
} ;

static inline void Trim(std::string& s) noexcept {
    auto it = s.begin() ;
    while (it != s.end() && std::isspace(static_cast<unsigned char>(*it))) ++it ;
    s.erase(s.begin(), it) ;
    
    auto rit = s.rbegin() ;
    while (rit != s.rend() && std::isspace(static_cast<unsigned char>(*rit))) ++rit ;
    s.erase(rit.base(), s.end()) ;
}

static void AppendCSVField(std::string& out, std::string_view s) {
    const char* p = s.data() ;
    const char* end = p + s.size() ;
    bool needQuote = false ;
    
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
    
    out.push_back('"') ;
    for (char c : s) {
        if (c == '"') out.append("\"\"") ;
        else out.push_back(c) ;
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

// Get character widths for ASCII range
inline void GetCharacterWidths(HDC hdc, FontEntry& entry) noexcept {
    // Get widths for printable ASCII (32-126)
    INT widths[CHAR_COUNT] ;
    
    if (GetCharWidth32W(hdc, CHAR_START, CHAR_END, widths)) {
        for (int i = 0 ; i < CHAR_COUNT ; i++) {
            entry.charWidths[i] = static_cast<int16_t>(widths[i]) ;
        }
    } else {
        // Fallback: use avgCharWidth
        for (int i = 0 ; i < CHAR_COUNT ; i++) {
            entry.charWidths[i] = entry.avgCharWidth ;
        }
    }
}

[[nodiscard]] static std::optional<FontEntry> TryCreateFont(HDC hdc, const std::wstring& familyName, int requestedWeight, bool requestedItalic, bool includeCharWidths = true) noexcept {
    LOGFONTW lf{} ;
    wcscpy_s(lf.lfFaceName, familyName.c_str()) ;
    lf.lfHeight = -16 ;
    lf.lfWeight = requestedWeight ;
    lf.lfItalic = requestedItalic ? TRUE : FALSE ;
    lf.lfCharSet = DEFAULT_CHARSET ;
    lf.lfQuality = CLEARTYPE_QUALITY ;
    
    HFONT hFont = CreateFontIndirectW(&lf) ;
    if (!hFont) return std::nullopt ;
    
    HGDIOBJ oldFont = SelectObject(hdc, hFont) ;
    
    wchar_t actualName[256] ;
    const int nameLen = GetTextFaceW(hdc, 256, actualName) ;
    if (nameLen <= 0) {
        SelectObject(hdc, oldFont) ;
        DeleteObject(hFont) ;
        return std::nullopt ;
    }
    
    TEXTMETRICW tm{} ;
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
    
    FontEntry entry{} ;
    entry.SetName(name) ;
    entry.height = static_cast<int16_t>(tm.tmHeight) ;
    entry.ascent = static_cast<int16_t>(tm.tmAscent) ;
    entry.descent = static_cast<int16_t>(tm.tmDescent) ;
    entry.avgCharWidth = static_cast<int16_t>(tm.tmAveCharWidth) ;
    entry.maxCharWidth = static_cast<int16_t>(tm.tmMaxCharWidth) ;
    entry.weight = static_cast<int16_t>(tm.tmWeight) ;
    
    const bool isBold = (tm.tmWeight >= FW_BOLD) ;
    const bool isItalic = (tm.tmItalic != 0) ;
    entry.style = static_cast<FontStyle>((isBold ? 1 : 0) | (isItalic ? 2 : 0)) ;
    
    // Get character widths
    if (includeCharWidths) {
        GetCharacterWidths(hdc, entry) ;
    } else {
        // Fallback: all chars use avgCharWidth
        for (int i = 0 ; i < CHAR_COUNT ; i++) {
            entry.charWidths[i] = entry.avgCharWidth ;
        }
    }
    
    SelectObject(hdc, oldFont) ;
    DeleteObject(hFont) ;
    
    return entry ;
}

[[nodiscard]] inline std::optional<std::pair<size_t, size_t>> DumpInstalledFonts(const DumpOptions& opt) noexcept {
    HDC screen = GetDC(nullptr) ;
    if (!screen) return std::nullopt ;
    
    HDC dc = CreateCompatibleDC(screen) ;
    ReleaseDC(nullptr, screen) ;
    if (!dc) return std::nullopt ;

    FONTDUMP_LOG("Collecting font families...") ;
    CollectContext collectCtx ;
    LOGFONTW lf{} ;
    lf.lfCharSet = static_cast<BYTE>(opt.charset) ;
    EnumFontFamiliesExW(dc, &lf, CollectFontFamiliesProc, reinterpret_cast<LPARAM>(&collectCtx), 0) ;
    
    const size_t familyCount = collectCtx.families.size() ;
    FONTDUMP_LOG_VALUE("Found font families", familyCount) ;

    std::vector<FontEntry> entries ;
    entries.reserve(familyCount * 4) ;
    
    std::string csvOutput ;
    if (opt.write_csv) {
        csvOutput.reserve(opt.reserve_bytes) ;
        if (opt.write_bom) csvOutput.append("\xEF\xBB\xBF") ;
        if (opt.write_header) {
            csvOutput.append("Font Name,Height,Ascent,Descent,Avg Char Width,Max Char Width,Weight,Style\n") ;
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
            auto entryOpt = TryCreateFont(dc, family, weight, italic, opt.include_char_widths) ;
            if (!entryOpt) continue ;
            
            FontEntry entry = *entryOpt ;
            
            std::string key ;
            key.reserve(entry.GetName().size() + 2) ;
            key.append(entry.GetName()) ;
            key.push_back('|') ;
            key.push_back('0' + static_cast<char>(entry.style)) ;
            
            if (!seen.insert(key).second) continue ;
            
            entries.push_back(entry) ;
            
            if (opt.write_csv) {
                AppendCSVField(csvOutput, entry.GetName()) ;
                csvOutput.push_back(',') ;
                csvOutput.append(std::to_string(entry.height)) ;
                csvOutput.push_back(',') ;
                csvOutput.append(std::to_string(entry.ascent)) ;
                csvOutput.push_back(',') ;
                csvOutput.append(std::to_string(entry.descent)) ;
                csvOutput.push_back(',') ;
                csvOutput.append(std::to_string(entry.avgCharWidth)) ;
                csvOutput.push_back(',') ;
                csvOutput.append(std::to_string(entry.maxCharWidth)) ;
                csvOutput.push_back(',') ;
                csvOutput.append(std::to_string(entry.weight)) ;
                csvOutput.push_back(',') ;
                csvOutput.append(std::to_string(static_cast<int>(entry.style))) ;
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
            FONTDUMP_LOG_VALUE("CSV saved", opt.out_path_csv) ;
        }
    }

    size_t binBytes = 0 ;
    if (opt.write_binary && !entries.empty()) {
        std::ofstream binFile(opt.out_path_bin, std::ios::binary) ;
        if (binFile) {
            FontBinaryHeader header ;
            header.count = static_cast<uint32_t>(entries.size()) ;
            header.timestamp = static_cast<uint64_t>(time(nullptr)) ;
            
            binFile.write(reinterpret_cast<const char*>(&header), sizeof(header)) ;
            binFile.write(reinterpret_cast<const char*>(entries.data()), static_cast<std::streamsize>(entries.size() * sizeof(FontEntry))) ;
            
            binBytes = sizeof(header) + entries.size() * sizeof(FontEntry) ;
            FONTDUMP_LOG_VALUE("Binary saved", opt.out_path_bin) ;
            FONTDUMP_LOG_VALUE("Entry size", sizeof(FontEntry)) ;
        }
    }

    return std::make_pair(entries.size(), std::max(csvBytes, binBytes)) ;
}

[[nodiscard]] inline std::optional<std::unordered_map<std::string, FontEntry>> LoadFontsFromBinary(const std::string& filename) noexcept {
    std::ifstream file(filename, std::ios::binary) ;
    if (!file) return std::nullopt ;
    
    file.seekg(0, std::ios::end) ;
    const auto fileSize = file.tellg() ;
    file.seekg(0, std::ios::beg) ;
    
    if (fileSize < static_cast<std::streamoff>(sizeof(FontBinaryHeader))) {
        return std::nullopt ;
    }
    
    FontBinaryHeader header ;
    file.read(reinterpret_cast<char*>(&header), sizeof(header)) ;
    
    if (!file || !header.IsValid()) return std::nullopt ;
    
    const size_t expectedSize = sizeof(FontBinaryHeader) + header.count * sizeof(FontEntry) ;
    if (static_cast<size_t>(fileSize) < expectedSize) {
        return std::nullopt ;
    }
    
    std::unordered_map<std::string, FontEntry> fontMap ;
    fontMap.reserve(header.count) ;
    
    std::vector<FontEntry> entries(header.count) ;
    file.read(reinterpret_cast<char*>(entries.data()), static_cast<std::streamsize>(header.count * sizeof(FontEntry))) ;
    
    if (!file) return std::nullopt ;
    
    for (auto& entry : entries) {
        std::string key = entry.GetNameStr() ;
        key.push_back('|') ;
        key.push_back('0' + static_cast<char>(entry.style)) ;
        fontMap.emplace(std::move(key), std::move(entry)) ;
    }
    
    return fontMap ;
}

[[nodiscard]] inline const FontEntry* FindFont(const std::unordered_map<std::string, FontEntry>& fontMap, std::string_view name, FontStyle style = FontStyle::Regular) noexcept {
    std::string key ;
    key.reserve(name.size() + 2) ;
    key.append(name) ;
    key.push_back('|') ;
    key.push_back('0' + static_cast<char>(style)) ;
    
    auto it = fontMap.find(key) ;
    return (it != fontMap.end()) ? &it->second : nullptr ;
}

[[nodiscard]] inline int ToGdiplusFontStyle(FontStyle style) noexcept {
    return static_cast<int>(style) ;
}

[[nodiscard]] inline std::unique_ptr<Gdiplus::Font> CreateGdiplusFont(const FontEntry* entry, Gdiplus::REAL emSize = 12.0f) noexcept {
    if (!entry) return nullptr ;
    
    std::wstring wname(entry->GetName().begin(), entry->GetName().end()) ;
    
    return std::make_unique<Gdiplus::Font>(
        wname.c_str(),
        emSize,
        ToGdiplusFontStyle(entry->style),
        Gdiplus::UnitPixel
    ) ;
}

[[nodiscard]] inline std::unique_ptr<Gdiplus::FontFamily> CreateGdiplusFontFamily(
    const FontEntry* entry) noexcept 
{
    if (!entry) return nullptr ;
    
    std::wstring wname(entry->GetName().begin(), entry->GetName().end()) ;
    return std::make_unique<Gdiplus::FontFamily>(wname.c_str()) ;
}

inline bool CreateDumpFonts(const char* filenameWithoutExt, uint32_t reserve_bytes = 1 << 20, bool write_csv = false, bool write_binary = true, bool include_char_widths = true) noexcept {
    fontdump::DumpOptions opt ;
    opt.out_path_csv = filenameWithoutExt + std::string(".csv")  ;
    opt.out_path_bin = filenameWithoutExt + std::string(".bin")  ;
    opt.write_csv = write_csv ;
    opt.write_binary = write_binary ;
    opt.reserve_bytes = reserve_bytes ;
    opt.include_char_widths = include_char_widths ;

    auto res = fontdump::DumpInstalledFonts(opt) ;
    
    if (!res) { 
        return false ; 
    }

    return true ;
}

} // namespace fontdump