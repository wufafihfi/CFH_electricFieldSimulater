#include "include/includeAll.hpp"

sf::RenderWindow m_window;

M_Property::PropertyContainer m_properties;

std::vector<float> bg_color = { 40.0f, 40.0f, 40.0f};

bool showLogWindow = true;

void ProcessEvents() {
    ImGuiIO& io = ImGui::GetIO();
    while (const auto event = m_window.pollEvent()) {
        ImGui::SFML::ProcessEvent(m_window, *event);

        if (event->is<sf::Event::Closed>()) {
            m_window.close();
        }
    }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	LogSystem::getInstance().init(PATH_CFG.GetLogPath() / "MainLog.log", LogLevel::DEBUG, true, true, 5000);
	LogSystem::getInstance().setWindowTitle(u8"日志");

	// 窗口
	sf::ContextSettings settings;
	settings.antiAliasingLevel = 4;

    m_window = sf::RenderWindow(sf::VideoMode({ 1600, 1000 }),
        L"电场强度可视化",
        sf::Style::Default,
        sf::State::Windowed,
        settings);
    m_window.setVerticalSyncEnabled(true);
    m_window.setFramerateLimit(120);

    LOG_INFO(u8"窗口创建 Window created");

    // imgui
    sf::Clock m_deltaClock;
    if (!ImGui::SFML::Init(m_window)) {
        return 0;
    }
    LOG_INFO(u8"IMGUI初始化成功 ImGui init OK");

    ImGuiStyle& style = ImGui::GetStyle();
    // 全局透明
    style.Alpha = 0.8f;
    // 设置各项圆角半径
    style.WindowRounding = 6.0f;      // 主窗口的圆角[citation:9]
    style.ChildRounding = 6.0f;       // 子窗口的圆角
    style.FrameRounding = 5.0f;       // 框架类控件（如按钮）的圆角[citation:3]
    style.PopupRounding = 6.0f;       // 弹出窗口的圆角
    style.ScrollbarRounding = 9.0f;   // 滚动条的圆角
    style.GrabRounding = 4.0f;        // 滑块把手的圆角
    style.TabRounding = 4.0f;         // 标签页的圆角[citation:2]
    // 边框大小
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;

    ImGuiIO& io = ImGui::GetIO();
    // 触摸屏兼容
    io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.MouseDragThreshold = 4.0f;
    // Docking
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // 启用 Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    // 窗口位置保存
    static std::string layoutiniPath = (PATH_CFG.GetConfigPath() / "imgui_layout.ini").u8string();
    io.IniFilename = layoutiniPath.c_str();
    // 字体
    io.Fonts->Flags |= ImFontAtlasFlags_NoPowerOfTwoHeight;
    io.Fonts->TexDesiredWidth = 2048;
    std::filesystem::path _fontFilePath = PATH_CFG.GetFontPath() / "SimHei.ttf";
    setFont(_fontFilePath, _fontFilePath,17);
    // DPI-imgui
    io.FontGlobalScale = 1.0f;
    // 模块
    simulater_UI::Init(&m_window, &m_properties);
    simulater_main::Init(&m_window, &m_properties);

    while (m_window.isOpen()) {
        ProcessEvents();
        ImGui::SFML::Update(m_window, m_deltaClock.restart());

        // view render
        m_window.clear(sf::Color(
            static_cast<std::uint8_t>(bg_color[0]),
            static_cast<std::uint8_t>(bg_color[1]),
            static_cast<std::uint8_t>(bg_color[2])
        ));

        simulater_main::Render();

        // imgui render
        ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode);

        simulater_UI::Render();
        SHOW_LOG_WINDOW(&showLogWindow);

        ImGui::SFML::Render(m_window);
        m_window.display();
    }
}