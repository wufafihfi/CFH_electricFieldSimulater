// SimpleDraw.h
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <algorithm> 

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//###############学习#################
// 阅 - 5遍
//####################################

class Canvas;

class SimpleDraw {
private:
    sf::RenderTarget* target;  // 可以是窗口或纹理
    sf::Color currentColor = sf::Color::White;
    float currentThickness = 1.0f;
    bool ownsTarget = false;   // 是否拥有目标的所有权

public:
    // 绑定到窗口
    void setSimpleDraw(sf::RenderWindow* _window) {
        target = _window;
    }

    // 绑定到画布 (离屏缓冲区)
    void setSimpleDraw(Canvas& canvas){
        target = &canvas.getTexture();
    }

    // 设置绘图目标
    void setTarget(sf::RenderTarget& newTarget) {
        target = &newTarget;
    }

    // 获取当前目标
    sf::RenderTarget* getTarget() { return target; }

    // 颜色设置
    void setColor(sf::Color color) { currentColor = color; }
    void setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
        currentColor = sf::Color(r, g, b, a);
    }

    // 线条粗细
    void setThickness(float thickness) { currentThickness = thickness; }

    // ========== 基础图形绘制 ==========
    void clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
        target->clear(sf::Color(r, g, b, a));
    }

    void putPixel(float x, float y) {
        sf::Vertex point{ sf::Vector2f(x, y), currentColor };
        target->draw(&point, 1, sf::PrimitiveType::Points);
    }

    void line(float x1, float y1, float x2, float y2) {
        sf::Vertex vertices[] = {
            sf::Vertex{sf::Vector2f(x1, y1), currentColor},
            sf::Vertex{sf::Vector2f(x2, y2), currentColor}
        };
        target->draw(vertices, 2, sf::PrimitiveType::Lines);
    }

    void fillRectangle(float x, float y, float width, float height) {
        sf::RectangleShape rect(sf::Vector2f(width, height));
        rect.setPosition({ x, y });
        rect.setFillColor(currentColor);
        target->draw(rect);
    }

    void rectangle(float x, float y, float width, float height) {
        float thickness = currentThickness > 1.0f ? currentThickness : 1.0f;

        // 上
        fillRectangle(x, y, width, thickness);
        // 下
        fillRectangle(x, y + height - thickness, width, thickness);
        // 左
        fillRectangle(x, y, thickness, height);
        // 右
        fillRectangle(x + width - thickness, y, thickness, height);
    }

    void circle(float cx, float cy, float radius) {
        const int segments = 100;
        std::vector<sf::Vertex> vertices;
        vertices.reserve(segments + 1);

        for (int i = 0; i <= segments; i++) {
            float angle = 2 * M_PI * i / segments;
            float x = cx + radius * std::cos(angle);
            float y = cy + radius * std::sin(angle);
            vertices.push_back({ sf::Vector2f(x, y), currentColor });
        }

        target->draw(vertices.data(), vertices.size(), sf::PrimitiveType::LineStrip);
    }

    void fillCircle(float cx, float cy, float radius) {
        sf::CircleShape circle(radius);
        circle.setPosition({ cx - radius, cy - radius });
        circle.setFillColor(currentColor);
        circle.setPointCount(10);
        target->draw(circle);
    }
    void fillCircle(float cx, float cy, float radius, int points) {
        sf::CircleShape circle(radius);
        circle.setPosition({ cx - radius, cy - radius });
        circle.setFillColor(currentColor);
        circle.setPointCount(points);
        target->draw(circle);
    }

    void fillTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
        sf::ConvexShape triangle;
        triangle.setPointCount(3);
        triangle.setPoint(0, sf::Vector2f(x1, y1));
        triangle.setPoint(1, sf::Vector2f(x2, y2));
        triangle.setPoint(2, sf::Vector2f(x3, y3));
        triangle.setFillColor(currentColor);
        target->draw(triangle);
    }

    // 进阶
    // 绘制粗线条（使用旋转矩形）
    void thickLine(float x1, float y1, float x2, float y2, float thickness) {
        sf::Vector2f point1(x1, y1);
        sf::Vector2f point2(x2, y2);
        sf::Vector2f direction = point2 - point1;
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        if (length < 0.0001f) return;  // 避免除零

        sf::RectangleShape line(sf::Vector2f(length, thickness));
        line.setFillColor(currentColor);
        line.setPosition(point1);
        line.setRotation(sf::radians(std::atan2(direction.y, direction.x)));
        line.setOrigin({ 0, thickness / 2.0f });

        target->draw(line);
    }

    // 使用内部设置好的粗细绘制线条
    void thickLine(float x1, float y1, float x2, float y2) {
        thickLine(x1, y1, x2, y2, currentThickness);
    }

    // 绘制圆角粗线条（使用圆形端点）
    void roundedThickLine(float x1, float y1, float x2, float y2, float thickness) {
        sf::Vector2f point1(x1, y1);
        sf::Vector2f point2(x2, y2);
        sf::Vector2f direction = point2 - point1;
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        thickLine(x1, y1, x2, y2, thickness);
        fillCircle(x1, y1, thickness / 2.0f, thickness * 10);
        fillCircle(x2, y2, thickness / 2.0f, thickness * 10);
    }

    // 使用当前粗细的圆角线条
    void roundedThickLine(float x1, float y1, float x2, float y2) {
        roundedThickLine(x1, y1, x2, y2, currentThickness);
    }

    // 批量绘制辅助方法
    void addTriangle(const sf::Vector2f& a, const sf::Vector2f& b,
        const sf::Vector2f& c, const sf::Color& color,
        std::vector<sf::Vertex>& vertices) {
        vertices.push_back(sf::Vertex({ a, color }));
        vertices.push_back(sf::Vertex({ b, color }));
        vertices.push_back(sf::Vertex({ c, color }));
    }

    void addCircleToVertices(const sf::Vector2f& center, float radius,
        const sf::Color& color, int segments,
        std::vector<sf::Vertex>& vertices) {
        // 确保至少3个段
        segments = segments > 3.0f ? segments : 3.0f;;

        for (int i = 0; i < segments; i++) {
            float angle1 = 2 * M_PI * i / segments;
            float angle2 = 2 * M_PI * (i + 1) / segments;

            sf::Vector2f p1(center.x + radius * std::cos(angle1),
                center.y + radius * std::sin(angle1));
            sf::Vector2f p2(center.x + radius * std::cos(angle2),
                center.y + radius * std::sin(angle2));

            addTriangle(center, p1, p2, color, vertices);
        }
    }

public:
    // 批量绘制圆角粗线条（添加到顶点数组）
    void roundedThickLineToVertices(float x1, float y1, float x2, float y2,
        float thickness, const sf::Color& color,
        std::vector<sf::Vertex>& vertices,
        int circleSegments = 16,  // 默认提高精度
        int lineSegments = 1) {   // 线条分段数（用于曲线效果）

        sf::Vector2f point1(x1, y1);
        sf::Vector2f point2(x2, y2);
        sf::Vector2f direction = point2 - point1;
        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        if (length < 0.0001f) {
            // 距离太短，直接画圆
            addCircleToVertices(point1, thickness / 2.0f, color, circleSegments, vertices);
            return;
        }

        // 对于长线条，可以分段绘制（提高弯曲精度）
        if (lineSegments > 1) {
            for (int i = 0; i < lineSegments; i++) {
                float t1 = (float)i / lineSegments;
                float t2 = (float)(i + 1) / lineSegments;
                sf::Vector2f p1 = point1 + direction * t1;
                sf::Vector2f p2 = point1 + direction * t2;
                roundedThickLineToVertices(p1.x, p1.y, p2.x, p2.y,
                    thickness, color, vertices,
                    circleSegments, 1);
            }
            return;
        }

        // 计算矩形四个角点
        sf::Vector2f perpendicular(-direction.y / length, direction.x / length);
        sf::Vector2f halfThick = perpendicular * (thickness / 2.0f);

        sf::Vector2f p1 = point1 - halfThick;
        sf::Vector2f p2 = point2 - halfThick;
        sf::Vector2f p3 = point2 + halfThick;
        sf::Vector2f p4 = point1 + halfThick;

        // 矩形（两个三角形）
        addTriangle(p1, p2, p3, color, vertices);
        addTriangle(p1, p3, p4, color, vertices);

        // 可选：添加矩形边缘抗锯齿（增加三角形细分）
        if (circleSegments >= 12) {
            // 添加四个角的圆角过渡
            addCornerTransition(p1, p2, point1, perpendicular, halfThick, color, vertices);
            addCornerTransition(p2, p3, point2, perpendicular, halfThick, color, vertices);
            addCornerTransition(p3, p4, point2, -perpendicular, halfThick, color, vertices);
            addCornerTransition(p4, p1, point1, -perpendicular, halfThick, color, vertices);
        }

        // 两端圆点（使用更高的精度）
        this->addCircleToVertices(point1, thickness / 2.0f, color, circleSegments, vertices);
        this->addCircleToVertices(point2, thickness / 2.0f, color, circleSegments, vertices);
    }

    // 批量绘制所有线条
    void drawLinesFromVertices(const std::vector<sf::Vertex>& vertices) {
        if (!vertices.empty()) {
            target->draw(vertices.data(), vertices.size(), sf::PrimitiveType::Triangles);
        }
    }

private:
    // 添加圆角过渡（使矩形和圆点连接更平滑）
    void addCornerTransition(const sf::Vector2f& corner1, const sf::Vector2f& corner2,
        const sf::Vector2f& center, const sf::Vector2f& direction,
        const sf::Vector2f& halfThick, const sf::Color& color,
        std::vector<sf::Vertex>& vertices) {
        // 简单的过渡：添加额外的三角形
        addTriangle(corner1, corner2, center + direction * halfThick.length(), color, vertices);
    }

public:
    sf::Image m_heatmapImage;
    sf::Texture m_heatmapTexture;
    // 热力图绘制 学阅6
    void generateHeatmapImage(
        const std::vector<std::vector<globalData::FieldPoint>>& _FieldData,
        const sf::Vector2u& imageSize,
        float minValue,
        float maxValue,
        float power,
        int numThreads,
        std::function<sf::Color(float)> colorMapper,
        bool usePotential = false
    ) {
        // 分配或复用
        if (m_heatmapImage.getSize() != sf::Vector2u(imageSize.x, imageSize.y)) {
            m_heatmapImage = sf::Image({ imageSize.x, imageSize.y }, sf::Color::Black);
            if (!m_heatmapTexture.resize({ imageSize.x, imageSize.y })) {
                return;
            }
        }

        // 范围保护
        float range = maxValue - minValue;
        if (range < 1e-10f) range = 1.0f;

        // 分块
        unsigned int blockRows = imageSize.y / numThreads;
        if (blockRows < 1) blockRows = 1;

        std::vector<std::future<void>> futures;
        for (int i = 0; i < numThreads; ++i) {
            unsigned int startRow = i * blockRows;
            unsigned int endRow = (i == numThreads - 1) ? imageSize.y : (i + 1) * blockRows;
            if (startRow >= imageSize.y) break;

            futures.push_back(globalData::g_threadPool->enqueue(
                [&, startRow, endRow, colorMapper, usePotential, minValue, range, power]() {
                    for (unsigned int row = startRow; row < endRow; ++row) {
                        for (unsigned int col = 0; col < imageSize.x; ++col) {
                            float value = usePotential
                                ? _FieldData[row][col].potential
                                : _FieldData[row][col].electricField.length();

                            // 归一化到 [0, 1]
                            value = std::clamp(value, minValue, maxValue);
                            float normalized = (value - minValue) / range;

                            // 非线性变换
                            if (power != 1.0f) {
                                normalized = std::pow(normalized, power);
                            }

                            normalized = std::clamp(normalized, 0.0f, 1.0f);
                            m_heatmapImage.setPixel({ col, row }, colorMapper(normalized));
                        }
                    }
                }));
        }

        for (auto& f : futures) f.wait();

        m_heatmapTexture.update(m_heatmapImage);
    }
    // 颜色映射
    static sf::Color vibrantColorMap(float normalized) {
        static const sf::Color colors[] = {
            {0, 0, 50},       // 深蓝
            {0, 0, 255},      // 蓝
            {0, 255, 255},    // 青
            {0, 255, 128},    // 青绿
            {255, 255, 0},    // 黄
            {255, 128, 0},    // 橙
            {255, 0, 0},      // 红
            {128, 0, 0}       // 深红
        };
        const int numColors = sizeof(colors) / sizeof(colors[0]);
        return mapColor(normalized, colors, numColors);
    }
    static sf::Color potentialColorMap(float normalized) {
        if (normalized < 0.5f) {
            // 负电势
            float t = normalized / 0.5f;
            uint8_t v = static_cast<uint8_t>(t * 128.0f);  // 0 → 128
            return sf::Color(v, v, v);
        }
        else {
            // 正电势
            float t = (normalized - 0.5f) / 0.5f;
            uint8_t v = static_cast<uint8_t>(128 + t * 127.0f);  // 128 → 255
            return sf::Color(v, v, v);
        }
    }
    static sf::Color mapColor(float normalized, const sf::Color* colors, int numColors) {
        normalized = std::clamp(normalized, 0.0f, 1.0f);

        if (numColors < 2) {
            return colors[0];
        }

        float segment = 1.0f / (numColors - 1);
        int idx = static_cast<int>(normalized / segment);
        idx = std::clamp(idx, 0, numColors - 2);

        float t = (normalized - idx * segment) / segment;
        t = std::clamp(t, 0.0f, 1.0f);

        uint8_t r = static_cast<uint8_t>(colors[idx].r + t * (colors[idx + 1].r - colors[idx].r));
        uint8_t g = static_cast<uint8_t>(colors[idx].g + t * (colors[idx + 1].g - colors[idx].g));
        uint8_t b = static_cast<uint8_t>(colors[idx].b + t * (colors[idx + 1].b - colors[idx].b));

        return sf::Color(r, g, b);
    }

    // 分布式场强指示线绘制
    void electricFieldVector(
        const std::vector<std::vector<globalData::FieldPoint>>& _FieldData,
        const sf::Vector2u& imageSize,
        float power = 0.6,
        float spacing = 50.0f,
        float circleR = 1.5f,
        sf::Color lineColor = sf::Color::White,
        sf::Color pointColor = sf::Color::Red
    ) {
        unsigned int cols = static_cast<unsigned int>(imageSize.x / spacing);
        unsigned int rows = static_cast<unsigned int>(imageSize.y / spacing);

        for (unsigned int row = 1; row < rows; ++row) {
            for (unsigned int col = 1; col < cols; ++col) {
                unsigned int x = col * spacing;
                unsigned int y = row * spacing;

                sf::Vector2f E = _FieldData[y][x].electricField;
                float len = E.length();
                if (len > 1e-10f) {
                    E = E / std::pow(len, power);
                }
                else {
                    E = sf::Vector2f(0.0f, 0.0f);
                }

                setColor(pointColor);
                fillCircle(x, y, circleR);
                setColor(lineColor);
                line(x, y, x + E.x, y + E.y);
            }
        }
    }

    void extractZeroPotentialLines(
        const std::vector<std::vector<globalData::FieldPoint>>& _FieldData,
        const sf::Vector2u& imageSize,
        float zeroValue,
        std::vector<std::pair<sf::Vector2f, sf::Vector2f>>& segments
    ) {
        segments.clear();

        for (unsigned int row = 0; row < imageSize.y - 1; ++row) {
            for (unsigned int col = 0; col < imageSize.x - 1; ++col) {
                // 4 个角的电势
                float v00 = _FieldData[row][col].potential - zeroValue;  // 左上
                float v10 = _FieldData[row][col + 1].potential - zeroValue;  // 右上
                float v01 = _FieldData[row + 1][col].potential - zeroValue;  // 左下
                float v11 = _FieldData[row + 1][col + 1].potential - zeroValue;  // 右下

                // 符号判断
                bool s00 = v00 > 0.0f;
                bool s10 = v10 > 0.0f;
                bool s01 = v01 > 0.0f;
                bool s11 = v11 > 0.0f;

                // 如果全同号，跳过
                if (s00 == s10 && s10 == s01 && s01 == s11) continue;

                // 插值函数：在 v1 和 v2 之间找 V = 0 的位置
                auto interp = [](float v1, float v2, float p1, float p2) -> float {
                    if (std::abs(v2 - v1) < 1e-10f) return (p1 + p2) * 0.5f;
                    float t = v1 / (v1 - v2);
                    return p1 + t * (p2 - p1);
                    };

                // 4 条边上的交点
                bool hasTop = (s00 != s10);  // 上边：v00 到 v10
                bool hasBottom = (s01 != s11);  // 下边：v01 到 v11
                bool hasLeft = (s00 != s01);  // 左边：v00 到 v01
                bool hasRight = (s10 != s11);  // 右边：v10 到 v11

                // 计算交点坐标
                float topX = hasTop ? interp(v00, v10, (float)col, (float)(col + 1)) : 0.0f;
                float botX = hasBottom ? interp(v01, v11, (float)col, (float)(col + 1)) : 0.0f;
                float leftY = hasLeft ? interp(v00, v01, (float)row, (float)(row + 1)) : 0.0f;
                float rightY = hasRight ? interp(v10, v11, (float)row, (float)(row + 1)) : 0.0f;

                float topY = (float)row;
                float botY = (float)(row + 1);
                float leftX = (float)col;
                float rightX = (float)(col + 1);

                // 根据符号变化，连接交点
                // 简化版：只处理"两条边有交点"的情况
                int count = hasTop + hasBottom + hasLeft + hasRight;

                if (count == 2) {
                    // 找到两个交点
                    std::vector<sf::Vector2f> points;
                    if (hasTop)    points.emplace_back(topX, topY);
                    if (hasBottom) points.emplace_back(botX, botY);
                    if (hasLeft)   points.emplace_back(leftX, leftY);
                    if (hasRight)  points.emplace_back(rightX, rightY);

                    if (points.size() == 2) {
                        segments.emplace_back(points[0], points[1]);
                    }
                }
                // count == 4 的情况（鞍点）需要特殊处理，这里简化跳过
            }
        }
    }
    // 提取指定行范围内的等势线段
    void extractMultiplePotentialLinesBlock(
        const std::vector<std::vector<globalData::FieldPoint>>& _FieldData,
        const sf::Vector2u& imageSize,
        const std::vector<float>& lineValues,
        unsigned int startRow,
        unsigned int endRow,
        std::vector<std::vector<std::pair<sf::Vector2f, sf::Vector2f>>>& outSegments
    ) {
        int numLines = static_cast<int>(lineValues.size());
        outSegments.resize(numLines);

        for (unsigned int row = startRow; row < endRow; ++row) {
            for (unsigned int col = 0; col < imageSize.x - 1; ++col) {
                float v00 = _FieldData[row][col].potential;
                float v10 = _FieldData[row][col + 1].potential;
                float v01 = _FieldData[row + 1][col].potential;
                float v11 = _FieldData[row + 1][col + 1].potential;

                for (int i = 0; i < numLines; ++i) {
                    float target = lineValues[i];
                    float d00 = v00 - target;
                    float d10 = v10 - target;
                    float d01 = v01 - target;
                    float d11 = v11 - target;

                    bool s00 = d00 > 0;
                    bool s10 = d10 > 0;
                    bool s01 = d01 > 0;
                    bool s11 = d11 > 0;

                    if (s00 == s10 && s10 == s01 && s01 == s11) continue;

                    auto interp = [](float d1, float d2, float p1, float p2) -> float {
                        if (std::abs(d2 - d1) < 1e-10f) return (p1 + p2) * 0.5f;
                        float t = d1 / (d1 - d2);
                        return p1 + t * (p2 - p1);
                        };

                    bool hasTop = (s00 != s10);
                    bool hasBottom = (s01 != s11);
                    bool hasLeft = (s00 != s01);
                    bool hasRight = (s10 != s11);

                    std::vector<sf::Vector2f> points;
                    if (hasTop)    points.emplace_back(interp(d00, d10, (float)col, (float)(col + 1)), (float)row);
                    if (hasBottom) points.emplace_back(interp(d01, d11, (float)col, (float)(col + 1)), (float)(row + 1));
                    if (hasLeft)   points.emplace_back((float)col, interp(d00, d01, (float)row, (float)(row + 1)));
                    if (hasRight)  points.emplace_back((float)(col + 1), interp(d10, d11, (float)row, (float)(row + 1)));

                    if (points.size() == 2) {
                        outSegments[i].emplace_back(points[0], points[1]);
                    }
                    else if (points.size() == 4) {
                        float vCenter = (v00 + v10 + v01 + v11) * 0.25f - target;
                        bool sCenter = vCenter > 0;
                        if (sCenter == s00) {
                            outSegments[i].emplace_back(points[0], points[2]);
                            outSegments[i].emplace_back(points[1], points[3]);
                        }
                        else {
                            outSegments[i].emplace_back(points[0], points[3]);
                            outSegments[i].emplace_back(points[1], points[2]);
                        }
                    }
                }
            }
        }
    }
    void extractMultiplePotentialLines(
        const std::vector<std::vector<globalData::FieldPoint>>& _FieldData,
        const sf::Vector2u& imageSize,
        float minValue, float maxValue,
        int numLines,
        int numThreads,
        std::vector<std::vector<std::pair<sf::Vector2f, sf::Vector2f>>>& allSegments
    ) {
        allSegments.clear();
        allSegments.resize(numLines);

        // 预计算等势线值
        std::vector<float> lineValues(numLines);
        float step = (maxValue - minValue) / (numLines + 1);
        for (int i = 0; i < numLines; ++i) {
            lineValues[i] = minValue + step * (i + 1);
        }

        // 分块
        unsigned int blockRows = (imageSize.y - 1) / numThreads;
        if (blockRows < 1) blockRows = 1;

        // 每个线程的独立输出
        std::vector<std::vector<std::vector<std::pair<sf::Vector2f, sf::Vector2f>>>> threadOutputs(numThreads);

        std::vector<std::future<void>> futures;
        for (int i = 0; i < numThreads; ++i) {
            unsigned int startRow = i * blockRows;
            unsigned int endRow = (i == numThreads - 1) ? (imageSize.y - 1) : (i + 1) * blockRows;
            if (startRow >= imageSize.y - 1) break;

            futures.push_back(globalData::g_threadPool->enqueue(
                [&, i, startRow, endRow]() {
                    extractMultiplePotentialLinesBlock(
                        _FieldData, imageSize, lineValues,
                        startRow, endRow,
                        threadOutputs[i]
                    );
                }));
        }

        for (auto& f : futures) f.wait();

        // 合并所有线程的输出
        for (int t = 0; t < numThreads; ++t) {
            for (int i = 0; i < numLines; ++i) {
                if (i < threadOutputs[t].size()) {
                    allSegments[i].insert(
                        allSegments[i].end(),
                        threadOutputs[t][i].begin(),
                        threadOutputs[t][i].end()
                    );
                }
            }
        }
    }
    void drawZeroPotentialLines(
        const std::vector<std::pair<sf::Vector2f, sf::Vector2f>>& segments,
        sf::Color color = sf::Color::Red
    ) {
        sf::VertexArray lines(sf::PrimitiveType::Lines);

        for (const auto& seg : segments) {
            lines.append(sf::Vertex({ seg.first, color }));
            lines.append(sf::Vertex({ seg.second, color }));
        }

        target->draw(lines);
    }
    void drawMultiplePotentialLines(
        const std::vector<std::vector<std::pair<sf::Vector2f, sf::Vector2f>>>& allSegments,
        sf::Color& color
    ) {
        sf::VertexArray lines(sf::PrimitiveType::Lines);

        for (size_t i = 0; i < allSegments.size(); ++i) {
            for (const auto& seg : allSegments[i]) {
                lines.append(sf::Vertex({ seg.first, color }));
                lines.append(sf::Vertex({ seg.second, color }));
            }
        }

        target->draw(lines);
    }

private:
    sf::Texture m_cachedTexture;
public:
    // 绘制纹理
    void drawTexture(const sf::Texture& texture, float x = 0.0f, float y = 0.0f) {
        sf::Sprite sprite(texture);
        sprite.setPosition({ x, y });
        target->draw(sprite);
    }
    // 绘制图像
    void drawImage(const sf::Image& image, float x = 0.0f, float y = 0.0f) {
        m_cachedTexture.loadFromImage(image);
        sf::Sprite sprite(m_cachedTexture);
        sprite.setPosition({ x, y });
        target->draw(sprite);
    }
};

#include "Canvas.h"