// ============================================================================
// МОДУЛЬ АДАПТЕРА / ЗАСТОСУНОК КОМАНДНОГО РЯДКА (CLI Application)
// ============================================================================
// Цей файл відповідає виключно за взаємодію з оточенням (ОС):
// - Розбір аргументів командного рядка (--input-file, --output-file, --trajectory)
// - Відкриття та закриття файлів вводу/виводу на диску
// - Вимірювання часу виконання обчислень ядра
// - Друк помилок у потік std::cerr та вивід часу у std::cout
// ============================================================================

#include <iostream>
#include <fstream>
#include <memory>
#include <chrono>
#include "core.h"

// Шаблонна функція вищого порядку для високоточного хронометрування обчислень ядра
template <typename Fn>
long long measure_us(Fn fn) {
    const auto start = std::chrono::steady_clock::now();
    fn();
    const auto finish = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count();
}

int main(int argc, char* argv[]) {
    std::string input_path;
    std::string output_path;
    std::string trajectory_type = "hover";
    
    // БЛОК 1: Розбір аргументів командного рядка
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
    
    // Перевірка наявності обов'язкових параметрів входу та виходу
    if (input_path.empty() || output_path.empty()) {
        std::cerr << "empty scenario\n";
        return 1;
    } 
    
    // БЛОК 2: Відкриття вхідного файлу та розбір сцени через API ядра
    std::ifstream input_file(input_path);
    if (!input_file.is_open()) {
        std::cerr << "empty scenario\n";
        return 1;
    }

    Scene scene;
    if (!parseScene(input_file, scene)) {
        std::cerr << "empty scenario\n";
        return 1;
    }
    input_file.close();

    // БЛОК 3: Підготовка поліморфної моделі траєкторії та списку боєприпасів
    std::unique_ptr<ITrajectorySource> trajectorySim;
    if (trajectory_type == "forward") { 
        trajectorySim = std::make_unique<ForwardTrajectory>();
    } else {
        trajectorySim = std::make_unique<HoverTrajectory>();
    }

    const std::vector<Ammo> all_ammo {
        {"Frag", 60.0, 25.0, 35.0},
        {"HE",   35.0, 55.0, 20.0},
        {"HEAT", 15.0, 130.0, 45.0}
    };

    // БЛОК 4: Вимірювання часу та виклик чистих обчислень ядра
    Sol_Candidate best_solution;
    long long elapsed_us = measure_us([&]() {
        best_solution = calculate_best_solution(scene, all_ammo, *trajectorySim);
    });

    // Обов'язковий вивід часу виконання у стандартний потік виводу
    std::cout << "elapsed_us " << elapsed_us << "\n";

    // БЛОК 5: Форматування та збереження результату у вихідний файл
    std::ofstream out_file(output_path);
    if (!out_file.is_open() || !formatResult(out_file, best_solution)) {
        return 1;
    }

    return 0;
}
