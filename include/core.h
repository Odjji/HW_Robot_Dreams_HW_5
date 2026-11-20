#pragma once

#include "types.h"
#include "trajectory.h"


Sol_Candidate calculate_best_solution(
    const std::vector<Target>& targets, 
    const std::vector<Ammo>& ammo_list,
    const Position_2D& wind,
    double altitude,
    const ITrajectorySource& trajectory
    );

    