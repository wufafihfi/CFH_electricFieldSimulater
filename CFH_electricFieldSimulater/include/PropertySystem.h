#pragma once
#include <string>
#include <variant>
#include <unordered_map>
#include <vector>
#include <functional>
#include <utility>
#include <optional>
#include <filesystem>

//###############学习#################
// 阅 - 1遍
//####################################
namespace M_Property {
    // 支持的数据类型
    using PropertyValue = std::variant<
        bool,
        int,
        float,
        double,
        std::string,
        std::vector<float>,  // 可用于 Vector2/Vector3
        std::function<void()>, // 用于按钮/回调
        std::filesystem::path
    >;

    // 单个属性
    struct Property {
        std::string name;           // 属性名称（唯一标识）
        std::string displayName;    // 显示名称（可带空格、中文）
        PropertyValue value;        // 当前值
        PropertyValue min;          // 最小值（可选）
        PropertyValue max;          // 最大值（可选）
        bool isReadOnly = false;    // 是否只读

        Property() = default;

        Property(const std::string& n, const PropertyValue& v)
            : name(n), displayName(n), value(v) {
        }

        Property(const std::string& n, const std::string& dn, const PropertyValue& v)
            : name(n), displayName(dn), value(v) {
        }
    };

    // 属性集合（用于分组）
    struct PropertyGroup {
        std::string name;                    // 组名
        std::vector<Property> properties;    // 组内的属性列表
        bool isExpanded = true;              // 是否展开
    };

    // 属性容器
    class PropertyContainer {
    public:
        // 添加属性组
        void AddGroup(const std::string& groupName) {
            if (m_groups.find(groupName) == m_groups.end()) {
                m_groups[groupName] = {};
            }
        }

        // 添加属性到指定组
        void AddProperty(const std::string& groupName, const Property& prop) {
            AddGroup(groupName);

            auto& group = m_groups[groupName];

            // 查找是否已存在同名属性
            for (auto& existingProp : group.properties) {
                if (existingProp.name == prop.name) {
                    // 覆盖已存在的属性
                    existingProp = prop;
                    return;
                }
            }

            // 不存在则添加
            group.properties.push_back(prop);
        }

        // 获取所有组
        std::unordered_map<std::string, PropertyGroup>* GetAllGroup() {
            return &m_groups;
        }

        // 获取组
        PropertyGroup* GetGroup(const std::string& groupName) {
            auto groupIt = m_groups.find(groupName);
            if (groupIt != m_groups.end()) {
                return &(groupIt->second);
            }
            return nullptr;
        }

        std::optional<Property> GetProperty(const std::string& groupName, const std::string& propName) {
            auto groupIt = m_groups.find(groupName);
            if (groupIt != m_groups.end()) {
                for (auto& prop : groupIt->second.properties) {
                    if (prop.name == propName) {
                        return prop;
                    }
                }
            }
            return std::nullopt;
        }

        // 获取属性值（通过组名和属性名）
        template<typename T>
        T GetValue(const std::string& groupName, const std::string& propName, const T& defaultValue = T{}) {
            auto groupIt = m_groups.find(groupName);
            if (groupIt != m_groups.end()) {
                for (auto& prop : groupIt->second.properties) {
                    if (prop.name == propName) {
                        if (std::holds_alternative<T>(prop.value)) {
                            return std::get<T>(prop.value);
                        }
                    }
                }
            }
            return defaultValue;
        }

        // 设置属性值
        template<typename T>
        void SetValue(const std::string& groupName, const std::string& propName, const T& value) {
            auto groupIt = m_groups.find(groupName);
            if (groupIt != m_groups.end()) {
                for (auto& prop : groupIt->second.properties) {
                    if (prop.name == propName) {
                        prop.value = value;
                        m_dirty = true;
                        // 触发变更回调
                        if (m_onPropertyChanged) {
                            m_onPropertyChanged(propName, value);
                        }
                        return;
                    }
                }
            }
        }

        void ForEachGroup(std::function<void(const std::string&, PropertyGroup&)> callback) {
            for (auto& pair : m_groups) {
                callback(pair.first, pair.second);
            }
        }

        // 结构化绑定（C++17）
        void ForEachGroupCpp17(std::function<void(const std::string&, PropertyGroup&)> callback) {
            for (auto& [groupName, group] : m_groups) {
                callback(groupName, group);
            }
        }

        // 清除所有属性
        void Clear() {
            m_groups.clear();
        }

        // 属性变更回调
        void SetOnPropertyChanged(std::function<void(const std::string&, const PropertyValue&)> callback) {
            m_onPropertyChanged = callback;
        }

        // 脏标志管理
        bool IsDirty() const { return m_dirty; }
        void ClearDirty() { m_dirty = false; }
        void SetDirty() { m_dirty = true; }

    private:
        std::unordered_map<std::string, PropertyGroup> m_groups;
        std::function<void(const std::string&, const PropertyValue&)> m_onPropertyChanged;
        bool m_dirty = false;
    };

    // 高频访问需求
    class Transform {
    public:
        // 直接访问（内联，零开销）
        float x = 0, y = 0, z = 0;
        float rotation = 0;
        float scaleX = 1, scaleY = 1;

        // 可选：变更通知（通过回调或脏标记）
        void SetX(float value) {
            x = value;
            m_dirty = true;
            if (onChange) onChange("x", value);
        }

        std::function<void(const char*, float)> onChange;

    private:
        bool m_dirty = false;
    };
}