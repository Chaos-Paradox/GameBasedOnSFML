#pragma once
#include "../components/Position.h"
#include "../components/InputCommand.h"

class MovementSystem {
public:
    float speed = 1.0f;

    void update(
        ComponentStore<Position>& pos,
        ComponentStore<InputCommand>& input,
        Entity e)
    {
        if (!pos.has(e) || !input.has(e)) return;

        auto& p = pos.get(e);
        auto& cmd = input.get(e).cmd;

        switch (cmd) {
        case Command::MoveUp:    p.y += speed; break;
        case Command::MoveDown:  p.y -= speed; break;
        case Command::MoveLeft:  p.x -= speed; break;
        case Command::MoveRight: p.x += speed; break;
        default: break;
        }
    }
};
