#pragma once

namespace globalData {
	struct Charge {
		int id;
		bool available = true;
		sf::Vector2f position;
		float quantity;
	};
	struct FieldPoint {
		sf::Vector2f electricField = sf::Vector2f(0.0f, 0.0f);
		float potential = 0.0f;
	};

	Canvas* mainCanvas_ptr = nullptr;
	Canvas* electricPotentialCanvas_ptr = nullptr;
	std::vector<Charge>* charges = nullptr;

	std::unique_ptr<ThreadPool> g_threadPool;

	sf::Vector2f mousePos_mainCanvas_imgui = sf::Vector2f(0, 0);

	int target_numThreads = 0;
	int max_numThreads = 0;

	float E_Heatmap_gamma = 0.3f;
	float E_Heatmap_Emax = 5000000.0f;

	bool is_put_mouseCharge = false;
	float mouseCharge_quantity = -1e-4;

	bool is_show_electric_field_vector = false;
	float E_line_power = 0.8f;
	float E_line_spacing = 50.0f;
	float E_line_circleR = 1.5f;

	bool is_show_coordinateAxis = false;

	bool is_show_potential_layer = false;
	float potential_layer_alpha = 0.5f;
	float V_Heatmap_gamma = 0.3f;
	float V_Heatmap_Vmax = 5000000.0f;
	bool is_show_potential_line = false;
	float V_equalV_value = 0.0f;
	sf::Color zero_line_color = sf::Color::Red;
	bool is_show_multi_potential_line = false;
	int potential_line_num = 10;
}