#ifndef TRACKING_H
#define TRACKING_H

#include "Tasker.h"
#include "Message.h"
#include "define.h"
#include "motionTasks.h"

enum TrackingMode
{
    IDLE,
    TRACK_SATELLITE,
    TRACK_GALACTIC,
    TRACK_EQUATORIAL
};

class Tracker : public Tasker
{
public:
    static Tracker &getInstance(); // Yields singleton

    void start(TrackingMode mode);
    void stop();
    virtual void task();
    bool setTrackingMode(TrackingMode mode);

    // Public methods for setting targets
    void setTLE(const String &newTLE);         // Set TLE for satellite tracking
    void setEquatorial(double ra, double dec); // Set equatorial coords
    void setGalactic(double l, double b);      // Set galactic coords

    void updateTargetCoordinates();

private:
    SemaphoreHandle_t positionMutex;
    TimerHandle_t trackingTimer;
    double targetAz, targetEl;

    TrackingMode currentMode;
    bool target_change_flag;
    double ra, dec, l, b;
    String tle;

    // Constructors (singleton)
    Tracker();                                    // Private constructor
    Tracker(const Tracker &) = delete;            // Delete copy constructor
    Tracker &operator=(const Tracker &) = delete; // Delete assignment operator

    // Method to update target coordinates based on current mode
    void updateFromTLE(double &az, double &el);
    void updateFromGalactic(double &az, double &el);

    // Method to check if position is valid
    bool isValidPosition(double az, double el);

    // Method to check if movement is needed
    bool needsMovement(double az, double el);

    // Timer to periodically update coordinates
    void setupTrackingTimer();
    void stopTrackingTimer();
};

void updateCoordinatesPeriodically(TimerHandle_t xTimer);

#endif