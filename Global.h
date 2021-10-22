#pragma once

constexpr float CAMERA_ASPECT_RATIO = 16.f / 9.f;
constexpr float CAMERA_NEAR_CLIP_PLANE = 0.1f;
constexpr float CAMERA_FAR_CLIP_PLANE = 1e6;
constexpr float CAMERA_VFOV = 45.f;
constexpr float CAMERA_HFOV = CAMERA_ASPECT_RATIO * CAMERA_VFOV;
constexpr float CAMERA_ANGULAR_AREA_SQ_DEG = CAMERA_VFOV * CAMERA_HFOV;
constexpr float CAMERA_ANGULAR_AREA_SQ_ARCSEC = CAMERA_ANGULAR_AREA_SQ_DEG * 60*60 * 60*60;

constexpr float STELLAR_DENSITY = 100.f; //1/pc^3
constexpr float STELLAR_RADIUS = 1.f;
constexpr float ABSOLUTE_VISUAL_MAGNITUDE = 4.83f;

constexpr int STARS_PER_THREAD = 500;
constexpr int THREAD_SLEEP_TIME = 10; //ms

constexpr char CSV_SEPARATOR[] = "\t";
