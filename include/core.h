#pragma once

#include <istream>
#include <ostream>
#include "types.h"
#include "trajectory.h"

bool parseScene(std::istream& in, Scene& out_scene);

bool formatResult(std::ostream& out, const Sol_Candidate& solution);

Sol_Candidate calculate_best_solution(
    const Scene& scene,
    const std::vector<Ammo>& ammo_list,
    const ITrajectorySource& trajectory
    );

Sol_Candidate calculate_best_solution(
    const std::vector<Target>& targets, 
    const std::vector<Ammo>& ammo_list,
    const Position_2D& wind,
    double altitude,
    const ITrajectorySource& trajectory
    );

    