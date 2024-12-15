#include "utils.h"

extern

    std::vector<String>
    splitString(const String &str, char delimiter)
{
    // Function to split a string by a delimiter

    std::vector<String> tokens;
    int start = 0;
    int end = str.indexOf(delimiter);

    while (end != -1)
    {
        tokens.push_back(str.substring(start, end));
        start = end + 1;
        end = str.indexOf(delimiter, start);
    }
    tokens.push_back(str.substring(start)); // Add the last token
    return tokens;
}

// Function to check if a string can be converted to float
bool isFloat(const String &str)
{
    char *endptr;
    str.toFloat(); // Trigger conversion
    strtod(str.c_str(), &endptr);
    return (*endptr == '\0'); // Check if the entire string was consumed
}

double getCurrentTime()
{
    using namespace std::chrono;
    auto now = system_clock::now();
    auto duration = now.time_since_epoch();
    auto microseconds_duration = duration_cast<microseconds>(duration).count();
    return (static_cast<double>(microseconds_duration)) / 1.e6f + start_time;
}

String getCurrentTimestamp()
{
    double time_in_seconds = getCurrentTime();
    return String(time_in_seconds, 6U); // Precision to the microsecond
}

// Calculate Greenwich Mean Sidereal Time
float calculateGMST(time_t now)
{
    std::tm *utc = std::gmtime(&now);
    int dayOfYear = utc->tm_yday + 1; // Day of the year
    float hours = utc->tm_hour + utc->tm_min / 60.0f + utc->tm_sec / 3600.0f;

    // Days since J2000.0
    float d = dayOfYear + (hours / 24.0f);
    float T = (d + (utc->tm_year - 100) * 365.25f - 0.5f) / 36525.0f;

    // GMST in degrees
    float GMST = 280.46061837f + 360.98564736629f * d + 0.000387933f * T * T - T * T * T / 38710000.0f;
    return fmod(GMST, 360.0f); // Normalize
}

// Convert equatorial coordinates to horizontal coordinates
std::tuple<float, float> Equatorial2AzAlt(float const &ra, float const &dec)
{
    double now = getCurrentTime();
    float az, el;
    // Observer's coordinates
    float observerLat = 45.0f;  // Example: Latitude in degrees
    float observerLon = -93.0f; // Example: Longitude in degrees

    // Calculate Local Sidereal Time (LST)
    float GMST = calculateGMST(now);
    float LST = fmod(GMST + observerLon, 360.0f);

    // Calculate Hour Angle
    float HA = fmod(LST - ra, 360.0f);

    // Convert to radians
    float HA_rad = HA * DEG_TO_RAD;
    float Dec_rad = dec * DEG_TO_RAD;
    float Lat_rad = observerLat * DEG_TO_RAD;

    // Calculate elevation
    float sin_el = sin(Dec_rad) * sin(Lat_rad) + cos(Dec_rad) * cos(Lat_rad) * cos(HA_rad);
    el = asin(sin_el) * RAD_TO_DEG;

    // Calculate azimuth
    float cos_az = (sin(Dec_rad) - sin(el * DEG_TO_RAD) * sin(Lat_rad)) /
                   (cos(el * DEG_TO_RAD) * cos(Lat_rad));
    az = acos(cos_az) * RAD_TO_DEG;

    // Adjust azimuth based on quadrant
    if (sin(HA_rad) > 0)
        az = 360.0f - az;

    return std::make_tuple(az, el);
}