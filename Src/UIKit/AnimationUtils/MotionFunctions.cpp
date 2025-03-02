#include "Common/Precompile.h"

#include "UIKit/AnimationUtils/MotionFunctions.h"

namespace d14engine::uikit::animation_utils
{
    float motionAccelUniformDecel(float dx, float dt, float s, float t1, float t2)
    {
        //------------------------------------------------------------------------
        //        accel (s2)              uniform              decel (s2)
        // start -----------> waypoint1 ----------> waypoint2 -----------> end
        //         (t2 / 2)                 t1                  (t2 / 2)
        //------------------------------------------------------------------------
        // dx --- current position
        // dt --- elapsed time of last frame
        // s  --- total distance
        // t1 --- uniform speed motion time
        // t2 --- variable speed motion time
        //------------------------------------------------------------------------
        // The following derivation requires a bit of high school physics knowledge.
        //
        // Since both ends are uniformly accelerated motion, the average speed of
        // the entire motion (which is essentially the speed of the uniform motion)
        // can be simply obtained by the following formula:
        //
        // v = s / (t1 + t2)
        //
        // Next, it is easy to obtain the distance of uniformly accelerated motion
        // (i.e. distance from start to waypoint1, equals from waypoint2 to end):
        //
        // s2 = (v * t2) / 2
        //
        // We have the quadratic relationship between displacement and velocity:
        //
        // (dx / s2) = (v2 / v)^2
        //
        // Then, the current speed can be calculated based on the current position
        // (especially the variable speed during the uniformly accelerated motion):
        //
        // v2 = sqrt((dx * v^2) / s2)
        // v2 = sqrt((dx * v^2) / ((v2 * t2) / 2))
        // v2 = sqrt((2 * dx * v) / t2)
        //
        // Finally, just perform the discrete animation: multiply the current speed
        // by the elapsed time of last frame to get the movement in current frame:
        //
        // dx += v * dt
        // dx += v2 * dt
        //------------------------------------------------------------------------

        if (dx < s)
        {
            float v = s / (t1 + t2);
            float s2 = (v * t2) / 2;

            if (dx < s2)
            {
                // Make the object start moving.
                constexpr float d = 0.1f;
                v = sqrt(2 * v * (dx + d) / t2);
            }
            else if (dx > (s - s2))
            {
                v = sqrt(2 * v * (s - dx) / t2);
            }
            dx += v * dt;
        }
        return std::clamp(dx, 0.0f, s);
    }
}
