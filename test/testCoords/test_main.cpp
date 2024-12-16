#include <iostream>
#include <string>
#include <cmath>
#include <chrono>
#include <tuple>
#define OBS_LAT 46.5194444
#define OBS_LON 6.565
#define OBS_HEIGHT 411.0
using namespace std;

float start_time = 0;
float PI = 3.14159;
float J2000 = 2451545.0;
double DEG_TO_RAD = PI / 180.;
double RAD_TO_DEG = 180 / PI;
constexpr double SECONDS_IN_DAY = 86400.0;
constexpr double UNIX_EPOCH_JD = 2440587.5;

// Convert Unix timestamp to Julian date
double unixTimeToJD(double const &unixTime)
{
    return unixTime / SECONDS_IN_DAY + UNIX_EPOCH_JD;
}

// Compute Greenwich Mean Sidereal Time
double computeGMST(double const &jd)
{
    double d = jd - J2000;
    double gmst = 18.697375 + 24.065709824279 * d;
    gmst = fmod(gmst, 24.0);
    return (gmst < 0) ? gmst + 24.0 : gmst;
}

// Nutation and Equation of Equinoxes
double computeEquationOfEquinoxes(double const &d)
{
    double omega = 125.04 - 0.052954 * d;
    double L = 280.47 + 0.98565 * d;
    double epsilon = 23.4393 - 0.0000004 * d;

    double deltaPsi = -0.000319 * sin(omega * DEG_TO_RAD) - 0.000024 * sin(2 * L * DEG_TO_RAD);
    return deltaPsi * cos(epsilon * DEG_TO_RAD);
}

double getCurrentTime()
{
    using namespace std::chrono;
    auto timestamp = system_clock::now();
    auto duration = timestamp.time_since_epoch();
    auto microseconds_duration = duration_cast<microseconds>(duration).count();
    return (static_cast<double>(microseconds_duration)) / 1.e6f + start_time;
}

// Convert RA/DEC to Alt/Az
std::tuple<double, double> raDecToAltAz(double ra, double dec, double unixTime)
{
    if (unixTime + 1 < 1e6f)
    {
        unixTime = getCurrentTime();
    }
    double jd = unixTimeToJD(unixTime);
    double d = jd - J2000;

    double gmst = computeGMST(jd);
    double eqeq = computeEquationOfEquinoxes(d);
    double gast = gmst + eqeq;

    double lst = gast + OBS_LON / 15.0;
    lst = fmod(lst, 24.0);
    if (lst < 0)
        lst += 24.0;

    double hourAngle = (lst * 15.0 - ra);
    hourAngle *= DEG_TO_RAD;

    double lat;

    lat = OBS_LAT * DEG_TO_RAD;
    dec *= DEG_TO_RAD;

    double sinAlt = sin(lat) * sin(dec) + cos(lat) * cos(dec) * cos(hourAngle);
    double alt = asin(sinAlt);

    double cosAz = (sin(dec) - sin(lat) * sin(alt)) / (cos(lat) * cos(alt));
    double az = acos(cosAz);

    if (sin(hourAngle) > 0)
        az = 2 * M_PI - az;

    return std::make_tuple(az * RAD_TO_DEG, alt * RAD_TO_DEG);
}
int main(int argc, char **argv)
{
    while (true)
    {

        string ra_str, dec_str;
        cout << "Enter RA";
        cin >> ra_str;
        cout << "Enter dec";
        cin >> dec_str;

        float ra, dec;
        ra = stof(ra_str);
        dec = stof(dec_str);

        float az, alt;
        std::tie(az, alt) = raDecToAltAz(ra, dec, getCurrentTime());
    }
    return 0;
}