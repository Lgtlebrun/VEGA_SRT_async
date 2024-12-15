#include "Tracker.h"

Tracker &Tracker::getInstance()
{
    static Tracker instance; // Created only once
    return instance;
}

// Constructor initializes mutex and tracking mode
Tracker::Tracker() : Tasker(), currentMode(IDLE), trackingTimer(nullptr), target_change_flag(false)
{
    targetAz = HOME_AZ;
    targetEl = HOME_EL;
    positionMutex = xSemaphoreCreateMutex();
}

// Start tracking by initializing tasks and timers
void Tracker::start(TrackingMode mode)
{
    switch (mode)
    {
    case IDLE:
        // Not supposed to be used ever, bust just in case
        stop();
        break;
    default:
        if (currentMode == IDLE)
        {
            bool mode_success(setTrackingMode(mode)); // Change mode
            if (!mode_success)
            { // Check success
                print_error("Failed to start tracking");
                return;
            }
            // START TRACKING
            startTracking(this); // From motionTasks
        }
        else
        {
            bool mode_success(setTrackingMode(mode)); // Change mode
            if (!mode_success)
            { // Check success
                print_error("Failed to change tracking mode");
                return;
            }
            target_change_flag = true; // Raises flag to kill point_to
        }
        break;
    }
}

// Stop tracking by deleting the task and stopping the timer
void Tracker::stop()
{
    if (currentMode != IDLE)
    {
        setTrackingMode(IDLE);
    }

    stopTrackingTimer(); // Stops the timer (in case it is on)
    // Note the task itself will be stopped by motionTasks internal logic
}

// Set tracking mode (e.g., satellite, galactic, equatorial)
bool Tracker::setTrackingMode(TrackingMode mode)
{
    // TODO Create checks on coords
    if (currentMode == mode)
    {
        return true;
    }

    xSemaphoreTake(positionMutex, portMAX_DELAY);
    currentMode = mode;
    xSemaphoreGive(positionMutex);
    return true;
}

// Update target coordinates based on the tracking mode
void Tracker::updateTargetCoordinates()
{
    if (currentMode == IDLE)
    {
        return; // No need to update in IDLE mode
    }

    float az, el;

    switch (currentMode)
    {
    case TRACK_SATELLITE:
        updateFromTLE(az, el);
        break;

    case TRACK_GALACTIC:
        updateFromGalactic(az, el);
        break;

    case TRACK_EQUATORIAL:
        std::tie(az, el) = Equatorial2AzAlt(ra, dec);
        break;

    default:
        return; // No update needed in IDLE mode
    }

    // Check if position is valid before updating
    if (isValidPosition(az, el))
    {
        xSemaphoreTake(positionMutex, portMAX_DELAY);
        targetAz = az;
        targetEl = el;
        xSemaphoreGive(positionMutex);
    }
    else
    {
        String az_str(az);
        String el_str(el);
        print_warning("Invalid position. Az : " + az_str + ", El : " + el_str);
    }
}

// Tracking task to move the antenna
void Tracker::task()
{
    if (currentMode == IDLE)
    {
        return; // No need to run in IDLE mode
    }

    bool status = true;

    setupTrackingTimer(); // Set up the timer to update coordinates

    while (true)
    {
        print_info("DEBUG : tracking task here");
        if (ulTaskNotifyTake(pdTRUE, 0))
        {
            print_info("Stopping tracker...");
            stop();
            print_info("Task gracefully canceled.");

            // Notify completion
            if (startPendingTaskHandle != nullptr)
            {
                xTaskNotifyGive(startPendingTaskHandle);
            }
            print_info("DEBUG : Notification sent.");
            vTaskDelete(NULL); // Delete this task
        }
        float az, el;
        xSemaphoreTake(positionMutex, portMAX_DELAY);
        // TODO get current position
        az = 50;
        el = 50;
        xSemaphoreGive(positionMutex);

        // Check if movement is needed
        if (needsMovement(az, el))
        {
            // Send command to point to new coordinates
            print_info("DEBUG : start PointTo");
            status = pointTo(targetAz, targetEl, &target_change_flag);
            print_info("DEBUG : pointTo returned");
            target_change_flag = false;
            if (!status)
            {
                print_info("Task gracefully canceled.");
                // Notify completion
                if (startPendingTaskHandle != nullptr)
                {
                    xTaskNotifyGive(startPendingTaskHandle);
                }
                vTaskDelete(NULL);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(TRACK_DELAY)); // Adjust the frequency as needed
    }
}

void Tracker::setTLE(const String &newTLE)
{
    if (xSemaphoreTake(positionMutex, portMAX_DELAY))
    {
        tle = newTLE;
        xSemaphoreGive(positionMutex);
    }
}

void Tracker::setEquatorial(float newRa, float newDec)
{
    if (xSemaphoreTake(positionMutex, portMAX_DELAY))
    {
        ra = newRa;
        dec = newDec;
        xSemaphoreGive(positionMutex);
    }
    print_info("DEBUG : RADEC updated");
}

void Tracker::setGalactic(float newL, float newB)
{
    if (xSemaphoreTake(positionMutex, portMAX_DELAY))
    {
        l = newL;
        b = newB;
        xSemaphoreGive(positionMutex);
    }
}

// Update the coordinates based on TLE data (satellite tracking)
void Tracker::updateFromTLE(float &az, float &el)
{
    // Perform TLE-based coordinate calculation (details omitted)
}

// Update the coordinates for galactic tracking
void Tracker::updateFromGalactic(float &az, float &el)
{
    // Perform galactic-based coordinate calculation (details omitted)
}

// Validate if the coordinates are within valid range
bool Tracker::isValidPosition(float az, float el)
{
    // Check if azimuth and elevation are within valid bounds
    return (az >= AZ_MIN && az <= AZ_MAX && el >= EL_MIN && el <= EL_MAX);
}

// Check if movement is required
bool Tracker::needsMovement(float az, float el)
{
    // You can add some logic to avoid unnecessary movements, for example:
    return (abs(targetAz - az) > MOTION_MIN || abs(targetEl - el) > MOTION_MIN);
}

// Periodically update the coordinates (this function is called by the timer)
void updateCoordinatesPeriodically(TimerHandle_t xTimer)
{
    Tracker::getInstance().updateTargetCoordinates();
}

// Timer setup to periodically call updateCoordinatesPeriodically
void Tracker::setupTrackingTimer()
{
    if (trackingTimer != nullptr)
    {
        stopTrackingTimer();
        setupTrackingTimer();
        return;
    }

    const TickType_t xFrequency = pdMS_TO_TICKS(100); // 100ms interval

    trackingTimer = xTimerCreate("TrackingTimer", xFrequency, pdTRUE, this, updateCoordinatesPeriodically);
    if (trackingTimer != NULL)
    {
        xTimerStart(trackingTimer, 0);
    }
    else
    {
        print_error("Failed to create timer for tracking.");
    }
}

void Tracker::stopTrackingTimer()
{
    if (trackingTimer != nullptr)
    {
        // Stop and delete the timer
        if (xTimerStop(trackingTimer, 0) == pdFAIL)
        {
            print_error("Failed to stop the tracking timer.");
        }

        if (xTimerDelete(trackingTimer, 0) == pdFAIL)
        {
            print_error("Failed to delete the tracking timer.");
        }
        else
        {
            trackingTimer = nullptr;
        }
    }
    else
    {
        print_warning("Tracking timer is not running.");
    }
}
