#include <Arduino.h>
#include <chrono>
#include <string>
#include <vector>
#include "utils.h"
#include "motionTasks.h"
#include "Tracker.h"

// ================= Global variables =================

// Get singleton instance
Tracker &tracker = Tracker::getInstance();

// ================= Prototypes =================
void receiveTask(void *parameter);
void broadcast_position_task(void *parameter);
std::vector<String> splitString(const String &str, char delimiter);
bool isFloat(const String &str);

ErrorStatus getPos(float &az, float &el);

// Mocks TODO : replace with apm methods
ErrorStatus getAz(float &);
ErrorStatus getEl(float &);

// ================= Arduino Framework =================

void setup()
{
    HWSerial.begin(SERIAL_BAUDRATE);

    // Initializes the serial reader
    xTaskCreate(receiveTask, "ReceiveTask", 4096, nullptr, 1, nullptr);
    xTaskCreate(broadcast_position_task, "BroadcastPositionTask", 4096, nullptr, 1, nullptr);

    start_time = 0;
}

void loop()
{
    vTaskDelay(10000 / portTICK_PERIOD_MS); // Just a placeholder delay
    print_info("pong");
}

// ================= TASKS DEFINITION =================

void receiveTask(void *parameter)
{
    while (true)
    {
        if (HWSerial.available() > 0)
        {
            ErrorStatus status; // None type, empty error message
            String commandLine = HWSerial.readStringUntil('\n');

            // Split the command line into tokens
            std::vector<String>
                tokens = splitString(commandLine, ' ');

            // Check if there is at least one token (the command)
            if (tokens.size() > 0)
            {
                String cmd_name = tokens[0]; // First token is the command

                // Add commands here
                if (cmd_name.equals("point_to"))
                {
                    if (tokens.size() < 3)
                    {
                        print_acknowledgement_error("Error: point_to command requires 2 parameters (az, elev).");
                        continue; // Skip to the next iteration
                    }

                    String az_str = tokens[1];
                    String el_str = tokens[2];

                    if (!isFloat(az_str) || !isFloat(el_str))
                    {
                        print_acknowledgement_error("Error: Invalid parameters for point_to. Both must be numbers. Received az = " + az_str + ", el = " + el_str);
                        continue; // Skip to the next iteration
                    }

                    float az = az_str.toFloat();
                    float el = el_str.toFloat();

                    print_acknowledgement("Asking pointing task towards az = " + az_str + ", el = " + el_str);
                    startPointTo(az, el); // Launch task only if all checks pass
                }
                else if (cmd_name.equals("track"))
                {
                    if (tokens.size() < 3)
                    {
                        print_acknowledgement_error("Error: track command requires type (radec, gal, tle) and according parameters.");
                        continue; // Skip to the next iteration
                    }
                    String trackType = tokens[1];
                    if (trackType.equals("radec"))
                    {
                        if (tokens.size() != 4)
                        {
                            print_acknowledgement_error("Error : track radec needs two arguments : ra and dec");
                            continue;
                        }
                        String ra_str = tokens[2];
                        String dec_str = tokens[3];

                        if (!isFloat(ra_str) || !isFloat(dec_str))
                        {
                            print_acknowledgement_error("Error: Invalid parameters for track radec. Both must be numbers. Received ra = " + ra_str + ", dec = " + dec_str);
                            continue; // Skip to the next iteration
                        }

                        float ra = ra_str.toFloat();
                        float dec = dec_str.toFloat();

                        tracker.setEquatorial(ra, dec);
                        tracker.start(TRACK_EQUATORIAL);
                        print_acknowledgement("Asked tracking RADEC ra = " + ra_str + ", dec = " + dec_str);
                    }
                    else if (trackType.equals("gal"))
                    {
                        if (tokens.size() != 4)
                        {
                            print_acknowledgement_error("Error : track gal needs two arguments : l and b");
                            continue;
                        }
                        String l_str = tokens[2];
                        String b_str = tokens[3];

                        if (!isFloat(l_str) || !isFloat(b_str))
                        {
                            print_acknowledgement_error("Error: Invalid parameters for track gal. Both must be numbers. Received l = " + l_str + ", b = " + b_str);
                            continue; // Skip to the next iteration
                        }

                        float l = l_str.toFloat();
                        float b = b_str.toFloat();

                        tracker.setGalactic(l, b);
                        tracker.start(TRACK_GALACTIC);
                        // TODO change completely receiving framework to JSON, in order to get tle correctly
                    }
                    else if (trackType.equals("tle"))
                    {
                    }
                    else
                    {
                        print_acknowledgement_error("Error : invalid track type. Valid types are radec, gal and tle. Received : " + trackType);
                        continue;
                    }
                }
                else if (cmd_name.equals("home"))
                {
                    print_acknowledgement("Asking for homing...");
                    startHoming();
                }
                else if (cmd_name.equals("untangle"))
                {
                    print_acknowledgement("Asking for untangling...");
                    startUntangle();
                }
                else if (cmd_name.equals("standby"))
                {
                    print_acknowledgement("Asking for standby...");
                    startStandby();
                }
                else if (cmd_name.equals("stop"))
                {
                    print_acknowledgement("STOPPING");
                    stopMotionTask();
                }
                else if (cmd_name.equals("ping"))
                {

                    print_acknowledgement("pong");
                }
                else if (cmd_name.equals("get_pos"))
                {
                    // Sends a POSITION message asap, then acknowledges
                    // TODO switch from mock
                    float az;
                    float el;
                    status = getPos(az, el);
                    print_position(az, el, status);
                    print_acknowledgement(cmd_name, status.type);
                }
                else if (cmd_name.equals("get_time"))
                {
                    print_acknowledgement(cmd_name);
                    print_timestamp();
                }
                // For debugging; remove if not needed
                else if (cmd_name.equals("ra2azalt"))
                {
                    if (tokens.size() < 3)
                    {
                        print_acknowledgement_error("Error: ra2azalt command requires 2 parameters (ra, dec).");
                        continue; // Skip to the next iteration
                    }

                    String ra_str = tokens[1];
                    String dec_str = tokens[2];

                    if (!isFloat(ra_str) || !isFloat(dec_str))
                    {
                        print_acknowledgement_error("Error: Invalid parameters for ra2azalt. Both must be numbers. Received az = " + ra_str + ", el = " + dec_str);
                        continue; // Skip to the next iteration
                    }

                    float ra = ra_str.toFloat();
                    float dec = dec_str.toFloat();

                    float az, alt;
                    std::tie(az, alt) = raDecToAltAz(ra, dec);
                    ra_str = String(az, 2U);
                    dec_str = String(alt, 2U);
                    print_acknowledgement("Az: " + ra_str + ", el: " + dec_str);
                }
                else if (cmd_name.equals("sync_time"))
                {
                    if (tokens.size() != 2)
                    {
                        print_acknowledgement_error("Error : sync_time needs an argument : timestamp");
                        continue;
                    }
                    String timestamp = tokens[1];
                    if (isFloat(timestamp))
                    {
                        start_time = timestamp.toDouble(); // In seconds
                        print_acknowledgement("Clock synchronized");
                    }
                    else
                    {
                        print_acknowledgement_error("Error : invalid timestamp. Received : " + timestamp);
                    }
                }
                // Ajouter ici les autres commandes
                else
                {
                    print_acknowledgement_error("Unknown command : " + cmd_name + " wtf les amis");
                }
            }
        }
        // HWSerial.readStringUntil('\n'); // Flushes until endline
        vTaskDelay(10 / portTICK_PERIOD_MS); // Pause pour éviter de monopoliser le processeur
    }
}
// Mock functions to get azimuth and altitude
ErrorStatus getAz(float &az)
{
    // Return a mocked azimuth value (this could be a dynamic value in a real scenario)
    az = 45.0;
    return ErrorStatus(ErrorType::NONE, "Azimuth read successfully"); // Example value
}

ErrorStatus getEl(float &el)
{
    // Return a mocked altitude value (this could be a dynamic value in a real scenario)
    el = 45.0;
    return ErrorStatus(ErrorType::NONE, "Elevation read successfully"); // Example value
}

ErrorStatus getPos(float &az, float &el)
{
    ErrorStatus statusAz;
    ErrorStatus statusEl;

    // TODO handle errors
    statusAz = getAz(az);
    print_msg_filtered(statusAz);
    statusEl = getEl(el);
    print_msg_filtered(statusEl);

    statusAz = hierarchize_status(statusAz, statusEl);
    return statusAz;
}

// Broadcast task that sends position data periodically
void broadcast_position_task(void *parameter)
{
    while (true)
    {
        // Get the current azimuth and altitude values from mock functions

        float az(0);
        float el(0);
        ErrorStatus status;

        status = getPos(az, el);

        // Call the print_position utility function to broadcast the position
        print_position(az, el, status);

        // Wait for the specified delay before broadcasting again
        vTaskDelay(POSITION_BROADCAST_DELAY / portTICK_PERIOD_MS);
    }
}
