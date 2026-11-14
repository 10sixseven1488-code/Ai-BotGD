#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <matjson/std.hpp>
#include <cmath>

using namespace geode::prelude;

// Одна точка, где бот умер: позиция объекта-убийцы и его ID.
struct HazardPoint {
    float x;
    float y;
    int objectID;
};

// Учим matjson сохранять/читать HazardPoint в save-файле мода.
template <>
struct matjson::Serialize<HazardPoint> {
    static matjson::Value toJson(HazardPoint const& h) {
        return matjson::makeObject({
            { "x", h.x },
            { "y", h.y },
            { "id", h.objectID }
        });
    }

    static geode::Result<HazardPoint> fromJson(matjson::Value const& value) {
        GEODE_UNWRAP_INTO(auto x, value["x"].asDouble());
        GEODE_UNWRAP_INTO(auto y, value["y"].asDouble());
        GEODE_UNWRAP_INTO(auto id, value["id"].asInt());
        return Ok(HazardPoint{ static_cast<float>(x), static_cast<float>(y), id });
    }
};

class $modify(AIDriverPlayLayer, PlayLayer) {
    struct Fields {
        std::vector<HazardPoint> m_hazards; // всё, что бот уже знает про этот уровень
        int m_levelKey = 0;
    };

    static std::string hazardKey(int levelID) {
        return "hazards-" + std::to_string(levelID);
    }

    bool init(GJGameLevel* level) {
        if (!PlayLayer::init(level)) return false;

        // Для онлайн-уровней m_levelID стабилен между попытками/сессиями.
        // У локальных несохранённых уровней он может быть 0 — тогда память
        // будет общей для всех таких уровней. Это ограничение первой версии,
        // поправим отдельным шагом, если понадобится (например, через
        // стороннее API назначения ID редакторским уровням).
        m_fields->m_levelKey = level->m_levelID;

        auto key = hazardKey(m_fields->m_levelKey);
        m_fields->m_hazards = Mod::get()->getSavedValue<std::vector<HazardPoint>>(key, {});

        log::info("AI Driver: загружено {} известных опасностей для уровня {}",
            m_fields->m_hazards.size(), m_fields->m_levelKey);

        return true;
    }

    // Вызывается GD перед тем, как убить игрока. object — то, что его убило.
    void destroyPlayerPre(PlayerObject* player, GameObject* object) {
        PlayLayer::destroyPlayerPre(player, object);

        if (!object) return;

        float hx = object->getPositionX();
        float hy = object->getPositionY();

        // Не дублируем точки, которые уже знаем (с небольшим допуском)
        for (auto& h : m_fields->m_hazards) {
            if (std::fabs(h.x - hx) < 5.f && std::fabs(h.y - hy) < 20.f) {
                return;
            }
        }

        m_fields->m_hazards.push_back({ hx, hy, object->m_objectID });
        Mod::get()->setSavedValue(hazardKey(m_fields->m_levelKey), m_fields->m_hazards);

        log::info("AI Driver: запомнил новую опасность на x={:.0f} (всего известно: {})",
            hx, m_fields->m_hazards.size());
    }
};
