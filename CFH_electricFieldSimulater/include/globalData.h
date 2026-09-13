#pragma once

namespace globalData {
	Canvas* mainCanvas_ptr = nullptr;

	sf::Vector2f mousePos_mainCanvas_imgui = sf::Vector2f(0, 0);

	float E_Heatmap_gama = 0.3f;

	bool is_show_electric_field_vector = false;
	float E_line_power = 0.8f;
	float E_line_spacing = 50.0f;
	float E_line_circleR = 1.5f;

	bool is_show_coordinateAxis = false;
}