#pragma once
#include "../components/InputCommand.h"

class InputSystem {
public:
    void update(ComponentStore<InputCommand>& inputs, Entity e, Command c) {
        if (inputs.has(e)) {
            inputs.get(e).cmd = c;
        }
    }

    void setCommand(ComponentStore<InputCommand>& inputs, Entity e, Command c) {
        if (!inputs.has(e)) inputs.add(e, InputCommand());
        inputs.get(e).cmd = c;
    }

};
