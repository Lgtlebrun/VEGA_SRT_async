from astropy.time import Time
from astropy.coordinates import SkyCoord, EarthLocation, AltAz, ICRS
from astropy import units as u
OBS_LAT = 46.5194444
OBS_LON = 6.565
OBS_HEIGHT = 411.0
LOC = (OBS_LAT, OBS_LON, OBS_HEIGHT)

def RaDec2AzAlt(ra, dec):
    """
    Utilitary method that transforms the input Right Ascension (RA) and Declination (Dec) coordinates to
    Azimuth (Az) and elevation (Alt) coordinates. AzAlt is computed taking into account the geolocation of VEGA,
    stored in global variables OBS_LAT, OBS_LON and OBS_HEIGHT.

    :param ra: Input RA coordinate in decimal hours
    :type ra: float
    :param dec: Input Dec coordinate in decimal degrees
    :type dec: float
    :return: Converted AzAlt coordinates
    :rtype: (float,float)
    """

    obs_loc = EarthLocation(
        # lat=46.52457*u.deg, lon=6.61650*u.deg, height=500*u.m)
        lat=OBS_LAT*u.deg, lon=OBS_LON * u.deg, height=OBS_HEIGHT*u.m)
    time_now = Time.now()  # + 2*u.hour Don't need to add the time difference
    coords = SkyCoord(ra*u.deg, dec*u.deg)
    altaz = coords.transform_to(AltAz(obstime=time_now, location=obs_loc))

    az, alt = [float(x) for x in altaz.to_string(
        'decimal', precision=4).split(' ')]

    return az, alt