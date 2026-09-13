// RatioView.h
#pragma once
#include <SFML/Graphics.hpp>

class RatioView {
private:
    sf::View m_view;
    sf::Vector2f m_logicSize;
    sf::Vector2f m_scale;      // 缩放比例
    sf::Vector2f m_offset;     // 偏移量（用于黑边）

    float m_scaleZoom;

public:
    void setRatioView(float logicWidth, float logicHeight) {
        m_logicSize = sf::Vector2f(logicWidth, logicHeight);
        m_scale = sf::Vector2f(1.0f, 1.0f);
        m_offset = sf::Vector2f(0, 0);
        m_scaleZoom = 1;
    }
    void setRatioView(sf::Vector2f WH) {
        m_logicSize = WH;
        m_scale = sf::Vector2f(1.0f, 1.0f);
        m_offset = sf::Vector2f(0, 0);
        m_scaleZoom = 1;
    }

    void reset(float logicWidth, float logicHeight) {
        m_logicSize = sf::Vector2f(logicWidth, logicHeight);
        m_scale = sf::Vector2f(1.0f, 1.0f);
        m_offset = sf::Vector2f(0, 0);
        m_scaleZoom = 1.0f;
    }

    void reset(const sf::Vector2f& wh) {
        reset(wh.x, wh.y);
    }

    void applyTo(sf::RenderTarget& window) {
        sf::Vector2u windowSize = window.getSize();
        float windowAspect = (float)windowSize.x / windowSize.y;
        float logicAspect = m_logicSize.x / m_logicSize.y;

        sf::Vector2f viewSize;
        if (windowAspect > logicAspect) {
            // 窗口更宽，上下留黑边
            viewSize.x = m_logicSize.y * windowAspect;
            viewSize.y = m_logicSize.y;
            m_offset.x = (viewSize.x - m_logicSize.x) / 2.0f;
            m_offset.y = 0;
        }
        else {
            // 窗口更高，左右留黑边
            viewSize.x = m_logicSize.x ;
            viewSize.y = m_logicSize.x / windowAspect;
            m_offset.x = 0;
            m_offset.y = (viewSize.y - m_logicSize.y) / 2.0f;
        }

        sf::Vector2f viewSizeScaled = sf::Vector2f(
            viewSize.x * m_scaleZoom,
            viewSize.y * m_scaleZoom
        );
        m_scale = sf::Vector2f(viewSizeScaled.x / m_logicSize.x, viewSizeScaled.y / m_logicSize.y);

        m_view.setSize(viewSizeScaled);
        m_view.setCenter({ 0,0 });
        window.setView(m_view);
    }

    // 获取缩放比例）
    sf::Vector2f getScale() const { return m_scale; }

    // 获取偏移量
    sf::Vector2f getOffset() const { return m_offset; }

    // 获取缩放偏移
    float getScaleZoom() const { return m_scaleZoom; }

    // 设置缩放偏移
    void setScaleZoom(float scaleZoom) { m_scaleZoom = scaleZoom; }

    // 将屏幕坐标转换为逻辑坐标
    sf::Vector2f toLogic(const sf::Vector2f& screenPos) const {
        return sf::Vector2f(
            (screenPos.x - m_offset.x) * m_scale.x,
            (screenPos.y - m_offset.y) * m_scale.y
        );
    }

    // 将逻辑坐标转换为屏幕坐标
    sf::Vector2f toScreen(const sf::Vector2f& logicPos) const {
        return sf::Vector2f(
            logicPos.x / m_scale.x + m_offset.x,
            logicPos.y / m_scale.y + m_offset.y
        );
    }
};