//
// Created by Ben Giacalone on 9/5/2021.
//

#ifndef GYMENV_MOUNTAIN_CAR_H
#define GYMENV_MOUNTAIN_CAR_H

#include <opencv2/core/mat.hpp>
#include "../env.h"
#include "../common/rendering.h"

namespace gym {
    /**
     * A CPP implementation of the Mountain Car environment, based on http://incompleteideas.net/MountainCar/MountainCar1.cp.
     *
     * The observation space is a Box space of 2:
     * 0:                   position [-1.2, 0.6]
     * 1:                   velocity [-0.07, 0.07]
     *
     * The action space is a Discrete space of 3:
     * 0:                   go left
     * 1:                   do nothing
     * 2:                   go right
     */

    class MountainCarEnv : public Env< std::vector<double>, int> {
    private:
        // Constants
        static constexpr float MIN_POS = -1.2;
        static constexpr float MAX_POS = 0.6;
        static constexpr float MAX_VEL = 0.07;
        static constexpr float GOAL_POS = 0.5;
        static constexpr float FORCE = 0.001;
        static constexpr float GRAVITY = 0.0025;

        // Environment variables
        std::vector<double> m_State{};

        // Viewer state
        std::unique_ptr<Viewer> m_Viewer;

        Color m_CarColor;
        FilledPolygon m_CarGeom;
        Transform m_CarTransform;

        Color m_TrackColor;
        PolyLine m_TrackGeom;

        float goal_velocity;

    public:

        explicit MountainCarEnv(int goal_velocity=0);

        StepT step(ActionT const& action)  noexcept final;

        ObservationT reset()  noexcept final;

        void render() final;

    };
}


#endif //GYMENV_MOUNTAIN_CAR_H
