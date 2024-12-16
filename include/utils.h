#ifndef UTILS_H
#define UTILS_H
#include <Arduino.h>
#include <vector>
#include <string>
#include <chrono>
#include "define.h"

extern "C"
{
#include <sofa.h> // Include the SOFA header
#include <sofam.h>
}

std::vector<String> splitString(const String &str, char delimiter);
constexpr double SECONDS_IN_DAY = 86400.0;
constexpr double UNIX_EPOCH_JD = 2440587.5;
// North Galactic Pole coordinates (J2000)
constexpr double GALACTIC_NGP_RA = 192.85948;  // degrees
constexpr double GALACTIC_NGP_DEC = 27.12825;  // degrees
constexpr double GALACTIC_NORTH_LON = 122.932; // degrees

// Function to check if a string can be converted to float
bool isFloat(const String &str);

double getCurrentTime();
String getCurrentTimestamp();

// Function to compute Earth Rotation Angle (ERA) from UNIX timestamp
double unixTimeToJD(double const &unixTime);
double computeGMST(double const &jd);
double computeEquationOfEquinoxes(double const &d);

std::tuple<double, double> raDecToAltAz(
    double ra, double dec, double unixTime = -1.);
std::tuple<double, double> galacticToEquatorial(double l, double b);
std::tuple<double, double> galacticToAltAz(double l, double b, double unixTime = -1.);
#endif