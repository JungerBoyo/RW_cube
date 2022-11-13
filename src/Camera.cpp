#include "Camera.hpp"

#include <cmath>

using namespace rw_cube;

auto wrap(auto value, auto abs_limit, auto wrap_to) {
	return std::abs(value) > abs_limit ? wrap_to : value;
}

// NOLINTBEGIN
static void rotateVec3InPlace(vec3 result, const vec3 angles, const vec3 vec, const mat3x3 basis) {
    static constexpr vec3 x_axis = {1.F, 0.F, 0.F};
    static constexpr vec3 y_axis = {0.F, 1.F, 0.F};
    static constexpr vec3 z_axis = {0.F, 0.F, 1.F};

    vec3 this_x_axis;
    mat3x3_mul_vec3(this_x_axis, basis, x_axis);
    vec3 this_y_axis;
    mat3x3_mul_vec3(this_y_axis, basis, y_axis);
    vec3 this_z_axis;
    mat3x3_mul_vec3(this_z_axis, basis, z_axis);

    quat q_this_x_axis;
    quat_rotate(q_this_x_axis, angles[0], this_x_axis);
    quat q_this_y_axis;
    quat_rotate(q_this_y_axis, angles[1], this_y_axis);
    quat q_this_z_axis;
    quat_rotate(q_this_z_axis, angles[2], this_z_axis);

    quat q_vec = {vec[0], vec[1], vec[2], 1.F};
    quat_mul_in_place(q_vec, q_this_x_axis, q_vec); 
    quat_conj(q_this_x_axis, q_this_x_axis);
    quat_mul_in_place(q_vec, q_vec, q_this_x_axis);

    quat_mul_in_place(q_vec, q_this_y_axis, q_vec); 
    quat_conj(q_this_y_axis, q_this_y_axis);
    quat_mul_in_place(q_vec, q_vec, q_this_y_axis);

    quat_mul_in_place(q_vec, q_this_z_axis, q_vec); 
    quat_conj(q_this_z_axis, q_this_z_axis);
    quat_mul_in_place(q_vec, q_vec, q_this_z_axis);

    result[0] = q_vec[0];
    result[1] = q_vec[1];
    result[2] = q_vec[2];
}

void Camera::lookAt(mat4x4 result) {
    // compute right direction
    vec3_mul_cross(
        right_direction_,
        negative_looking_direction_,
        world_up_direction_
    );
    vec3_norm(right_direction_, right_direction_);

    // compute up direction
    vec3_mul_cross(
        up_direction_,
        right_direction_,
        negative_looking_direction_
    );
    vec3_norm(up_direction_, up_direction_);

    // construct lookAt matrix (camera basis matrix * camera translation matrix)
    result[0][0] = right_direction_[0];
    result[0][1] = up_direction_[0];
    result[0][2] = -negative_looking_direction_[0];
    result[0][3] = 0.F;
    
    result[1][0] = right_direction_[1];
    result[1][1] = up_direction_[1];
    result[1][2] = -negative_looking_direction_[1];
    result[1][3] = 0.F;

    result[2][0] = right_direction_[2];
    result[2][1] = up_direction_[2];
    result[2][2] = -negative_looking_direction_[2];
    result[2][3] = 0.F;

    result[3][0] = -vec3_mul_inner(right_direction_, position_);
    result[3][1] = -vec3_mul_inner(up_direction_, position_);
    result[3][2] = vec3_mul_inner(negative_looking_direction_, position_);
    result[3][3] = 1.F;
}

void Camera::rotate(float x_angle, float y_angle, float z_angle) {
    mat3x3 basis = {
        {right_direction_           [0], right_direction_           [1], right_direction_           [2]},
        {up_direction_              [0], up_direction_              [1], up_direction_              [2]},
        {negative_looking_direction_[0], negative_looking_direction_[1], negative_looking_direction_[2]}
    };
    vec3 angles = {x_angle, y_angle, z_angle};

    rotateVec3InPlace(negative_looking_direction_, angles, negative_looking_direction_, basis);
    vec3_norm(negative_looking_direction_, negative_looking_direction_);
}

void Camera::rotateXYPlane(float x_vec, float y_vec) {
    negative_looking_direction_angles_[0] = wrap(
        negative_looking_direction_angles_[0] + y_vec, 
        2.F * std::numbers::pi_v<float>, 
        0.F
    );
    negative_looking_direction_angles_[1] = wrap(
        negative_looking_direction_angles_[1] + x_vec, 
        2.F * std::numbers::pi_v<float>, 
        0.F
    );
    negative_looking_direction_[0] =  
        // rot around Y in XZ plane (x value)             // scale by rot around X becuase vec len changed from x/z perspective
        cos(negative_looking_direction_angles_[1]) * cos(negative_looking_direction_angles_[0]);
    negative_looking_direction_[1] =  
        // rot around X in Y(X/Z) plane (y value) 
        sin(negative_looking_direction_angles_[0]);
    negative_looking_direction_[2] = 
        // rot around Y in XZ plane (z value)             // scale by rot around X becuase vec len changed from x/z perspective
        sin(negative_looking_direction_angles_[1]) * cos(negative_looking_direction_angles_[0]);
    vec3_norm(negative_looking_direction_, negative_looking_direction_);
}
void Camera::move(float x_mv, float y_mv, float z_mv) {
    mat3x3 basis = {
        {right_direction_           [0], right_direction_           [1], right_direction_           [2]},
        {up_direction_              [0], up_direction_              [1], up_direction_              [2]},
        {negative_looking_direction_[0], negative_looking_direction_[1], negative_looking_direction_[2]}
    };

    vec3 mv_vec = {x_mv, y_mv, z_mv};
    vec3 this_mv_vec = {x_mv, y_mv, z_mv};
    mat3x3_mul_vec3(this_mv_vec, basis, mv_vec);

    vec3_add(position_, position_, this_mv_vec);
}
// NOLINTEND