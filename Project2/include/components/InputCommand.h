#pragma once

enum class Command {
    None, MoveUp, MoveDown, MoveLeft, MoveRight, Attack
};

struct InputCommand {
    Command cmd = Command::None;
};
