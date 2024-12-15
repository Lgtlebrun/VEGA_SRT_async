#ifndef MOTIONTASKS_H
#define MOTIONTASKS_H
#include "define.h"
#include "Message.h"
#include "Tasker.h"
#include <Arduino.h>

typedef void (*TaskFunction_t)(void *);

inline unsigned long lastPointCommandTime(0);

extern SemaphoreHandle_t motionMutex; // Mutex de protection

extern TaskHandle_t motionTaskHandle;       // Tâche point active
extern TaskHandle_t startPendingTaskHandle; // Handle for startPointTo task

bool pointTo(float const &az, float const &el, bool *stop_flag = nullptr);
void pointToTask(void *parameters);
void homingTask(void *parameters);
void untangleTask(void *parameters);
void standbyTask(void *parameters);
void untangleTask(void *parameters);
void trackingTask(void *parameters);

void startMotionTask(TaskFunction_t taskFunction, void *args, const char *taskName);
void startPointTo(float az, float elev);
void startHoming();
void startStandby();
void startUntangle();
void stopMotionTask();
void startTracking(Tasker *tracker);

#endif