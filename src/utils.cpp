#include "utils.h"

extern std::vector<String> splitString(const String &str, char delimiter)
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
    auto timestamp = system_clock::now();
    auto duration = timestamp.time_since_epoch();
    auto microseconds_duration = duration_cast<microseconds>(duration).count();
    return (static_cast<double>(microseconds_duration)) / 1.e6f + start_time;
}

String getCurrentTimestamp()
{
    double time_in_seconds = getCurrentTime();
    return String(time_in_seconds, 6U); // Precision to the microsecond
}

// Convert Unix timestamp to Julian date
double unixTimeToJD(double const &unixTime)
{
    return unixTime / SECONDS_IN_DAY + UNIX_EPOCH_JD;
}

// Compute Greenwich Mean Sidereal Time
double computeGMST(double const &jd)
{
    // Split JD into UT1 components using the Date & Time method
    double uta = floor(jd); // Integer part
    double utb = jd - uta;  // Fractional day part

    // NB : If this slows down, just use TT = UT for simplicity
    double tta, ttb, tai1, tai2;
    iauUtctai(uta, utb, &tai1, &tai2);
    iauTaitt(tai1, tai2, &tta, &ttb);

    // Call SOFA function to compute GMST in radians
    double gmst_rad = iauGmst00(uta, utb, tta, ttb);

    // Convert GMST from radians to hours
    double gmst_hours = gmst_rad * 12.0 / M_PI;

    // Ensure GMST is in the range [0, 24) hours
    if (gmst_hours < 0)
    {
        gmst_hours += 24.0;
    }
    return gmst_hours;
}
/*
{
    double d = jd - J2000;
    double gmst = 18.697375 + 24.065709824279 * d;
    gmst = fmod(gmst, 24.0);
    return (gmst < 0) ? gmst + 24.0 : gmst;
}*/

// Nutation and Equation of Equinoxes
double computeEquationOfEquinoxes(double const &d)
{
    double omega = 125.04 - 0.052954 * d;
    double L = 280.47 + 0.98565 * d;
    double epsilon = 23.4393 - 0.0000004 * d;

    double deltaPsi = -0.000319 * sin(omega * DEG_TO_RAD) - 0.000024 * sin(2 * L * DEG_TO_RAD);
    return deltaPsi * cos(epsilon * DEG_TO_RAD);
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

    double sh, ch, sd, cd, sp, cp, x, y, z, r, a;

    // Starting from here : taken from SOFA hd2ae.c
    /* Useful trig functions. */
    sh = sin(hourAngle);
    ch = cos(hourAngle);
    sd = sin(dec);
    cd = cos(dec);
    sp = sin(lat);
    cp = cos(lat);

    /* Az,Alt unit vector. */
    x = -ch * cd * sp + sd * cp;
    y = -sh * cd;
    z = ch * cd * cp + sd * sp;

    /* To spherical. */
    r = sqrt(x * x + y * y);
    a = (r != 0.0) ? atan2(y, x) : 0.0;
    double az = (a < 0.0) ? a + 2 * M_PI : a;
    double alt = atan2(z, r);

    return std::make_tuple(az * RAD_TO_DEG, alt * RAD_TO_DEG);
}

// Convert Galactic to Equatorial (RA, Dec)
std::tuple<double, double> galacticToEquatorial(double l, double b)
{
    l *= DEG_TO_RAD;
    b *= DEG_TO_RAD;

    double sinDec = sin(b) * sin(GALACTIC_NGP_DEC * DEG_TO_RAD) +
                    cos(b) * cos(GALACTIC_NGP_DEC * DEG_TO_RAD) * cos(l - GALACTIC_NORTH_LON * DEG_TO_RAD);
    double dec = asin(sinDec);

    double y = cos(b) * sin(l - GALACTIC_NORTH_LON * DEG_TO_RAD);
    double x = cos(b) * cos(l - GALACTIC_NORTH_LON * DEG_TO_RAD) * sin(GALACTIC_NGP_DEC * DEG_TO_RAD) -
               sin(b) * cos(GALACTIC_NGP_DEC * DEG_TO_RAD);

    double ra = atan2(y, x) + GALACTIC_NGP_RA * DEG_TO_RAD;

    ra = fmod(ra, 2 * M_PI);
    if (ra < 0)
        ra += 2 * M_PI;

    return std::make_tuple(ra * RAD_TO_DEG, dec * RAD_TO_DEG);
}

std::tuple<double, double> galacticToAltAz(double l, double b, double unixTime)
{
    auto [ra, dec] = galacticToEquatorial(l, b);
    return raDecToAltAz(ra, dec, unixTime);
}