#include "motionTasks.h"

TaskHandle_t motionTaskHandle = nullptr;                 // Tâche point active
TaskHandle_t startPendingTaskHandle = nullptr;           // Handle for startPointTo task
SemaphoreHandle_t motionMutex = xSemaphoreCreateMutex(); // Créer le mutex

// TODO Mocks ; update with real tasks (apm->truc)
bool pointTo(float const &az, float const &el, bool *stop_flag)
{
    for (size_t i(0); i < 50; i++)
    {
        // Vérifier l'annulation
        if (ulTaskNotifyTakeIndexed(0, pdTRUE, 0))
        {
            print_info("Point_to task gracefully canceled.");
            return false; // Exit for loop, stop tracking if applicable
        }

        if ((stop_flag != nullptr) && (*stop_flag))
        {
            print_info("Point_to task gracefully canceled.");
            return true; // Exit for loop, continue tracking if applicable
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
        print_info("Mock pointing task to az=" + String(az) + ", el=" + String(el));
    }
    print_info("For loop exited .");
    // apm->point_to(az, elev);  // Commande de pointage asynchrone
    return true;
}

void pointToTask(void *parameters)
{
    unsigned long currentTime = millis();
    if (currentTime - lastPointCommandTime < 1000)
    {
        print_info("Command ignored: Anti-spam active.");
        return; // Ignore les commandes trop proches
    }

    lastPointCommandTime = currentTime;

    auto *args = (float *)parameters;
    float az = args[0];
    float el = args[1];

    pointTo(az, el);

    // Notify completion
    if (startPendingTaskHandle != nullptr)
    {
        xTaskNotifyGive(startPendingTaskHandle);
    }
    print_info("Notification sent.");
    delete[] args;     // Libérer la mémoire utilisée
    vTaskDelete(NULL); // Delete this task
}

void homingTask(void *parameters)
{
    unsigned long currentTime = millis();
    if (currentTime - lastPointCommandTime < 1000)
    {
        print_info("Command ignored: Anti-spam active.");
        return; // Ignore les commandes trop proches
    }

    lastPointCommandTime = currentTime;
    for (size_t i(0); i < 50; i++)
    {
        // Vérifier l'annulation
        if (ulTaskNotifyTake(pdTRUE, 0))
        {
            print_info("Task gracefully canceled.");
            break; // Exit for loop
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
        print_info("Mock homing task here");
    }
    print_info("For loop exited .");
    // apm->point_to(az, elev);  // Commande de pointage asynchrone

    // Notify completion
    if (startPendingTaskHandle != nullptr)
    {
        xTaskNotifyGive(startPendingTaskHandle);
    }
    print_info("Notification sent.");

    vTaskDelete(NULL); // Delete this task
}

void standbyTask(void *parameters)
{
    unsigned long currentTime = millis();
    if (currentTime - lastPointCommandTime < 1000)
    {
        print_info("Command ignored: Anti-spam active.");
        return; // Ignore les commandes trop proches
    }

    lastPointCommandTime = currentTime;
    for (size_t i(0); i < 50; i++)
    {
        // Vérifier l'annulation
        if (ulTaskNotifyTake(pdTRUE, 0))
        {
            print_info("Task gracefully canceled.");
            break; // Exit for loop
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
        print_info("Mock standby task here");
    }
    print_info("For loop exited .");
    // apm->point_to(az, elev);  // Commande de pointage asynchrone

    // Notify completion
    if (startPendingTaskHandle != nullptr)
    {
        xTaskNotifyGive(startPendingTaskHandle);
    }
    print_info("Notification sent.");

    vTaskDelete(NULL); // Delete this task
}

void untangleTask(void *parameters)
{
    unsigned long currentTime = millis();
    if (currentTime - lastPointCommandTime < 1000)
    {
        print_info("Command ignored: Anti-spam active.");
        return; // Ignore les commandes trop proches
    }

    lastPointCommandTime = currentTime;
    for (size_t i(0); i < 50; i++)
    {
        // Vérifier l'annulation
        if (ulTaskNotifyTake(pdTRUE, 0))
        {
            print_info("Task gracefully canceled.");
            break; // Exit for loop
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
        print_info("Mock untangle task here");
    }
    print_info("For loop exited .");
    // apm->point_to(az, elev);  // Commande de pointage asynchrone

    // Notify completion
    if (startPendingTaskHandle != nullptr)
    {
        xTaskNotifyGive(startPendingTaskHandle);
    }
    print_info("Notification sent.");

    vTaskDelete(NULL); // Delete this task
}

// Declare the external task function
void trackingTask(void *parameters)
{
    // Cast the parameters to the Tracker object
    Tasker *trackerInstance = static_cast<Tasker *>(parameters);

    // Now you can call the tracking logic in the Tracker class
    trackerInstance->task();
}

void stopMotionTask()
{
    if (xSemaphoreTake(motionMutex, portMAX_DELAY))
    {
        if (motionTaskHandle != nullptr)
        {
            print_warning("DEBUG : Previous task still running. Attempting to stop it");
            startPendingTaskHandle = xTaskGetCurrentTaskHandle(); // Current task waiting for cancellation
            xTaskNotifyGive(motionTaskHandle);                    // Notify to cancel

            // Wait for confirmation of cancellation
            if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(TASK_TIMEOUT)) == 0)
            {
                vTaskDelete(motionTaskHandle); // Delete this task
                print_warning("Task stop timed out.");
            }
            else
            {
                print_info("Motion task stopped.");
            }

            motionTaskHandle = nullptr; // Reset task handle
        }

        xSemaphoreGive(motionMutex); // Release the mutex
    }
}

void startMotionTask(TaskFunction_t taskFunction, void *args, const char *taskName)
{

    // First stop previous task if any
    stopMotionTask();

    // Take/Wait for the lock
    if (xSemaphoreTake(motionMutex, portMAX_DELAY))
    {
        // Start the new task
        if (xTaskCreate(taskFunction, taskName, 4096, args, 1, &motionTaskHandle) == pdPASS)
        {
            print_info(String("New task started: ") + taskName);
        }
        else
        {
            print_error(String("Failed to create task: ") + taskName);
            delete[] args; // Free memory if task creation fails
        }

        xSemaphoreGive(motionMutex); // Release mutex
    }
}

void startPointTo(float az, float elev)
{
    float *args = new float[2]{az, elev};
    startMotionTask(pointToTask, args, "PointTo");
}

void startHoming()
{
    startMotionTask(homingTask, nullptr, "Homing");
}

void startStandby()
{
    startMotionTask(standbyTask, nullptr, "Standby");
}

void startUntangle()
{
    startMotionTask(untangleTask, nullptr, "Untangle");
}

void startTracking(Tasker *tracker)
{
    startMotionTask(trackingTask, tracker, "Tracking");
}