#ifndef RW_CUBE_CAMERA_HPP
#define RW_CUBE_CAMERA_HPP

#include <numbers>
#include <linmath.h>

namespace rw_cube {

// NOLINTBEGIN
struct Camera {
    // every value represents camera in WORLD space
    vec3 position_                            = { 0.F, 0.F, 0.F};

    vec3 negative_looking_direction_angles_ = { 
        std::numbers::pi_v<float>,      // vector (0,0,1) is NEGATIVE 
                                        // so starting point is vector
                                        // (0,0,-1). vector (0,0,1) is 
                                        // rotated around X axis by 180
        -std::numbers::pi_v<float>/2.F, // because vector (-1,0,0) is 
                                        // x axis vector so -90 in x gives
                                        // (0,0,1)
        0.F
    };

    vec3 world_up_direction_                  = { 0.F, 1.F, 0.F};
    vec3 negative_looking_direction_          = { 0.F, 0.F, 1.F};
    vec3 right_direction_                     = {-1.F, 0.F, 0.F};
    vec3 up_direction_                        = { 0.F, 1.F, 0.F};

    void lookAt(mat4x4 result);
    void rotate(float x_angle, float y_angle, float z_angle);
    void rotateXYPlane(float x_vec, float y_vec);
    void move(float x_mv, float y_mv, float z_mv);
};
// NOLINTEND

}


#endif