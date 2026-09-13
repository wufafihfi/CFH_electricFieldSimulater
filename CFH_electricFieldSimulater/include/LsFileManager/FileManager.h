#pragma once
#include <string>
#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>
#include <optional>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace fs = std::filesystem;

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

class FileManager {
public:
    static FileManager& Get() {
        static FileManager instance;
        return instance;
    }

    // ========== 基础路径操作 ==========

    // 获取可执行文件所在目录（唯一需要写死的）
    fs::path GetExecutablePath() const {
        return m_exePath;
    }

    // 获取当前工作目录
    fs::path GetCurrentPath() const {
        return fs::current_path();
    }

    // 设置当前工作目录
    bool SetCurrentPath(const fs::path& path) {
        std::error_code ec;
        fs::current_path(path, ec);
        return !ec;
    }

    // ========== 目录操作 ==========

    // 确保目录存在（如果不存在则创建）
    bool EnsureDirectoryExists(const fs::path& path) {
        if (fs::exists(path)) {
            return fs::is_directory(path);
        }
        std::error_code ec;
        return fs::create_directories(path, ec);
    }

    // 检查目录是否存在
    bool DirectoryExists(const fs::path& path) const {
        return fs::exists(path) && fs::is_directory(path);
    }

    // ========== 文件信息操作 ==========

    // 检查文件是否存在
    bool FileExists(const fs::path& path) const {
        std::error_code ec;
        return fs::exists(path, ec) && fs::is_regular_file(path, ec);
    }

    // 获取文件大小
    size_t GetFileSize(const fs::path& path) const {
        if (!FileExists(path)) return 0;
        std::error_code ec;
        return fs::file_size(path, ec);
    }

    // 获取文件最后修改时间
    auto GetFileTime(const fs::path& path) const {
        if (!FileExists(path)) return fs::file_time_type{};
        std::error_code ec;
        return fs::last_write_time(path, ec);
    }

    // ========== 文件打开操作 ==========

    // 打开文件用于读取（二进制模式）
    std::unique_ptr<std::ifstream> OpenForRead(const fs::path& path, std::ios::openmode mode = std::ios::binary) {
        auto file = std::make_unique<std::ifstream>();
#ifdef _WIN32
        std::wstring wpath = path.wstring();

        // 使用 _wfopen_s
        FILE* fp = nullptr;
        errno_t err = _wfopen_s(&fp, wpath.c_str(), L"rb");
        if (err != 0 || !fp) {
            LOG_ERROR_STREAM << u8"无法打开文件进行读取: " << path.u8string();
            return nullptr;
        }

        // 将 FILE* 转换为 std::ifstream（需要 C++11 的 rdbuf 方法）
        file = std::make_unique<std::ifstream>(fp);
        if (!file->is_open()) {
            fclose(fp);
            LOG_ERROR_STREAM << u8"无法打开文件进行读取: " << path.u8string();
            return nullptr;
        }
#else
        file->open(path.u8string(), mode);
#endif
        return file;
    }

    // 打开文件用于写入（二进制模式）
    std::unique_ptr<std::ofstream> OpenForWrite(const fs::path& path, std::ios::openmode mode = std::ios::binary) {
        // 确保父目录存在
        EnsureDirectoryExists(path.parent_path());

        auto file = std::make_unique<std::ofstream>();
#ifdef _WIN32
        file->open(path.wstring(), mode);
#else
        file->open(path.u8string(), mode);
#endif
        if (!file->is_open()) {
            LOG_ERROR_STREAM << u8"无法打开文件进行写入: " << path.u8string();
            return nullptr;
        }
        return file;
    }

    // 打开文件用于追加写入
    std::unique_ptr<std::ofstream> OpenForAppend(const fs::path& path) {
        return OpenForWrite(path, std::ios::binary | std::ios::app);
    }

    // 打开文件用于读取文本
    std::unique_ptr<std::ifstream> OpenForReadText(const fs::path& path) {
        return OpenForRead(path, std::ios::in);
    }

    // 打开文件用于写入文本
    std::unique_ptr<std::ofstream> OpenForWriteText(const fs::path& path) {
        return OpenForWrite(path, std::ios::out);
    }

    // ========== 文件读写操作 ==========

    // 读取整个文件内容（文本）
    std::string ReadAllText(const fs::path& path) {
        auto file = OpenForReadText(path);
        if (!file) return "";

        std::stringstream ss;
        ss << file->rdbuf();
        return ss.str();
    }

    // 读取整个文件内容（二进制）
    std::vector<uint8_t> ReadAllBytes(const fs::path& path) {
        auto file = OpenForRead(path);
        if (!file) return {};

        file->seekg(0, std::ios::end);
        size_t size = static_cast<size_t>(file->tellg());
        file->seekg(0, std::ios::beg);

        std::vector<uint8_t> buffer(size);
        file->read(reinterpret_cast<char*>(buffer.data()), size);
        return buffer;
    }

    // 写入文本到文件
    bool WriteAllText(const fs::path& path, const std::string& content) {
        auto file = OpenForWriteText(path);
        if (!file) return false;

        *file << content;
        return true;
    }

    // 写入二进制到文件
    bool WriteAllBytes(const fs::path& path, const uint8_t* data, size_t size) {
        auto file = OpenForWrite(path);
        if (!file) return false;

        file->write(reinterpret_cast<const char*>(data), size);
        return true;
    }

    // ========== 文件扫描 ==========

    // 扫描目录下所有文件（可选扩展名过滤）
    std::vector<fs::path> ScanFiles(const fs::path& directory, const std::string& extension = "") {
        std::vector<fs::path> files;

        if (!DirectoryExists(directory)) {
            LOG_WARNING_STREAM << u8"目录不存在: " << directory.u8string();
            return files;
        }

        try {
#ifdef _WIN32
            // Windows 下使用宽字符串遍历
            std::wstring widePath = directory.wstring();
            for (const auto& entry : fs::directory_iterator(widePath)) {
                if (entry.is_regular_file()) {
                    if (extension.empty()) {
                        files.push_back(entry.path());
                    }
                    else {
                        std::string ext = entry.path().extension().string();
                        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                        std::string targetExt = extension;
                        std::transform(targetExt.begin(), targetExt.end(), targetExt.begin(), ::tolower);
                        if (ext == targetExt || (targetExt[0] != '.' && ext == "." + targetExt)) {
                            files.push_back(entry.path());
                        }
                    }
                }
            }
#else
            for (const auto& entry : fs::directory_iterator(directory)) {
                if (entry.is_regular_file()) {
                    if (extension.empty()) {
                        files.push_back(entry.path());
                    }
                    else {
                        std::string ext = entry.path().extension().string();
                        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                        std::string targetExt = extension;
                        std::transform(targetExt.begin(), targetExt.end(), targetExt.begin(), ::tolower);
                        if (ext == targetExt || (targetExt[0] != '.' && ext == "." + targetExt)) {
                            files.push_back(entry.path());
                        }
                    }
                }
            }
#endif
        }
        catch (const std::exception& e) {
            LOG_ERROR_STREAM << u8"扫描目录异常: " << e.what();
        }

        return files;
    }

    // 递归扫描目录下所有文件
    std::vector<fs::path> ScanFilesRecursive(const fs::path& directory, const std::string& extension = "") {
        std::vector<fs::path> files;

        if (!DirectoryExists(directory)) {
            LOG_WARNING_STREAM << u8"目录不存在: " << directory.u8string();
            return files;
        }

        try {
#ifdef _WIN32
            std::wstring widePath = directory.wstring();
            for (const auto& entry : fs::recursive_directory_iterator(widePath)) {
                if (entry.is_regular_file()) {
                    if (extension.empty()) {
                        files.push_back(entry.path());
                    }
                    else {
                        std::string ext = entry.path().extension().string();
                        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                        std::string targetExt = extension;
                        std::transform(targetExt.begin(), targetExt.end(), targetExt.begin(), ::tolower);
                        if (ext == targetExt || (targetExt[0] != '.' && ext == "." + targetExt)) {
                            files.push_back(entry.path());
                        }
                    }
                }
            }
#else
            for (const auto& entry : fs::recursive_directory_iterator(directory)) {
                if (entry.is_regular_file()) {
                    if (extension.empty()) {
                        files.push_back(entry.path());
                    }
                    else {
                        std::string ext = entry.path().extension().string();
                        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                        std::string targetExt = extension;
                        std::transform(targetExt.begin(), targetExt.end(), targetExt.begin(), ::tolower);
                        if (ext == targetExt || (targetExt[0] != '.' && ext == "." + targetExt)) {
                            files.push_back(entry.path());
                        }
                    }
                }
            }
#endif
        }
        catch (const std::exception& e) {
            LOG_ERROR_STREAM << u8"递归扫描目录异常: " << e.what();
        }

        return files;
    }

    // 扫描目录下所有子目录
    std::vector<fs::path> ScanDirectories(const fs::path& directory) {
        std::vector<fs::path> dirs;

        if (!DirectoryExists(directory)) {
            LOG_WARNING_STREAM << u8"目录不存在: " << directory.u8string();
            return dirs;
        }

        try {
#ifdef _WIN32
            std::wstring widePath = directory.wstring();
            for (const auto& entry : fs::directory_iterator(widePath)) {
                if (entry.is_directory()) {
                    dirs.push_back(entry.path());
                }
            }
#else
            for (const auto& entry : fs::directory_iterator(directory)) {
                if (entry.is_directory()) {
                    dirs.push_back(entry.path());
                }
            }
#endif
        }
        catch (const std::exception& e) {
            LOG_ERROR_STREAM << u8"扫描子目录异常: " << e.what();
        }

        return dirs;
    }

    // ========== 文件夹操作 ==========

    // 在资源管理器中打开文件夹
    bool OpenFolderInExplorer(const fs::path& path) {
        if (!DirectoryExists(path)) {
            LOG_WARNING_STREAM << u8"目录不存在，无法打开: " << path.u8string();
            return false;
        }

#ifdef _WIN32
        std::wstring wpath = path.wstring();
        HINSTANCE result = ShellExecuteW(NULL, L"open", L"explorer", wpath.c_str(), NULL, SW_SHOW);
        return reinterpret_cast<intptr_t>(result) > 32;
#else
        std::string command = "xdg-open \"" + path.u8string() + "\"";
        return system(command.c_str()) == 0;
#endif
    }

    // 在资源管理器中打开并选中文件
    bool OpenFileInExplorer(const fs::path& path) {
        if (!FileExists(path)) {
            LOG_WARNING_STREAM << "文件不存在，无法打开: " << path.u8string();
            return false;
        }

#ifdef _WIN32
        std::wstring wpath = path.wstring();
        HINSTANCE result = ShellExecuteW(NULL, L"open", L"explorer",
            (L"/select,\"" + wpath + L"\"").c_str(), NULL, SW_SHOW);
        return reinterpret_cast<intptr_t>(result) > 32;
#else
        // Linux/Mac 下打开文件所在目录
        fs::path parent = path.parent_path();
        std::string command = "xdg-open \"" + parent.u8string() + "\"";
        return system(command.c_str()) == 0;
#endif
    }

    // ========== 路径转换 ==========

    // 将路径转换为 UTF-8 字符串（用于显示）
    std::string ToUtf8String(const fs::path& path) const {
        return path.u8string();
    }

    // 将 UTF-8 字符串转换为路径
    fs::path FromUtf8String(const std::string& utf8Str) const {
        return fs::path(utf8Str);
    }

#ifdef _WIN32
    // 将路径转换为宽字符串（Windows API）
    std::wstring ToWideString(const fs::path& path) const {
        return path.wstring();
    }

    // 将宽字符串转换为路径
    fs::path FromWideString(const std::wstring& wideStr) const {
        return fs::path(wideStr);
    }
#endif

    // ========== 文件名操作 ==========

    // 获取文件名（不含路径）
    std::string GetFilename(const fs::path& path) const {
        return path.filename().u8string();
    }

    // 获取文件名（不含扩展名）
    std::string GetFilenameWithoutExtension(const fs::path& path) const {
        return path.stem().u8string();
    }

    // 获取扩展名
    std::string GetExtension(const fs::path& path) const {
        std::string ext = path.extension().u8string();
        if (!ext.empty() && ext[0] == '.') {
            ext = ext.substr(1);
        }
        return ext;
    }

    // 组合路径（安全版本）
    fs::path Combine(const fs::path& base, const std::string& sub) const {
        return base / sub;
    }

    // ========== 文件名生成 ==========

    // 生成唯一的文件名（避免重名）
    fs::path GenerateUniqueFilename(const fs::path& directory, const std::string& baseName, const std::string& extension) {
        fs::path result = directory / (baseName + "." + extension);
        int counter = 1;

        while (FileExists(result)) {
            result = directory / (baseName + "_" + std::to_string(counter) + "." + extension);
            counter++;
        }

        return result;
    }

    // 生成带时间戳的文件名
    fs::path GenerateTimestampFilename(const fs::path& directory, const std::string& prefix, const std::string& extension) {
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

        std::string filename = prefix + "_" + ss.str() + "." + extension;
        return directory / filename;
    }

    // ========== UTF-8 字符串直接支持（Windows 宽字符兼容） ==========

// 从 UTF-8 字符串创建路径（自动处理 Windows 宽字符转换）
    static fs::path FromUtf8(const std::string& utf8Str) {
#ifdef _WIN32
        if (utf8Str.empty()) return fs::path();

        int wideLen = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), (int)utf8Str.size(), nullptr, 0);
        if (wideLen == 0) {
            // 降级：可能是纯 ASCII 或转换失败
            return fs::path(utf8Str);
        }

        std::wstring wideStr(wideLen, 0);
        MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), (int)utf8Str.size(), &wideStr[0], wideLen);
        return fs::path(wideStr);
#else
        return fs::path(utf8Str);
#endif
    }

    // 从 UTF-8 字符串检查文件是否存在
    bool FileExistsUtf8(const std::string& utf8Path) const {
        return FileExists(FromUtf8(utf8Path));
    }

    // 从 UTF-8 字符串检查目录是否存在
    bool DirectoryExistsUtf8(const std::string& utf8Path) const {
        return DirectoryExists(FromUtf8(utf8Path));
    }

    // 从 UTF-8 字符串打开文件（读取）
    std::unique_ptr<std::ifstream> OpenForReadUtf8(const std::string& utf8Path, std::ios::openmode mode = std::ios::binary) {
        return OpenForRead(FromUtf8(utf8Path), mode);
    }

    // 从 UTF-8 字符串打开文件（写入）
    std::unique_ptr<std::ofstream> OpenForWriteUtf8(const std::string& utf8Path, std::ios::openmode mode = std::ios::binary) {
        return OpenForWrite(FromUtf8(utf8Path), mode);
    }

    // 从 UTF-8 字符串扫描文件
    std::vector<fs::path> ScanFilesUtf8(const std::string& directory, const std::string& extension = "") {
        return ScanFiles(FromUtf8(directory), extension);
    }

private:
    FileManager() {
        // 初始化可执行文件路径（这是唯一需要写死的）
#ifdef _WIN32
        wchar_t szFilePath[MAX_PATH + 1] = { 0 };
        GetModuleFileNameW(NULL, szFilePath, MAX_PATH);
        m_exePath = fs::path(szFilePath).parent_path();
#else
        char buffer[PATH_MAX] = { 0 };
        ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (len != -1) {
            buffer[len] = '\0';
            m_exePath = fs::path(buffer).parent_path();
        }
        else {
            m_exePath = fs::current_path();
        }
#endif
    }

    ~FileManager() = default;
    FileManager(const FileManager&) = delete;
    FileManager& operator=(const FileManager&) = delete;

    fs::path m_exePath;
};

// ========== 便捷宏定义 ==========
#define FILE_MGR FileManager::Get()