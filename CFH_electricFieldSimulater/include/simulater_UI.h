#pragma once

namespace simulater_UI {
	sf::RenderWindow* window;
	M_Property::PropertyContainer* properties;

	void Init(
		sf::RenderWindow* _window,
		M_Property::PropertyContainer* _properties
	) {
		window = _window;
		properties = _properties;
	}

	void electricFieldSimulater();
	void simulaterSetting();

	void Render() {
		electricFieldSimulater();
		simulaterSetting();
	}

    // simulater_UI.h

    // 绘制竖向颜色条 学阅1
    void drawColorBarVertical(
        const char* label,
        float minValue, float maxValue,
        std::function<sf::Color(float)> colorMapper,
        int numTicks = 5
    ) {
        ImGui::BeginGroup();
        ImGui::Text("%s", label);

        const float barWidth = 20.0f;
        const float barHeight = 180.0f;

        ImVec2 barPos = ImGui::GetCursorScreenPos();
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // 渐变（分段绘制）
        const int segments = 64;
        for (int i = 0; i < segments; ++i) {
            float t0 = static_cast<float>(i) / segments;
            float t1 = static_cast<float>(i + 1) / segments;

            float y0 = barPos.y + barHeight * (1.0f - t1);
            float y1 = barPos.y + barHeight * (1.0f - t0);

            sf::Color c0 = colorMapper(t0);
            sf::Color c1 = colorMapper(t1);

            ImU32 col0 = IM_COL32(c0.r, c0.g, c0.b, c0.a);
            ImU32 col1 = IM_COL32(c1.r, c1.g, c1.b, c1.a);

            drawList->AddRectFilledMultiColor(
                ImVec2(barPos.x, y0),
                ImVec2(barPos.x + barWidth, y1),
                col0, col0, col1, col1
            );
        }

        // 边框
        drawList->AddRect(
            barPos,
            ImVec2(barPos.x + barWidth, barPos.y + barHeight),
            IM_COL32(255, 255, 255, 128)
        );

        // 刻度 + 数值
        for (int i = 0; i <= numTicks; ++i) {
            float t = static_cast<float>(i) / numTicks;
            float y = barPos.y + barHeight * (1.0f - t);

            drawList->AddLine(
                ImVec2(barPos.x + barWidth, y),
                ImVec2(barPos.x + barWidth + 5, y),
                IM_COL32(255, 255, 255, 200)
            );

            float value = minValue + t * (maxValue - minValue);
            char buf[64];
            snprintf(buf, sizeof(buf), "%.2e", value);

            drawList->AddText(
                ImVec2(barPos.x + barWidth + 8, y - 7),
                IM_COL32(255, 255, 255, 255),
                buf
            );
        }

        ImGui::Dummy(ImVec2(barWidth + 70, barHeight));
        ImGui::EndGroup();
    }

    void electricFieldSimulater() {
        ImGui::Begin(u8"电场模拟热力图", nullptr, ImGuiWindowFlags_NoScrollbar);

        // FPS / 线程数 / 内存
        static sf::Clock clock;
        float deltaTime = clock.restart().asSeconds();
        float fps = 1.0f / deltaTime;
        ImGui::Text(u8"帧率: %.1f FPS", fps);
        ImGui::SameLine();

        ImGui::Text(u8"|线程数: %d / %d", globalData::target_numThreads, globalData::max_numThreads);
        ImGui::SameLine();

        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
            ImGui::Text(u8"|内存: %d MB", pmc.WorkingSetSize / 1024 / 1024);
        }

        ImGui::Separator();

        // ========== 左图像 + 右颜色条 ==========
        float totalWidth = ImGui::GetContentRegionAvail().x;
        float imageWidth = totalWidth * 0.8f;
        float legendWidth = totalWidth * 0.2f;

        // 左侧图像
        ImGui::BeginChild("ImageArea", ImVec2(imageWidth, 0), false);

        // 自适应缩放
        sf::Vector2f canvasSize = globalData::mainCanvas_ptr->getSize();
        ImVec2 available = ImGui::GetContentRegionAvail();
        float scale = std::min(available.x / canvasSize.x, available.y / canvasSize.y);
        sf::Vector2f displaySize(canvasSize.x * scale, canvasSize.y * scale);

        ImVec2 imagePos = ImGui::GetCursorScreenPos();

        if(globalData::potential_layer_alpha != 1.0f)
        {
            ImGui::Image(globalData::mainCanvas_ptr->getTexture(),
                displaySize,
                sf::Color::White,
                sf::Color::Transparent);
        }

        if (globalData::is_show_potential_layer) {
            ImGui::SetCursorScreenPos(imagePos);
            uint8_t alpha = static_cast<uint8_t>(globalData::potential_layer_alpha * 255.0f);
            ImGui::Image(globalData::electricPotentialCanvas_ptr->getTexture(),
                displaySize,
                sf::Color(255, 255, 255, alpha),
                sf::Color::Transparent);
        }

        // 鼠标坐标映射 第一层
        ImVec2 mousePos = ImGui::GetMousePos();
        float relX = mousePos.x - imagePos.x;
        float relY = mousePos.y - imagePos.y;
        float canvasX = relX / scale;
        float canvasY = relY / scale;
        globalData::mousePos_mainCanvas_imgui = sf::Vector2f(canvasX, canvasY);
        
        ImGui::EndChild();

        ImGui::SameLine();

        // 右侧颜色条
        ImGui::BeginChild("LegendArea", ImVec2(legendWidth, 0), false);

        // 场强颜色条
        if (globalData::potential_layer_alpha != 1.0f) {
            drawColorBarVertical(
                u8"场强 gamma = 1",
                0.0f, globalData::E_Heatmap_Emax,
                [](float n) { return SimpleDraw::vibrantColorMap(n); },
                5
            );
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // 电势颜色条
        if (globalData::is_show_potential_layer) {
            drawColorBarVertical(
                u8"电势 gamma = 1",
                -globalData::V_Heatmap_Vmax, globalData::V_Heatmap_Vmax,
                [](float n) { return SimpleDraw::potentialColorMap(n); },
                5
            );
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text(u8"电荷信息:");
        for (auto charge : *globalData::charges) {
            ImGui::Text(u8"id:%d", charge.id);
            ImGui::SameLine();
            ImGui::Text(u8"|Q:%g C", charge.quantity);
        }

        ImGui::EndChild();

        ImGui::End();
    }

	void simulaterSetting() {
		ImGui::Begin(u8"电场模拟参数调整");

		ImGui::Text(u8"场强热力图伽马值((E / E_max )^gamma)");
		ImGui::SliderFloat(u8"##E_slider", &globalData::E_Heatmap_gamma, 0.1f, 2.0f);
        ImGui::Text(u8"热力图颜色映射范围 Emax = %0.1f ([0,Emax])", globalData::E_Heatmap_Emax);
        ImGui::SliderFloat(u8"##Emax_slider", &globalData::E_Heatmap_Emax, 1.0f, 5000000.0f);

		ImGui::Checkbox(u8"显示坐标轴", &globalData::is_show_coordinateAxis);

        ImGui::Checkbox(u8"鼠标点电荷", &globalData::is_put_mouseCharge);
        if (globalData::is_put_mouseCharge) {
            ImGui::Text(u8"电荷量: %g C", globalData::mouseCharge_quantity);
            ImGui::SliderFloat(u8"##mouseCharge_quantity_slider", &globalData::mouseCharge_quantity, -1e-2, 1e-2);
        }

        ImGui::Checkbox(u8"显示电势叠加层", &globalData::is_show_potential_layer);
        if (globalData::is_show_potential_layer) {
            ImGui::Indent(20.0f);

            ImGui::Text(u8"叠加层透明度(alpha = 0 全透)");
            ImGui::SliderFloat(u8"##Vimage_alpha_slider", &globalData::potential_layer_alpha, 0.0f, 1.0f);
            ImGui::Text(u8"灰度图伽马值((V - V_min / (V_max - V_min) )^gamma)");
            ImGui::SliderFloat(u8"##V_slider", &globalData::V_Heatmap_gamma, 0.1f, 4.0f);
            ImGui::Text(u8"灰度图颜色映射范围 Vmax = %0.1f ([-Vmax,Vmax])", globalData::V_Heatmap_Vmax);
            ImGui::SliderFloat(u8"##Vmax_slider", &globalData::V_Heatmap_Vmax, 1.0f, 5000000.0f);
        
            ImGui::Checkbox(u8"显示等势线", &globalData::is_show_potential_line);
            if (globalData::is_show_potential_line) {
                ImGui::Indent(20.0f);

                ImGui::Checkbox(u8"显示多个等势线", &globalData::is_show_multi_potential_line);
                if (globalData::is_show_multi_potential_line) {
                    globalData::V_equalV_value = 0.0f;

                    ImGui::Text(u8"等势线数量");
                    ImGui::SliderInt(u8"##equalV_line_num_slider", &globalData::potential_line_num, 1, 100);
                }
                else
                {
                    ImGui::Text(u8"等势线电势值");
                    ImGui::SliderFloat(u8"##equalV_value_slider", &globalData::V_equalV_value, -5000000.0f, 5000000.0f);
                }
                static float zero_line_color_f[3] = { 1.0f, 0.0f, 0.0f };
                if (ImGui::ColorEdit3(u8"等势线颜色", zero_line_color_f)) {
                    globalData::zero_line_color = sf::Color(
                        static_cast<uint8_t>(zero_line_color_f[0] * 255.0f),
                        static_cast<uint8_t>(zero_line_color_f[1] * 255.0f),
                        static_cast<uint8_t>(zero_line_color_f[2] * 255.0f)
                    );
                }

                ImGui::Unindent(20.0f);
            }

            ImGui::Unindent(20.0f);
        }

		ImGui::Checkbox(u8"分布式场强矢量线", &globalData::is_show_electric_field_vector);
		if (globalData::is_show_electric_field_vector) {
            ImGui::Indent(20.0f);

            ImGui::Text(u8"测量点间距");
			ImGui::SliderFloat(u8"##slider2", &globalData::E_line_spacing, 5.0f, 100.0f);
			ImGui::Text(u8"线长衰减指数(E / |E|^power)");
			ImGui::SliderFloat(u8"##slider3", &globalData::E_line_power, 0.1f, 2.0f);
			ImGui::Text(u8"测量点指示圆半径");
			ImGui::SliderFloat(u8"##slider4", &globalData::E_line_circleR, 0.5f, 3.0f);
		    
            ImGui::Unindent(20.0f);
        }

		ImGui::End();
	}
}