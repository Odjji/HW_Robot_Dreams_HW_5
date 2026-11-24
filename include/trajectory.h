#pragma once

#include "types.h"

// БЛОК: Абстрактний інтерфейс (контракт) для розрахунку джерел траєкторій
class ITrajectorySource {
public:
    static constexpr double G_ACCEL = 9.81;
    
    // Віртуальний деструктор за замовчуванням для безпечного автоматичного очищення через розумні вказівники
    virtual ~ITrajectorySource() = default;
    virtual Position_2D calculateRelease(const Position_2D& impact, double altitude, const Position_2D& wind) const = 0;
};

// БЛОК: Конкретна реалізація траєкторії для режиму зависання (Hover)
// Використовується, коли дрон нерухомо висить над сектором цілей
class HoverTrajectory : public ITrajectorySource {
public:
    // Перевизначення базового методу розрахунку точки скиду боєприпасу
    Position_2D calculateRelease(const Position_2D& impact, double altitude, const Position_2D& wind) const override {
        // 1. Рахуємо точний час падіння боєприпасу з висоти польоту
        double t = std::sqrt((2.0 * altitude) / ITrajectorySource::G_ACCEL);
        
        // 2. Повертаємо точку скиду, зміщену тільки проти напрямку вітру
        return impact - wind * t;
    }
};

// БЛОК: Конкретна реалізація траєкторії для скиду на ходу (Forward)
// Використовується, коли дрон продовжує рух під час відділення боєприпасу
class ForwardTrajectory : public ITrajectorySource {
public:
    // Конструктор класу: фіксує постійну крейсерську швидкість дрона (12 м/с строго по осі X)
    ForwardTrajectory() : v_drone(12.0, 0.0) {}

    // Перевизначення базового методу розрахунку для динамічного польоту
    Position_2D calculateRelease(const Position_2D& impact, double altitude, const Position_2D& wind) const override {
        // 1. Обчислюємо час падіння боєприпасу (аналогічно до моделі Hover)
        double t = std::sqrt((2.0 * altitude) / ITrajectorySource::G_ACCEL);
        
        // 2. Повертаємо точку скиду, зміщену проти суми швидкостей вітру та самого дрона
        return impact - (wind + v_drone) * t;
    }

private:
    Position_2D v_drone; // Внутрішня константа власної горизонтальної швидкості БПЛА
};