#pragma once

#include <string>
#include <cmath>
#include <vector>


// Перелік типів цілей для класифікації об'єктів на карті
enum TargetType {
    UNKNOW = 0,
    PERSONNEL, 
    LOGISTICS, 
    APC, 
    TANK, 
    RADAR
};


// --- ДОПОМІЖНА ФУНКЦІЯ КОНВЕРТАЦІЇ 
inline  double get_target_value(const std::string& text) {
    if (text == "PERSONNEL") return 10;
    if (text == "LOGISTICS") return 20;
    if (text == "APC")       return 40;
    if (text == "TANK")      return 60;
    if (text == "RADAR")     return 90;
    return 0.0; 
}


// БЛОК: Базова математична структура для збереження координат та дій над ними
struct Position_2D {
    double x = 0.0;
    double y = 0.0;
    
    Position_2D() : x(0.0), y(0.0) {}
    Position_2D(double init_x, double init_y) : x(init_x), y(init_y) {}
    
    // Обчислення прямої відстані між двома точками на площині
    double distanceTo(const Position_2D& other) const {
        double dx = this->x - other.x; 
        double dy = this->y - other.y; 
        return std::sqrt(dx * dx + dy * dy);
    }
    
    // Оператор віднімання для зсуву координат проти вектора зносу
    Position_2D operator-(const Position_2D& other) const {
        Position_2D result;
        result.x = this->x - other.x;
        result.y = this->y - other.y;
        return result;
    }
    
    // Оператор додавання для об'єднання кількох сил зносу (вітер + хід дрона)
    Position_2D operator+(const Position_2D& other) const {
        Position_2D result;
        result.x = this->x + other.x;
        result.y = this->y + other.y;
        return result;
    }
    
    // Оператор множення на скаляр (масштабування вектора швидкості на час падіння)
    Position_2D operator*(double scalar) const {
        Position_2D result;
        result.x = this->x * scalar;
        result.y = this->y * scalar;
        return result;
    }
};

// БЛОК: Структура для збереження потенційного результату (Кандидата) розрахунку системи донаведення
struct Sol_Candidate {
    std::string ammo_name;   // Назва обраного боєприпасу
    int destroyed;           // Кількість повністю знищених цілей
    double value_sum;        // Сумарна цінність знищених об'єктів
    double price_ammo;       // Ціна використаного боєприпасу
    double max_netto;        // Чисте тактичне нетто (цінність мінус витрати)
    Position_2D impact;      // Оптимальна точка прицілювання (влучання)
    Position_2D release;     // Розрахована точка скиду з урахуванням балістики
};

// БЛОК: Клас, що описує поодиноку ціль на полі бою
class Target {
public:
    Target() : pos(0.0, 0.0), hp(0.0), value(0.0), type(UNKNOW) {}
    
    // Конструктор класу: виправлено порядок ініціалізації полів відповідно до їх оголошення в private
    Target(Position_2D init_pos, double init_hp, TargetType init_type, double init_value)
         : pos(init_pos), hp(init_hp), value(init_value), type(init_type) {}
        
    Position_2D getPos() const { return pos; }
    double getHp()       const { return hp; }
    double getValue()    const { return value; }
    TargetType getType() const { return type; }

private:
    Position_2D pos;
    double hp;
    double value;
    TargetType type;
};

struct Scene{
    double altitude; // Висота польоту дрона, м
    Position_2D wind; // Швидкість і напрямок вітру, (Х, Y)
    std::vector<Target> targets; // Список усіх цілей на карті
};

// БЛОК: Клас, що описує характеристики боєприпасу
class Ammo {
public:
    Ammo() : name_ammo(""), defet_rad(0.0), damage_ammo(0.0), price_ammo(0.0) {}
    Ammo(std::string init_name, double init_defet_rad, double init_damage_ammo, double init_price_ammo)
         : name_ammo(init_name), defet_rad(init_defet_rad), damage_ammo(init_damage_ammo), price_ammo(init_price_ammo) {}

    std::string getNameAmmo()    const { return name_ammo; }
    double getDefetRad()         const { return defet_rad; }
    double getDamageAmmo()       const { return damage_ammo; }
    double getPriceAmmo()        const { return price_ammo; }
    
private:
    std::string name_ammo;
    double defet_rad;
    double damage_ammo;
    double price_ammo;
};
