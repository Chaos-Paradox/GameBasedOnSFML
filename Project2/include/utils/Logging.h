#pragma once
#include "../core/ECS.h"
#include "../components/InputCommand.h"
#include "../components/Position.h"
#include "../components/Stats.h"
#include "../components/AttackState.h"

// -----------------------------
// Utils: stringify components for logging
// -----------------------------
std::string to_string_cmd(Command c) {
    switch (c) {
    case Command::None: return "None";
    case Command::MoveUp: return "MoveUp";
    case Command::MoveDown: return "MoveDown";
    case Command::MoveLeft: return "MoveLeft";
    case Command::MoveRight: return "MoveRight";
    case Command::Attack: return "Attack";
    }
    return "Unknown";
}
std::string dump(const Position& p) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "Position(x=%.2f,y=%.2f)", p.x, p.y);
    return buf;
}
std::string dump(const Stats& s) {
    char buf[128];
    std::snprintf(buf, sizeof(buf), "Stats(HP=%.1f,ATK=%.1f,AS=%.2f)", s.hp, s.attackDamage, s.attackSpeed);
    return buf;
}
std::string dump(const InputCommand& ic) {
    return std::string("Input(") + to_string_cmd(ic.cmd) + ")";
}
std::string dump(const AttackState& a) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "AttackState(cd=%.3f,attacking=%s)", a.cooldown, a.attacking ? "true" : "false");
    return buf;
}

// -----------------------------
// Debug print: per-frame
// -----------------------------
void printFrameDebug(const ECS& ecs,
    const ComponentStore<Position>& positions,
    const ComponentStore<Stats>& stats,
    const ComponentStore<InputCommand>& inputs,
    const ComponentStore<AttackState>& attacks,
    int frame)
{
    std::cout << "\n======================== FRAME " << frame << " ========================\n";
    // list entities
    const auto& ents = ecs.entities();
    std::cout << "[DEBUG] Entities (created order):";
    for (auto e : ents) std::cout << " " << e;
    std::cout << "\n";

    // for each entity, show which components they have and the dump
    for (auto e : ents) {
        std::cout << "  Entity " << e << ":\n";
        if (positions.has(e))   std::cout << "    - " << dump(positions.get(e)) << "\n";
        if (stats.has(e))       std::cout << "    - " << dump(stats.get(e)) << "\n";
        if (inputs.has(e))      std::cout << "    - " << dump(inputs.get(e)) << "\n";
        if (attacks.has(e))     std::cout << "    - " << dump(attacks.get(e)) << "\n";
    }
    std::cout << "============================================================\n";
}