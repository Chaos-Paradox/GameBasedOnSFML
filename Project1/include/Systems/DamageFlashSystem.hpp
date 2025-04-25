#pragma once
#include "ComponentManager.hpp"
#include "Components.hpp"

class DamageFlashSystem {
public:
    DamageFlashSystem(ComponentManager& cm) : cm(cm) {}

    void update(float dt) {
        cm.forEach<DamageFlashComponent>([&](int id, DamageFlashComponent& df) {
            if (df.timer > 0.f) {
                df.timer -= dt;
                float freq = 10.f; // …¡À∏∆µ¬ £®¥Œ/√Î£©
                df.visible = static_cast<int>(df.timer * freq) % 2 == 0;
            } else {
                df.visible = true;
                df.timer = 0.f;
            }
        });
    }

    void flash(Entity e, float duration = 0.5f) {
        auto* comp = cm.getComponent<DamageFlashComponent>(e);
        if (comp) {
            comp->timer = duration;
            comp->totalDuration = duration;
        }
    }

private:
    ComponentManager& cm;
};
