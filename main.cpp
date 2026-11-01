#include <iostream>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <string>

const double G_ACCEL = 9.81;
const int MAX_TARGETS = 64;

enum TargetType{PERSONNEL, LOGISTICS, APC, TANK, RADAR};

struct Target{
    double targ_x;
    double targ_y;
    double targ_hp;
    double targ_value;
    TargetType targ_type;
};

struct Ammo{
    std::string name_ammo;
    double defet_rad;
    double damage_ammo;
    double price_ammo;
};

double get_target_value(std::string text){
  if(text == "PERSONNEL")  return 10;
  if(text == "LOGISTICS")  return 20;
  if(text == "APC")        return 40;
  if(text == "TANK")       return 60;
  if(text == "RADAR")      return 90;
  return 0.0; 
}

int main(int argc, char* argv[])
{
    std::string input_path = "";
    std::string output_path = "";
    
    for (int i = 1; i < argc; ++i){
        std::string arg = argv[i];
        if(arg == "--input-file" && i + 1 < argc){
            input_path = argv[++i];
        }else if(arg =="--output-file" && i + 1 < argc){
            output_path = argv[++i];
        }
    }
    
    if (input_path.empty() || output_path.empty()) {
        std::cerr << "Помилка: Не вказано обов'язкові аргументи!\n";
        std::cerr << "Використання: " << argv[0] << " --input-file <шлях> --output-file <шлях>\n";
        return 1;
    }  
    
    double altitude_m = 0.0;
    double target_x;
    double target_y;
    double wind_x = 0.0;
    double wind_y = 0.0;
    int count = 0;
    
    std::ifstream my_file(input_path);
    if( !my_file.is_open()){
        std::cerr << "Помилка! Не вдалося відкрити файл!\n";
        return 1;
    }
    
    my_file >> altitude_m;
    my_file >> wind_x >> wind_y;
    my_file >> count;
    Target targets[MAX_TARGETS];
    
    for(int i = 0; i < count; i++){
        std::string temp_type;
        my_file >> targets[i].targ_x
                >> targets[i].targ_y
                >> targets[i].targ_hp
                >> temp_type;
        targets[i].targ_value = get_target_value(temp_type);
    }
    my_file.close();

    Ammo all_ammo[3]{
        {"Frag", 60.0, 25.0, 35.0},
        {"HE",   35.0, 55.0, 20.0},
        {"HEAT", 15.0, 130.0, 45.0}
    };
    
    double max_net = -999999.0;
    int best_i = 0;
    int best_a = 0;
    int best_destroyed = 0;
    double best_value_sum = 0.0;
    
    for (int i = 0; i < count; ++i) {
        double impact_x = targets[i].targ_x;
        double impact_y = targets[i].targ_y;
        for (int a = 0; a < 3; ++a) {
            double curr_value_sum  = 0.0;
            int curr_destr_count   = 0;
            for (int j = 0; j < count; ++j) {
                double dx = targets[j].targ_x - impact_x;
                double dy = targets[j].targ_y - impact_y;
                double dist_to_targ = std::sqrt(dx*dx + dy*dy);
                if (dist_to_targ <= all_ammo[a].defet_rad && all_ammo[a].damage_ammo >= targets[j].targ_hp) {
                    curr_value_sum += targets[j].targ_value;
                    curr_destr_count += 1;
                }
            } 
            double current_net = curr_value_sum - all_ammo[a].price_ammo;
            if (current_net > max_net) {
                max_net = current_net;
                best_i = i;
                best_a = a;
                best_destroyed = curr_destr_count;
                best_value_sum = curr_value_sum;
            } 
            else if (std::abs(current_net - max_net) < 1e-7) {
                if (all_ammo[a].price_ammo < all_ammo[best_a].price_ammo) {
                    best_i = i;
                    best_a = a;
                    best_destroyed = curr_destr_count;
                    best_value_sum = curr_value_sum;
                }
            }    
        } 
    } 
           
    double impact_x = targets[best_i].targ_x;
    double impact_y = targets[best_i].targ_y;
    double t = std::sqrt((2.0 * altitude_m) / G_ACCEL);
    double release_x = impact_x - wind_x * t;
    double release_y = impact_y - wind_y * t;
    
    std::ofstream out_file(output_path);
    if( !out_file.is_open()){
        std::cerr << "Помилка! Не вдалося відкрити файл результату!\n";
        return 1;
    }
    
    out_file << std::fixed << std::setprecision(2);
    out_file << impact_x << " " << impact_y << "\n";
    out_file << release_x << " " << release_y << "\n";
    out_file << all_ammo[best_a].name_ammo << "\n";
    out_file << best_destroyed << " " 
         << static_cast<int>(best_value_sum) << " " 
         << static_cast<int>(all_ammo[best_a].price_ammo) << " " 
         << static_cast<int>(max_net) << "\n";
    out_file.close();
    return 0;
}
