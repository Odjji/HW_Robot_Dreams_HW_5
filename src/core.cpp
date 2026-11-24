#include "core.h"
#include "provided_coverage.h"
#include <algorithm>
#include <iomanip>


bool parseScene(std::istream& in, Scene& out_scene) {
    
    int count = 0;

    in >> out_scene.altitude;
    in >> out_scene.wind.x >> out_scene.wind.y;
    in >> count;

    if (!in || count == 0) { 
        return false;
    }

    out_scene.targets.clear();

    for (int  i = 0; i < count; ++i) {
        double x = 0.0, y = 0.0, thp = 0.0;
        std::string temp_type; 
        in >> x >> y >> thp >> temp_type;
        
        double tvalue  = get_target_value(temp_type);
        
        Target target(Position_2D(x, y), thp, UNKNOW, tvalue);

        out_scene.targets.push_back(target);
    }
    return true;
}

bool formatResult(std::ostream& out, const Sol_Candidate& candidate) {

    out << std::fixed << std::setprecision(2);

    out << candidate.impact.x << " " << candidate.impact.y << "\n";
    out << candidate.release.x << " " << candidate.release.y << "\n";
    out << candidate.ammo_name << "\n";

    out << candidate.destroyed << " " 
        << static_cast<int>(candidate.value_sum) << " "
        << static_cast<int>(candidate.price_ammo) << " " 
        << static_cast<int>(candidate.max_netto) << "\n";

    return true;
}


Sol_Candidate calculate_best_solution(
    const Scene& scene,
    const std::vector<Ammo>& ammo_list,
    const ITrajectorySource& trajectory
) {
    return calculate_best_solution(scene.targets, ammo_list, scene.wind, scene.altitude, trajectory);
}   
    

Sol_Candidate calculate_best_solution(
    const std::vector<Target>& targets,
    const std::vector<Ammo>& ammo_list,
    const Position_2D& wind,
    double altitude,
    const ITrajectorySource& trajectory
) {
    std::vector<Sol_Candidate> candidates;
    candidates.reserve(targets.size() * ammo_list.size());
    

    for (size_t i = 0; i < targets.size(); ++i) { 
            Position_2D impact_pos = targets[i].getPos();
        
            // 2. Обираємо тип боєприпасу a
            for (size_t a = 0; a < ammo_list.size(); ++a) { 
                double curr_value_sum  = 0.0;
                int curr_destr_count   = 0;
            
                // 3. Симулюємо вибух: перевіряємо ВСІ цілі j за допомогою модуля накриття
                for (size_t j = 0; j < targets.size(); ++j) { 
                    if (CoverageChecker::is_target_covered(impact_pos, targets[j], ammo_list[a])) {
                        curr_value_sum += targets[j].getValue();
                        curr_destr_count += 1;
                    }
                } 
            
                // Підбиття підсумків: чисте нетто комбінації (цінність мінус вартість бомби)
                double current_net = curr_value_sum - ammo_list[a].getPriceAmmo();
                
                // Розраховуємо точку скиду через поліморфний розумний вказівник
                Position_2D curr_realese = trajectory.calculateRelease(impact_pos, altitude, wind);
                
                // Сформуємо та запишемо поточного кандидата у загальну базу даних
                Sol_Candidate my_candidate;
                my_candidate.ammo_name    = ammo_list[a].getNameAmmo();
                my_candidate.price_ammo   = ammo_list[a].getPriceAmmo();
                my_candidate.destroyed    = curr_destr_count;
                my_candidate.value_sum    = curr_value_sum;
                my_candidate.max_netto    = current_net;
                my_candidate.impact       = impact_pos;
                my_candidate.release      = curr_realese;
                
                candidates.push_back(my_candidate);
            } 
    }
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
    
    // Повертаємо найкращого кандидата або пустий об'єкт, якщо кандидатів немає
    if (best_it != candidates.end()) {
        return *best_it;
    } else {
        return Sol_Candidate{};
    }
}
