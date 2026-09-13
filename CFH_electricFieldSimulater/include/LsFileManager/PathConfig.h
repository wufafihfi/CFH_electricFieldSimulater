// PathConfig.h - 业务路径配置（可自定义）
#pragma once
#include "FileManager.h"

class PathConfig {
public:
    static PathConfig& Get() {
        static PathConfig instance;
        return instance;
    }

    // 初始化所有目录（在程序启动时调用）
    void Initialize() {
        // 可以直接使用 FileManager 创建目录
        FILE_MGR.EnsureDirectoryExists(GetDataPath());
        FILE_MGR.EnsureDirectoryExists(GetConfigPath());
        FILE_MGR.EnsureDirectoryExists(GetLogPath());;
        FILE_MGR.EnsureDirectoryExists(GetFontPath());
    }

    // 数据目录
    fs::path GetDataPath() const {
        return FILE_MGR.GetExecutablePath() / "data";
    }

    // 配置目录
    fs::path GetConfigPath() const {
        return FILE_MGR.GetExecutablePath() / "config";
    }

    // 日志目录
    fs::path GetLogPath() const {
        return FILE_MGR.GetExecutablePath() / "log";
    }

    // 字体目录
    fs::path GetFontPath() const {
        return FILE_MGR.GetExecutablePath() / "font";
    }

    //  默认字体路径
    fs::path GetDefaultFontPath() const {
        return FILE_MGR.GetExecutablePath() / "font" / "DefaultFont" / "SimHei.ttf";
    }

    // 配置文件路径
    fs::path GetMainConfigPath() const {
        return GetConfigPath() / "mainConfig.json";
    }

    // 日志文件路径
    fs::path GetMainLogPath() const {
        return GetLogPath() / "MainLog.log";
    }

private:
    PathConfig() = default;
};

#define PATH_CFG PathConfig::Get()