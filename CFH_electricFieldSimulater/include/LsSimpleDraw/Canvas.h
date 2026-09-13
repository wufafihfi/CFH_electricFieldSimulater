// Canvas.h
#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <stdexcept>

//###############学习#################
// 阅 - 5遍
//####################################

class Canvas {
private:
    bool hasCreated = false;

    sf::RenderTexture texture;
    std::optional<sf::Sprite> sprite;
    sf::Color clearColor = sf::Color::Black;
    bool autoClear = false;

    sf::Vector2f m_position = { 0, 0 };

public:
    //Canvas(unsigned int width, unsigned int height) {
        //create(width, height);
        //hasCreated = true;
    //}

    // 如果已经调用了构造函数,那么此函数会失效
    void create(unsigned int width, unsigned int height) {
        if (hasCreated == true) {
            return;
        }
        sf::ContextSettings settings;
        settings.antiAliasingLevel = 8;
        if (!texture.resize({ width, height })) {
            throw std::runtime_error("Failed to create canvas texture");
        }
        texture.setSmooth(true);
        sprite.emplace(texture.getTexture());
        sprite->setOrigin(sf::Vector2f(texture.getSize()) * 0.5f);
    }

    // 开始绘图
    void beginDraw() {
        if (autoClear) {
            texture.clear(clearColor);
        }
    }

    // 结束绘图
    void endDraw() {
        texture.display();
    }

    // 绘制到绘制目标
    void render(sf::RenderTarget& target) {
        if (!sprite.has_value()) return;
        sprite->setTexture(texture.getTexture());
        sprite->setPosition(m_position);
        target.draw(*sprite);
    }

    // 获取绘图目标
    sf::RenderTexture& getTexture() { return texture; }
    const sf::RenderTexture& getTexture() const { return texture; }

    // 设置清除颜色
    void setClearColor(sf::Color color) { clearColor = color; }
    void setAutoClear(bool autoClear) { this->autoClear = autoClear; }

    // 获取画布尺寸
    sf::Vector2f getSize() const { return sf::Vector2f(texture.getSize()); }

    // 清除画布
    void clear() {
        texture.clear(clearColor);
    }

    // 保存到文件
    bool saveToFile(const std::string& filename) {
        return texture.getTexture().copyToImage().saveToFile(filename);
    }

    void setPosition(float x, float y) {
        m_position = { x, y };
    }

    void setPosition(const sf::Vector2f& pos) {
        m_position = pos;
    }

    sf::Vector2f getPosition() {
        return m_position;
    }

    bool SaveToFile(const std::string& filename) {
        return texture.getTexture().copyToImage().saveToFile(filename);
    }
};