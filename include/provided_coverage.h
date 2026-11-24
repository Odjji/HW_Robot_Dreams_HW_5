#pragma once

#include "types.h"

// Header-only модуль швидкої перевірки накриття цілі боєприпасом
class CoverageChecker {
public:
    // Швидка перевірка накриття цілі без використання повільної математичної операції std::sqrt
    static inline bool is_target_covered(
        const Position_2D& impact, 
        const Target& target, 
        const Ammo& ammo
    ) {
        // 1. Перевіряємо пробиття міцності/броні цілі
        if (ammo.getDamageAmmo() < target.getHp()) {
            return false;
        }

        // 2. Рахуємо квадрат прямої відстані між точкою влучання та ціллю (dx^2 + dy^2)
        double dx = impact.x - target.getPos().x;
        double dy = impact.y - target.getPos().y;
        double squared_dist = dx * dx + dy * dy;

        // 3. Рахуємо квадрат радіуса ураження боєприпасу
        double rad = ammo.getDefetRad();
        double squared_rad = rad * rad;

        // 4. Порівнюємо квадрати відстаней
        return squared_dist <= squared_rad;
    }
};
