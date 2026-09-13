#pragma once

/*
* 下一步：
* 程序内重置模拟器，修改模拟环境大小
* 添加和控制自定义电荷(LUA脚本和JSON场景保存)
* 显示电荷信息
* 能保存模拟图像并附带数据说明
* 性能尽可能优化的更好
*/

namespace simulater_main {
	struct Charge {
		int id;
		sf::Vector2f position;
		float quanity;
	};
	// 计算区块
	struct Block {
		unsigned int startX;
		unsigned int endX;
		unsigned int startY;
		unsigned int endY;
	};

	sf::RenderWindow* window;
	M_Property::PropertyContainer* properties;

	Canvas mainCanvas;
	SimpleDraw mainDraw;
	RatioView mainView;

	// simulater data
	constexpr double kCoulomb = 8.9875517923e9;

	sf::Vector2u gridSize;

	int chargeID = 0;
	std::vector<Charge> charges;
	std::vector <std::vector<sf::Vector2f>> fieldMagnitude;

	// 多线程
	std::atomic<int> completedBlocks{ 0 };
	int totalBlocks = 0;

	sf::Vector2f electricFieldVector(Charge charge,sf::Vector2f point) {
		sf::Vector2f distance = point - charge.position;
		float rSquared = distance.lengthSquared();

		float magnitude = static_cast<float>(kCoulomb) * charge.quanity / rSquared;
		return magnitude * distance.normalized();
	}

	void computeBlock(
		const Block& block, 
		const std::vector<Charge>& chargesRef,
		std::vector<std::vector<sf::Vector2f>>& fieldData
	) {
		for (int X = block.startX; X < block.endX; X++) {
			for (int Y = block.startY; Y < block.endY; Y++) {
				sf::Vector2f total_E = sf::Vector2f(0, 0);
				sf::Vector2f point = sf::Vector2f(X,Y);
				for (const auto& charge : chargesRef) {
					total_E = total_E + electricFieldVector(charge, point);
				}
				fieldData[X][Y] = total_E;
			}
		}
	}

	void makeFieldMagnitude(sf::Vector2u _gridSize, int target_numThreads = 10) {
		int numThreads = 1;
		if (numThreads == 0) {
			numThreads = static_cast<int>(std::thread::hardware_concurrency());
			if (numThreads >= target_numThreads && !(target_numThreads < 1)) {
				numThreads = target_numThreads;
			}
			if (numThreads == 0) {
				numThreads = 4;
			}
		}
		
		fieldMagnitude.clear();
		fieldMagnitude.resize(_gridSize.x, std::vector<sf::Vector2f>(_gridSize.y, sf::Vector2f(0,0)));

		std::vector<Block> blocks;
		unsigned int blockRows = _gridSize.y / numThreads;

		for (int i = 0; i < numThreads; ++i) {
			Block block;
			block.startX = 0;
			block.endX = _gridSize.x;
			block.startY = i * blockRows;
			block.endY = (i == numThreads - 1) ? _gridSize.y : (i + 1) * blockRows;
			blocks.push_back(block);
		}

		std::vector<std::thread> threads;
		for (const auto& block : blocks) {
			threads.emplace_back(computeBlock, block, std::cref(charges), std::ref(fieldMagnitude));
		}

		for (auto& t : threads) {
			if (t.joinable()) {
				t.join();
			}
		}
	}

	static sf::Vector2f GetCanvasPosition(const sf::Vector2f& worldPos, Canvas* canvas) {
		return worldPos - canvas->getPosition() + canvas->getSize() * 0.5f;
	}

	void Init(
		sf::RenderWindow* _window,
		M_Property::PropertyContainer* _properties
	) {
		window = _window;
		properties = _properties;

		gridSize = sf::Vector2u(500, 500);
	
		sf::Vector2 window_size = window->getSize();
		mainCanvas.create(gridSize.x, gridSize.y);
		mainDraw.setSimpleDraw(mainCanvas);
		mainView.setRatioView(mainCanvas.getSize());

		// 全局数据引用
		globalData::mainCanvas_ptr = &mainCanvas;

		Charge charge_1 = { 
			chargeID,
			{ 
				static_cast<float>(gridSize.x) / 2.0f,
				static_cast<float>(gridSize.y) / 2.0f
			},
			-1e-4
		};
		chargeID++;
		charges.push_back(charge_1);
		Charge charge_2 = {
			chargeID,
			{
				static_cast<float>(gridSize.x) / 3.0f,
				static_cast<float>(gridSize.y) / 3.0f
			},
			1e-3
		};
		chargeID++;
		charges.push_back(charge_2);
		Charge charge_3 = {
			chargeID,
			{
				static_cast<float>(gridSize.x) / 3.0f,
				static_cast<float>(gridSize.y) / 1.5f,
			},
			1e-3
		};
		chargeID++;
		charges.push_back(charge_3);
		int R = 100;
		for (double i = 0; i < 2*M_PI ; i += 2 * M_PI / 20) {
			sf::Vector2f offset = sf::Vector2f(R*cos(i),R*sin(i));
			Charge charge_f = {
				chargeID,
				{
					static_cast<float>(gridSize.x) / 2.0f + offset.x,
					static_cast<float>(gridSize.y) / 2.0f + offset.y,
				},
				-6e-4
			};
			chargeID++;
			//charges.push_back(charge_f);
		}
	}

	//sf::Vector2f mousePos_mainCanvas = sf::Vector2f(0, 0);
	void Render() {
		//mousePos_mainCanvas = GetCanvasPosition(window->mapPixelToCoords(sf::Mouse::getPosition(*window)), &mainCanvas);

		mainCanvas.clear();

		// simulaterCanvas
		for (auto& charge : charges) {
			if (charge.id == 0) {
				charge.position = globalData::mousePos_mainCanvas_imgui;
			}
		}

		makeFieldMagnitude(gridSize,0);
		sf::Image heatmapImage = mainDraw.generateHeatmapImage(
			fieldMagnitude, 
			gridSize,
			5000000,
			globalData::E_Heatmap_gama);
		mainDraw.drawImage(heatmapImage, 0.0f, 0.0f);

		if(globalData::is_show_electric_field_vector)
		{
			mainDraw.electricFieldVector(
				fieldMagnitude,
				gridSize,
				globalData::E_line_power,
				globalData::E_line_spacing,
				globalData::E_line_circleR
			);
		}

		if(globalData::is_show_coordinateAxis)
		{
			mainDraw.setColor(0, 255, 0);
			mainDraw.line(gridSize.x / 2, gridSize.y / 2, gridSize.x, gridSize.y / 2);
			mainDraw.setColor(255, 0, 0);
			mainDraw.line(gridSize.x / 2, gridSize.y / 2, gridSize.x / 2, gridSize.y);
		}

		//mainView.applyTo(*window);

		//mainCanvas.render(*window);
	}
}