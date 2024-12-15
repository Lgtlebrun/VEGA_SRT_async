#ifndef UTILS_H
#define UTILS_H
#include <Arduino.h>
#include <vector>
#include <string>
#include <chrono>
#include "define.h"

std::vector<String> splitString(const String &str, char delimiter);

// Function to check if a string can be converted to float
bool isFloat(const String &str);

double getCurrentTime();
String getCurrentTimestamp();
float calculateGMST(time_t now);
std::tuple<float, float> Equatorial2AzAlt(float const &ra, float const &dec);
#endif