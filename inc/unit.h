#pragma once

#include <limits>

typedef float unit;
constexpr unit UNIT(1.0f);        // a '1' in engines units
constexpr unit ZERO(0.0f);        // a '0' in engines units
constexpr unit EPSILON(1e-5f);    // near zero cutoff point to avoid random sign flips
constexpr unit DIST_ATTEN(3e-5f); // light attenuation factor according to object's distance from light source
constexpr unit LIGHT_DIR_X(ZERO);
constexpr unit LIGHT_DIR_Y(ZERO);
constexpr unit LIGHT_DIR_Z(UNIT);
constexpr unit LIGHT_POS_X(ZERO);
constexpr unit LIGHT_POS_Y(20.0f);
constexpr unit LIGHT_POS_Z(75.0f);
constexpr unit CAM_POS_X(ZERO);
constexpr unit CAM_POS_Y(ZERO);
constexpr unit CAM_POS_Z(1000.0f);
constexpr unit FOCAL_LEN(2000.0f);
constexpr unit NEAR_EPS(UNIT);
constexpr unit DEG(180.0f);
constexpr unit PI(3.14159265358979323846f);
constexpr unit DEG_TO_RAD = PI / DEG;
constexpr unit NORMAL_SCALE_FACTOR(50.0f);
constexpr int FORWARD_LOOKING_SIGN(-1);
constexpr int DEF_LOOP_NUM(-1); // endless
constexpr int DEF_LOOP_DELAY(20);

// Scale factor for EPSILON when computing the squared tolerance
// for detecting nearly-duplicate polygon vertices.
static constexpr unit CONSEC_TOLERANCE_SCALE = 10.0f;
