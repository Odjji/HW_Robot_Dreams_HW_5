#include <iostream>
#include <fstream>
#include <iomanip>
#include <memory>
#include <chrono>
#include "core.h"






    
// Запис фінального звіту про роботу системи донаведення
bool saveToFile(
    const std::string& output_path,
    const Sol_Candidate& best
) {
    std::ofstream out_file(output_path);

    if (!out_file.is_open()) {
        std::cout << "Помилка! Не вдалося відкрити файл результату! \n";
        return false;
    }
    
    out_file << std::fixed << std::setprecision(2);
    out_file << best.impact.x << " " << best.impact.y << "\n";
    out_file << best.release.x << " " << best.release.y << "\n";
    out_file << best.ammo_name << "\n";

     out_file << best.destroyed << " " 
             << static_cast<int>(best.value_sum) << " " 
             << static_cast<int>(best.price_ammo) << " " 
             << static_cast<int>(best.max_net) << "\n";

    out_file.close();  
    return true; 
}    




// БЛОК: Шаблонна функція вищого порядку для високоточного хронометражу обчислень
template <typename Fn_cl>
long long measure_us(Fn_cl fn_cl) {
    const auto start = std::chrono::steady_clock::now(); // Фіксуємо мітку часу ДО початку
    fn_cl();                                             // Виконуємо переданий блок розрахунків
    const auto finish = std::chrono::steady_clock::now(); // Фіксуємо мітку часу ПІСЛЯ завершення
    // Повертаємо чисту тривалість, приведену до мікросекунд
    return std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count();
}

// ============================================================================
// ОСНОВНА ФУНКЦІЯ ПРОГРАМИ: MAIN
// ============================================================================
int main(int argc, char* argv[])
{
    std::string input_path = "";
    std::string output_path = "";
    std::string trajectory_type = "hover";
    
    // БЛОК: Парсинг аргументів командного рядка
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--input-file" && i + 1 < argc) {
            input_path = argv[++i];
        } else if (arg == "--output-file" && i + 1 < argc) {
            output_path = argv[++i];
        } else if (arg == "--trajectory" && i + 1 < argc) {
            trajectory_type = argv[++i];
        }
    }
    
    // Перевірка наявності обов'язкових шляхів до файлів сценарію
    if (input_path.empty() || output_path.empty()) {
        std::cerr << "Помилка: Не вказано обов'язкові аргументи!\n";
        return 1;
    } 
    
    
    std::ifstream input_file(input_path);
    if (!input_file.is_open()) {
        std::cerr << "Помилка: Не вдалося відкрити файл сценарію!\n";
        return 1;
    }

    double altitude_m = 0.0;
    Position_2D wind(0.0, 0.0);
    int count = 0;

    input_file >> altitude_m;
    input_file >> wind.x >> wind.y;
    input_file >> count;

    // --- ОБРОБКА ПОРОЖНЬОГО СЦЕНАРІЮ 
    if (count == 0) {
        std::cerr << "Помилка: Сценарій порожній!\n";
        return 1;
    }
    std::vector<Target> targets;
    targets.reserve(count);
    for (int i = 0; i < count; ++i) {
        double tx, ty, thp;
        std::string temp_type;
        my_file >> tx >> ty >> thp >> temp_type;
        double tvalue = get_target_value(temp_type);
        targets.push_back(Target(Position_2D(tx, ty), thp, UNKNOW, tvalue));
    }

    my_file.close();
    
    // Створюємо універсальний роз'єм для моделі траєкторії (розумний вказівник RAII)
    std::unique_ptr<ITrajectorySource> trajectorySim = nullptr;

    // Визначаємо, яку саме модель динаміки польоту підключити
    if (trajectory_type == "forward") { 
        trajectorySim = std::make_unique<ForwardTrajectory>(); // Створюємо модель ходу
    } else {
        trajectorySim = std::make_unique<HoverTrajectory>();   // Інакше створюємо модель зависання
    }

    // Динамічний вектор доступних боєприпасів із використанням класу Ammo (Modern C++)
    std::vector<Ammo> all_ammo {
        {"Frag", 60.0, 25.0, 35.0},
        {"HE",   35.0, 55.0, 20.0},
        {"HEAT", 15.0, 130.0, 45.0}
    };
    
    // Створюємо плоский контейнер для збереження абсолютно всіх комбінацій рішень
    Sol_Candidate best_solution;
          
    // --- ПОЧАТОК ЗОНИ ХРОНОМЕТРАЖУ ЧИСТИХ ОБЧИСЛЕНЬ ---
    long long elpased_us = measure_us([&]() {
    
        // 1. Фіксуємо точку прицілювання (ціль i)
        for (int i = 0; i < mission.getCount(); ++i) { 
            Position_2D impact_pos = mission.getTarget(i).getPos();
        
            // 2. Обираємо тип боєприпасу a
            for (size_t a = 0; a < all_ammo.size(); ++a) { 
                double curr_value_sum  = 0.0;
                int curr_destr_count   = 0;
            
                // 3. Симулюємо вибух: перевіряємо ВСІ цілі j на карті
                for (int j = 0; j < mission.getCount(); ++j) { 
                    double dist_to_targ = mission.getTarget(j).getPos().distanceTo(impact_pos);
                
                    // Перевірка радіусу ураження та пробиття броні/міцності цілі
                    if (dist_to_targ <= all_ammo[a].getDefetRad() && all_ammo[a].getDamageAmmo() >= mission.getTarget(j).getHp()) {
                        curr_value_sum += mission.getTarget(j).getValue();
                        curr_destr_count += 1;
                    }
                } 
            
                // Підбиття підсумків: чисте нетто комбінації (цінність мінус вартість бомби)
                double current_net = curr_value_sum - all_ammo[a].getPriceAmmo();
                
                // Розраховуємо точку скиду через поліморфний розумний вказівник
                Position_2D curr_realese = trajectorySim->calculateRelease(impact_pos, mission.getAltitudeM(), mission.getWind());
                
                // Сформуємо та запишемо поточного кандидата у загальну базу даних
                Sol_Candidate my_candidate;
                my_candidate.ammo_name    = all_ammo[a].getNameAmmo();
                my_candidate.price_ammo   = all_ammo[a].getPriceAmmo();
                my_candidate.destroyed    = curr_destr_count;
                my_candidate.value_sum    = curr_value_sum;
                my_candidate.max_netto    = current_net;
                my_candidate.impact       = impact_pos;
                my_candidate.release      = curr_realese;
                
                candidates.push_back(my_candidate);
            } 
        }
    });
    // --- КІНЕЦЬ ЗОНИ ХРОНОМЕТРАЖУ ЧИСТИХ ОБЧИСЛЕНЬ ---
    
    // БЛОК: Вибір найкращого рішення стандартним STL алгоритмом пошуку максимуму
    auto best_it = std::max_element(candidates.begin(), candidates.end(), [](const Sol_Candidate& a, const Sol_Candidate& b) {
        // Критерій 1: Шукаємо варіант із найбільшим чистим нетто
        if (a.max_netto != b.max_netto) {
            return a.max_netto < b.max_netto;
        } 
        // Тайбрейк 1: За рівного нетто обираємо дешевший боєприпас (дорожчий є "гіршим")
        if (a.price_ammo != b.price_ammo) {
            return a.price_ammo > b.price_ammo;
        }
        // Тайбрейк 2: Перемагає варіант, що зустрівся першим (гарантовано логікою std::max_element)
        return false;
    });  
    
    // Друкуємо тривалість розрахунку у мікросекундах суворо в stdout за форматом ТЗ
    std::cout << "elapsed_us " << elpased_us << "\n";

    // БЛОК: Запис фінальних результатів у файл
    if (best_it != candidates.end()) {
        // Викликаємо збереження, дістаючи всі дані напряму з найкращого кандидата через ітератор
        mission.saveToFile(
            output_path,
            best_it->impact,
            best_it->release,
            best_it->ammo_name,
            best_it->destroyed,
            best_it->value_sum,
            best_it->price_ammo,
            best_it->max_netto
        );
    }
    
    // Розумний вказівник усе видалить сам. Ручних delete немає!
    return 0;
}
