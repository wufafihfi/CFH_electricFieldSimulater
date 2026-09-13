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

	void electricFieldSimulater() {
		ImGui::Begin(u8"电场模拟热力图",nullptr, ImGuiWindowFlags_NoScrollbar);
		
		// FPS
		static sf::Clock clock;
		static float deltaTime = clock.restart().asSeconds();
		float fps = 1.0f / deltaTime;
		deltaTime = clock.restart().asSeconds();
		ImGui::Text(u8"帧率: %.1f FPS", fps);

		sf::Vector2f canvasSize = globalData::mainCanvas_ptr->getSize();
		ImVec2 available = ImGui::GetContentRegionAvail();

		// 保持宽高比，自适应缩放
		float scale = std::min(available.x / canvasSize.x, available.y / canvasSize.y);
		sf::Vector2f displaySize(canvasSize.x * scale, canvasSize.y * scale);

		ImVec2 imagePos = ImGui::GetCursorScreenPos();

		ImGui::Image(globalData::mainCanvas_ptr->getTexture(),
			displaySize,
			sf::Color::White,
			sf::Color::Transparent);

		ImVec2 mousePos = ImGui::GetMousePos();

		float relX = mousePos.x - imagePos.x;
		float relY = mousePos.y - imagePos.y;

		float canvasX = relX / scale;
		float canvasY = relY / scale;

		globalData::mousePos_mainCanvas_imgui = sf::Vector2f(canvasX, canvasY);

		ImGui::End();
	}

	void simulaterSetting() {
		ImGui::Begin(u8"电场模拟参数调整");

		ImGui::Text(u8"热力图伽马值((E / E_max )^gama)");
		ImGui::SliderFloat(u8"##slider1", &globalData::E_Heatmap_gama, 0.1f, 2.0f);

		ImGui::Checkbox(u8"显示坐标轴", &globalData::is_show_coordinateAxis);

		ImGui::Checkbox(u8"分布式场强矢量线", &globalData::is_show_electric_field_vector);
		if (globalData::is_show_electric_field_vector) {
			ImGui::Text(u8"测量点间距");
			ImGui::SliderFloat(u8"##slider2", &globalData::E_line_spacing, 5.0f, 100.0f);
			ImGui::Text(u8"线长衰减指数(E / |E|^power)");
			ImGui::SliderFloat(u8"##slider3", &globalData::E_line_power, 0.1f, 2.0f);
			ImGui::Text(u8"测量点指示圆半径");
			ImGui::SliderFloat(u8"##slider4", &globalData::E_line_circleR, 0.5f, 3.0f);
		}



		ImGui::End();
	}
}