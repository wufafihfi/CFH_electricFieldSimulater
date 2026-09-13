#pragma once
#include <windows.h>
#include <string>
#include <iostream>
#include <filesystem>
#include <json/json.h>
#include <vector>
#include <algorithm>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

//###############学习#################
// 阅 - n遍
//####################################

// ========== 路径相关（兼容旧代码） ==========
inline fs::path getExecutablePath() {
    return FILE_MGR.GetExecutablePath();
}

// ========== UTF-8 与宽字符转换 ==========
#ifdef _WIN32
// UTF-8 转 UTF-16 (Windows宽字符)
inline std::wstring Utf8ToWide(const std::string& utf8Str) {
    if (utf8Str.empty()) return L"";

    int wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), (int)utf8Str.size(), nullptr, 0);
    if (wideLen == 0) {
        LOG_ERROR_STREAM << u8"Utf8ToWide: MultiByteToWideChar Failed, error code:" << GetLastError();
        return L"";
    }

    std::wstring wideStr(wideLen, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), (int)utf8Str.size(), &wideStr[0], wideLen);

    return wideStr;
}

// 宽字符转 UTF-8
inline std::string WideToUtf8(const std::wstring& wideStr) {
    if (wideStr.empty()) return "";

    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), (int)wideStr.size(), nullptr, 0, nullptr, nullptr);
    if (utf8Len == 0) return "";

    std::string utf8Str(utf8Len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), (int)wideStr.size(), &utf8Str[0], utf8Len, nullptr, nullptr);

    return utf8Str;
}
#endif

// ========== 文件/目录检查（兼容旧代码，内部使用 FileManager） ==========
inline bool CheckIsDirectory(const std::string& path) {
    if (path.empty()) return false;
    return FILE_MGR.DirectoryExistsUtf8(path);
}

inline bool CheckFileExists(const std::string& path) {
    if (path.empty()) return false;
    return FILE_MGR.FileExistsUtf8(path);
}

// ========== 字体扫描 ==========
inline std::vector<fs::path> scanFontFiles(const std::string& folderPath, const std::string& extension) {
    std::vector<fs::path> fontFiles;

    if (!CheckIsDirectory(folderPath)) {
        LOG_ERROR_STREAM << u8"Error:目标路径不是目录或不存在";
        return fontFiles;
    }

#ifdef _WIN32
    LOG_INFO_STREAM << u8"使用宽字符串版本遍历目录...";
    try {
        std::wstring widePath = Utf8ToWide(folderPath);
        for (const auto& entry : fs::directory_iterator(widePath)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                if (ext == extension) {
                    fontFiles.push_back(entry.path());
                    LOG_INFO_STREAM << u8"找到字体文件: " << entry.path().filename().string();
                }
            }
        }
    }
    catch (const std::exception& e) {
        LOG_ERROR_STREAM << u8"扫描目录异常: " << e.what();
        return fontFiles;
    }
#else
    LOG_INFO_STREAM << u8"使用 UTF-8 版本遍历目录...";
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == extension) {
                fontFiles.push_back(entry.path());
            }
        }
    }
#endif

    LOG_INFO_STREAM << u8"找到 " << fontFiles.size() << u8" 个字体文件";
    return fontFiles;
}

inline std::string AnsiToUtf8(const std::string& ansiStr) {
    if (ansiStr.empty()) return "";

    int wideLen = MultiByteToWideChar(CP_ACP, 0, ansiStr.c_str(), -1, nullptr, 0);
    if (wideLen == 0) return "";

    std::wstring wideStr(wideLen, 0);
    MultiByteToWideChar(CP_ACP, 0, ansiStr.c_str(), -1, &wideStr[0], wideLen);

    int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (utf8Len == 0) return "";

    std::string utf8Str(utf8Len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, &utf8Str[0], utf8Len, nullptr, nullptr);

    if (!utf8Str.empty() && utf8Str.back() == '\0') {
        utf8Str.pop_back();
    }
    return utf8Str;
}

inline std::string getFileName(const std::string& fullPath) {
    size_t lastSlash = fullPath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        return fullPath.substr(lastSlash + 1);
    }
    return fullPath;
}

// ========== 字体加载 ==========
inline bool AddFontFromFileTTF(ImGuiIO& io, const fs::path& fontPath, float fontSize, const ImWchar* glyphRanges = nullptr) {
    std::string utf8Path = fontPath.u8string();
    return io.Fonts->AddFontFromFileTTF(utf8Path.c_str(), fontSize, nullptr, glyphRanges) != nullptr;
}

inline bool setFont(fs::path DefaultFontPath, fs::path path, float size = 16.0f) {
    if (path.empty()) {
        LOG_WARNING_STREAM << u8"目标字体路径为空，使用默认字体";
        path = DefaultFontPath;
    }

    LOG_INFO_STREAM << u8"字体加载:";
    LOG_INFO_STREAM << u8"默认字体路径: " << DefaultFontPath.u8string();
    LOG_INFO_STREAM << u8"目标字体路径: " << path.u8string();

    bool DefaultFontFileExists = FILE_MGR.FileExists(DefaultFontPath);
    bool FontFileExists = FILE_MGR.FileExists(path);
    if (!(DefaultFontFileExists && FontFileExists)) {
        LOG_ERROR_STREAM << u8"字体文件不存在:";
        if (!DefaultFontFileExists) LOG_ERROR_STREAM << u8" 默认字体不存在";
        if (!FontFileExists) LOG_ERROR_STREAM << u8" 目标字体不存在";
        return false;
    }

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();

    ImFontConfig simhei_config;
    simhei_config.OversampleH = 1;
    simhei_config.OversampleV = 1;
    simhei_config.PixelSnapH = true;

    std::string defaultFontUtf8 = DefaultFontPath.u8string();
    std::string fontUtf8 = path.u8string();

    ImFont* defaultFont = io.Fonts->AddFontFromFileTTF(defaultFontUtf8.c_str(),
        size,
        &simhei_config,
        io.Fonts->GetGlyphRangesChineseFull()
    );

    if (!defaultFont) {
        LOG_ERROR_STREAM << u8"默认字体加载失败";
        io.Fonts->AddFontDefault();
    }

    ImFont* font = io.Fonts->AddFontFromFileTTF(fontUtf8.c_str(),
        size,
        nullptr,
        io.Fonts->GetGlyphRangesChineseFull()
    );

    io.Fonts->Build();
    ImGui::SFML::UpdateFontTexture();

    return font != nullptr;
}

// ========== JSON 文件操作（只保留 fs::path 版本） ==========
inline bool saveJsonFile(const fs::path& filename, const Json::Value& root) {
    if (!FILE_MGR.EnsureDirectoryExists(filename.parent_path())) {
        LOG_ERROR_STREAM << u8"无法创建目录: " << filename.parent_path().u8string();
        return false;
    }

    auto file = FILE_MGR.OpenForWrite(filename);
    if (!file) {
        LOG_ERROR_STREAM << u8"无法打开文件进行写入: " << filename.u8string();
        return false;
    }

    Json::StreamWriterBuilder writerBuilder;
    writerBuilder["indentation"] = "  ";
    writerBuilder["commentStyle"] = "None";
    writerBuilder["emitUTF8"] = true;

    try {
        std::unique_ptr<Json::StreamWriter> writer(writerBuilder.newStreamWriter());
        writer->write(root, file.get());
        file->close();
        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR_STREAM << u8"保存JSON文件时发生异常: " << e.what();
        file->close();
        return false;
    }
}

inline bool readJsonFile(const fs::path& filename, Json::Value& root) {
    auto file = FILE_MGR.OpenForRead(filename);

    if (!file) {
        LOG_WARNING_STREAM << u8"配置文件不存在，将创建默认配置: " << filename.u8string();
        FILE_MGR.EnsureDirectoryExists(filename.parent_path());

        Json::Value defaultRoot;
        if (saveJsonFile(filename, defaultRoot)) {
            root = defaultRoot;
            return true;
        }
        return false;
    }

    Json::CharReaderBuilder readerBuilder;
    std::string parseErrors;

    file->seekg(0, std::ios::end);
    std::streampos fileSize = file->tellg();
    file->seekg(0, std::ios::beg);

    std::string fileContent;
    fileContent.resize(static_cast<size_t>(fileSize));
    file->read(fileContent.data(), fileContent.size());
    file->close();

    std::istringstream contentStream(fileContent);
    bool success = Json::parseFromStream(readerBuilder, contentStream, &root, &parseErrors);

    if (!success) {
        LOG_ERROR_STREAM << u8"JSON解析失败: " << parseErrors << u8" 文件: " << filename.u8string();
        return false;
    }

    return true;
}

// ========== 文件名生成 ==========
static std::string generateTimestampFilename(std::string extension) {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::stringstream ss;

#ifdef _WIN32
    struct tm buf;
    localtime_s(&buf, &now_time_t);
    ss << std::put_time(&buf, "%Y%m%d_%H%M%S");
#else
    struct tm buf;
    localtime_r(&now_time_t, &buf);
    ss << std::put_time(&buf, "%Y%m%d_%H%M%S");
#endif

    ss << "_" << std::setfill('0') << std::setw(3) << ms.count();
    ss << extension;

    return ss.str();
}

// ========== 文件夹打开 ==========
static void OpenFolderInExplorer(const std::string& path) {
#ifdef _WIN32
    std::wstring wpath = Utf8ToWide(path);
    ShellExecuteW(NULL, L"open", L"explorer", wpath.c_str(), NULL, SW_SHOW);
#else
    std::string command = "xdg-open \"" + path + "\"";
    system(command.c_str());
#endif
}