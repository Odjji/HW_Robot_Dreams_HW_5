#include <gtest/gtest.h>
#include <sstream>
#include "core.h"

// 1. Тести розбору (Parser)
TEST(ParserTest, HandlesEmptyScenario) {
    std::stringstream ss("0\n"); // 0 цілей
    Scene scene;
    EXPECT_FALSE(parseScene(ss, scene));
}

TEST(ParserTest, ParsesValidScenario) {
    std::string data = 
        "100.0\n"
        "2.0 3.0\n"
        "1\n"
        "10.0 20.0 50 PERSONNEL\n";
    std::stringstream ss(data);
    Scene scene;
    
    ASSERT_TRUE(parseScene(ss, scene));
    EXPECT_DOUBLE_EQ(scene.altitude, 100.0);
    EXPECT_DOUBLE_EQ(scene.wind.x, 2.0);
    EXPECT_DOUBLE_EQ(scene.wind.y, 3.0);
    ASSERT_EQ(scene.targets.size(), 1u);
}

TEST(ParserTest, HandlesCorruptedData) {
    std::stringstream ss("not_a_number\n");
    Scene scene;
    EXPECT_FALSE(parseScene(ss, scene));
}

// 2. Тести балістичних моделей (Trajectory)
TEST(TrajectoryTest, HoverModelCalculatesCorrectRelease) {
    HoverTrajectory hover;
    Position_2D impact(100.0, 200.0);
    Position_2D wind(5.0, 0.0);
    double altitude = 120.0;
    
    Position_2D release = hover.calculateRelease(impact, altitude, wind);
    
    EXPECT_NEAR(release.x, 75.27, 0.1);
    EXPECT_NEAR(release.y, 200.0, 0.01);
}

TEST(TrajectoryTest, ForwardModelAddsDroneVelocity) {
    ForwardTrajectory forward;
    Position_2D impact(100.0, 200.0);
    Position_2D wind(5.0, 0.0);
    double altitude = 120.0;
    
    Position_2D release = forward.calculateRelease(impact, altitude, wind);
    
    EXPECT_LE(release.x, 75.0);
}

// 3. Тести форматування результату (Formatter)
TEST(FormatterTest, FormatsResultWithExactPrecision) {
    Sol_Candidate sol;
    sol.impact = Position_2D(300.0, 400.0);
    sol.release = Position_2D(275.27, 400.0);
    sol.ammo_name = "Frag";
    sol.destroyed = 4;
    sol.value_sum = 50.0;
    sol.price_ammo = 35.0;
    sol.max_netto = 15.0;

    std::stringstream ss;
    ASSERT_TRUE(formatResult(ss, sol));

    std::string expected = 
        "300.00 400.00\n"
        "275.27 400.00\n"
        "Frag\n"
        "4 50 35 15\n";

    EXPECT_EQ(ss.str(), expected);
}
// 4. Регресійні тести сценаріїв ДЗ 5 (Hover & Forward)
const std::vector<Ammo> default_ammo_list {
    {"Frag", 60.0, 25.0, 35.0},
    {"HE",   35.0, 55.0, 20.0},
    {"HEAT", 15.0, 130.0, 45.0}
};

TEST(RegressionTest, MissionA_Hover) {
    std::string input_data = 
        "120.0\n"
        "5.0 0.0\n"
        "4\n"
        "300.0 400.0 20 LOGISTICS\n"
        "300.0 450.0 15 PERSONNEL\n"
        "260.0 380.0 15 PERSONNEL\n"
        "340.0 380.0 15 PERSONNEL\n";
    
    std::stringstream ss(input_data);
    Scene scene;
    ASSERT_TRUE(parseScene(ss, scene));

    HoverTrajectory hover;
    Sol_Candidate res = calculate_best_solution(scene, default_ammo_list, hover);

    EXPECT_NEAR(res.impact.x, 300.00, 0.01);
    EXPECT_NEAR(res.impact.y, 400.00, 0.01);
    EXPECT_NEAR(res.release.x, 275.27, 0.01);
    EXPECT_NEAR(res.release.y, 400.00, 0.01);
    EXPECT_EQ(res.ammo_name, "Frag");
    EXPECT_EQ(res.destroyed, 4);
    EXPECT_EQ(static_cast<int>(res.value_sum), 50);
    EXPECT_EQ(static_cast<int>(res.price_ammo), 35);
    EXPECT_EQ(static_cast<int>(res.max_netto), 15);
}

TEST(RegressionTest, MissionC_Forward) {
    std::string input_data = 
        "150.0\n"
        "3.0 4.0\n"
        "4\n"
        "0.0 0.0 50 APC\n"
        "20.0 10.0 50 APC\n"
        "-15.0 20.0 45 RADAR\n"
        "300.0 300.0 15 PERSONNEL\n";
        
    std::stringstream ss(input_data);
    Scene scene;
    ASSERT_TRUE(parseScene(ss, scene));

    ForwardTrajectory forward;
    Sol_Candidate res = calculate_best_solution(scene, default_ammo_list, forward);

    EXPECT_NEAR(res.impact.x, 0.00, 0.01);
    EXPECT_NEAR(res.impact.y, 0.00, 0.01);
    EXPECT_NEAR(res.release.x, -82.95, 0.01);
    EXPECT_NEAR(res.release.y, -22.12, 0.01);
    EXPECT_EQ(res.ammo_name, "HE");
    EXPECT_EQ(res.destroyed, 3);
    EXPECT_EQ(static_cast<int>(res.value_sum), 170);
    EXPECT_EQ(static_cast<int>(res.price_ammo), 20);
    EXPECT_EQ(static_cast<int>(res.max_netto), 150);
}

TEST(RegressionTest, MissionE_Hover) {
    std::string input_data = 
        "80.0\n"
        "7.0 0.0\n"
        "3\n"
        "1000.0 200.0 120 RADAR\n"
        "300.0 300.0 15 PERSONNEL\n"
        "320.0 310.0 15 PERSONNEL\n";
        
    std::stringstream ss(input_data);
    Scene scene;
    ASSERT_TRUE(parseScene(ss, scene));

    HoverTrajectory hover;
    Sol_Candidate res = calculate_best_solution(scene, default_ammo_list, hover);

    EXPECT_NEAR(res.impact.x, 1000.00, 0.01);
    EXPECT_NEAR(res.impact.y, 200.00, 0.01);
    EXPECT_NEAR(res.release.x, 971.73, 0.01);
    EXPECT_NEAR(res.release.y, 200.00, 0.01);
    EXPECT_EQ(res.ammo_name, "HEAT");
    EXPECT_EQ(res.destroyed, 1);
    EXPECT_EQ(static_cast<int>(res.value_sum), 90);
    EXPECT_EQ(static_cast<int>(res.price_ammo), 45);
    EXPECT_EQ(static_cast<int>(res.max_netto), 45);
}
