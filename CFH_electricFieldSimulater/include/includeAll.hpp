#pragma once
#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>

#include <windows.h>
#include <string>
#include <filesystem>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <psapi.h>

#include "Thread_tools/ThreadPool.h"
#include "PropertySystem.h"
#include "Log/LogSystem.h"
#include "LsFileManager/FileManager.h"
#include "LsFileManager/PathConfig.h"
#include "fileTools.h"
#include "dllChecker.h"
#include "LsSimpleDraw/Canvas.h"
#include "LsSimpleDraw/RatioView.h"

#include "globalData.h"

#include "LsSimpleDraw/SimpleDraw.h"

#include "simulater_main.h"
#include "simulater_UI.h"