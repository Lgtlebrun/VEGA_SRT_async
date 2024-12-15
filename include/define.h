#ifndef DEFINE_H
#define DEFINE_H
#include <Arduino.h>
#include <chrono>

#define HWSerial Serial
#define SERIAL_BAUDRATE 921600
#define POSITION_BROADCAST_DELAY 5000

#define TASK_TIMEOUT 5000

#define TRACK_DELAY 2000
#define MOTION_MIN 0.1f

#define AZ_MAX 360
#define AZ_MIN 0
#define EL_MAX 89
#define EL_MIN 1

#define HOME_AZ 0
#define HOME_EL 89

extern inline double start_time = 0; // In seconds
#endif