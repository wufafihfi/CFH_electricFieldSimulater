#pragma once

/*
* 下一步：
* 数据叠加层 完成进度:%50
* 程序内重置模拟器，修改模拟环境大小
* 添加和控制自定义电荷(LUA脚本和JSON场景保存)
* 显示电荷信息
* 能保存模拟图像并附带数据说明
* 性能尽可能优化的更好（每一步都要做到）
*/

namespace simulater_main {
	// 模拟环境区块 行优先
	struct Block {
		unsigned int startRow;
		unsigned int endRow;
		unsigned int startCol;
		unsigned int endCol;
	};

	sf::RenderWindow* window;
	M_Property::PropertyContainer* properties;

	Canvas mainCanvas;
	SimpleDraw mainDraw;

	Canvas electricPotentialCanvas;
	SimpleDraw electricPotentialDraw;

	// simulater data
	constexpr double kCoulomb = 8.9875517923e9;

	sf::Vector2u gridSize;

	int chargeID = 0;
	std::vector<globalData::Charge> charges;
	std::vector<std::vector<globalData::FieldPoint>> fieldData;

	// 多线程
	std::atomic<int> completedBlocks{ 0 };
	int totalBlocks = 0;

	inline globalData::FieldPoint computeFieldAndPotential(const globalData::Charge& charge, const sf::Vector2f& point) {
		sf::Vector2f delta = point - charge.position;
		float rSquared = delta.lengthSquared();

		if (rSquared < 1e-10f) {
			return { sf::Vector2f(0.0f, 0.0f), 0.0f };
		}

		float r = std::sqrt(rSquared);
		float invR = 1.0f / r;
		float invR2 = invR * invR;
		float invR3 = invR2 * invR;

		float kQ = static_cast<float>(kCoulomb) * charge.quantity;

		globalData::FieldPoint result;
		result.electricField = kQ * invR3 * delta;  // kQ/r² * (delta/r)
		result.potential = kQ * invR;               // kQ/r
		return result;
	}

	// x = col, y = row
	void computeBlock(
		const Block& block,
		const std::vector<globalData::Charge>& chargesRef,
		std::vector<std::vector<globalData::FieldPoint>>& _fieldData
	) {
		for (int row = block.startRow; row < block.endRow; row++) {
			for (int col = block.startCol; col < block.endCol; col++) {
				globalData::FieldPoint pData = { sf::Vector2f(0,0),0.0f };
				sf::Vector2f point(col, row);

				for (const auto& charge : chargesRef) {
					globalData::FieldPoint result = computeFieldAndPotential(charge, point);
					pData.electricField += result.electricField;
					pData.potential += result.potential;
				}

				_fieldData[row][col] = pData;
			}
		}
	}

	void makeFieldValue(sf::Vector2u _gridSize, int target_numThreads = 10) {
		if (fieldData.size() != _gridSize.y ||
			fieldData.empty() ||
			fieldData[0].size() != _gridSize.x) {
			fieldData.resize(_gridSize.y,
				std::vector<globalData::FieldPoint>(_gridSize.x));
		}

		std::vector<Block> blocks;
		unsigned int blockRows = _gridSize.y / target_numThreads;

		for (int i = 0; i < target_numThreads; ++i) {
			Block block;
			block.startCol = 0;
			block.endCol = _gridSize.x;
			block.startRow = i * blockRows;
			block.endRow = (i == target_numThreads - 1) ? _gridSize.y : (i + 1) * blockRows;
			blocks.push_back(block);
		}

		std::vector<std::future<void>> futures;
		for (const auto& block : blocks) {
			futures.push_back(globalData::g_threadPool->enqueue([&, block]() {
				computeBlock(block, charges, fieldData);
				}));
		}
		
		for (auto& f : futures) {
			f.wait();
		}
	}

	void limitThreadNum(int& target_numThreads, bool initMode = false) {
		globalData::max_numThreads = static_cast<int>(std::thread::hardware_concurrency());

		if (target_numThreads < 3) {
			target_numThreads = 3;
		}
		if ((target_numThreads > globalData::max_numThreads)|| initMode) {
			target_numThreads = globalData::max_numThreads;
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

		electricPotentialCanvas.create(gridSize.x, gridSize.y);
		electricPotentialDraw.setSimpleDraw(electricPotentialCanvas);

		if (fieldData.size() != gridSize.y ||
			fieldData.empty() ||
			fieldData[0].size() != gridSize.x) {
			fieldData.resize(gridSize.y,
				std::vector<globalData::FieldPoint>(gridSize.x));
		}

		// 线程池初始化
		limitThreadNum(globalData::target_numThreads,true);
		globalData::g_threadPool = std::make_unique<ThreadPool>(globalData::target_numThreads);

		// 全局数据引用
		globalData::charges = &charges;
		globalData::mainCanvas_ptr = &mainCanvas;
		globalData::electricPotentialCanvas_ptr = &electricPotentialCanvas;

		globalData::Charge charge_1 = {
			chargeID,
			{ 
				static_cast<float>(gridSize.x) / 2.0f,
				static_cast<float>(gridSize.y) / 2.0f
			},
			-1e-4
		};
		chargeID++;
		charges.push_back(charge_1);
		globalData::Charge charge_2 = {
			chargeID,
			{
				static_cast<float>(gridSize.x) / 3.0f,
				static_cast<float>(gridSize.y) / 3.6f
			},
			1e-3
		};
		chargeID++;
		//charges.push_back(charge_2);
		globalData::Charge charge_3 = {
			chargeID,
			{
				static_cast<float>(gridSize.x) / 3.0f,
				static_cast<float>(gridSize.y) / 1.3f,
			},
			1e-3
		};
		chargeID++;
		//charges.push_back(charge_3);
		int R = 100;
		for (double i = 0; i < 2*M_PI ; i += 2 * M_PI / 20) {
			sf::Vector2f offset = sf::Vector2f(R*cos(i),R*sin(i));
			globalData::Charge charge_f = {
				chargeID,
				{
					static_cast<float>(gridSize.x) / 2.0f + offset.x,
					static_cast<float>(gridSize.y) / 2.0f + offset.y,
				},
				-6e-4
			};
			chargeID++;
			charges.push_back(charge_f);
		}
	}

	void Render() {
		limitThreadNum(globalData::target_numThreads);
		mainCanvas.clear();
		electricPotentialCanvas.clear();

		// simulaterCanvas
		for (auto& charge : charges) {
			if (charge.id == 0) {
				charge.position = globalData::mousePos_mainCanvas_imgui;
			}
		}

		makeFieldValue(gridSize, globalData::target_numThreads);

		mainDraw.generateHeatmapImage(
			fieldData,
			gridSize,
			0, globalData::E_Heatmap_Emax,
			globalData::E_Heatmap_gamma,
			globalData::target_numThreads,
			[](float n) { return mainDraw.vibrantColorMap(n); },
			false
		);
		mainDraw.drawTexture(mainDraw.m_heatmapTexture, 0.0f, 0.0f);

		electricPotentialDraw.generateHeatmapImage(
			fieldData,
			gridSize,
			globalData::V_Heatmap_Vmax * -1, globalData::V_Heatmap_Vmax,
			globalData::V_Heatmap_gamma,
			globalData::target_numThreads,
			[](float n) { return electricPotentialDraw.potentialColorMap(n); },
			true
		);
		electricPotentialDraw.drawTexture(electricPotentialDraw.m_heatmapTexture, 0.0f, 0.0f);

		if(globalData::is_show_electric_field_vector)
		{
			mainDraw.electricFieldVector(
				fieldData,
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

		mainCanvas.getTexture().display();
		electricPotentialCanvas.getTexture().display();
	}
}