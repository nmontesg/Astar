/*** This file includes the definition of various heuristic functions. For detailed explanations, go to: http://movable-type.co.uk/scripts/latlong.html ***/

/*** Haversine distance ***/
double haversine (node u, node v) {
    double diff_lat = (u.lat - v.lat) * pi / 180.f;
    double diff_lon = (u.lon - v.lon) * pi / 180.f;
    double a = pow(sin(diff_lat/2), 2) + cos(u.lat * pi / 180.f) * cos(v.lat * pi / 180.f) * pow(sin(diff_lon/2), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    double d = R * c;
    return d;
}

/*** Spherical law of cosines ***/
double spherical_law_cosines (node u, node v) {
    double term1 = sin(u.lat * pi / 180.f) * sin(v.lat * pi / 180.f);
    double diff_lon = (u.lon - v.lon) * pi / 180.f;
    double term2 = sin(u.lat * pi / 180.f) * sin(v.lat * pi / 180.f) * cos(diff_lon);
    double d = acos(term1 + term2) * R;
    return d;
}

/*** Equirectangular approximation ***/
double equir_approx (node u, node v) {
    double diff_lat = (u.lat - v.lat) * pi / 180.f;
    double diff_lon = (u.lon - v.lon) * pi / 180.f;
    double avg_lat = (u.lat + v.lat) * pi / 360.f;
    double x = diff_lon * cos(avg_lat);
    double y = diff_lat;
    double d = R * sqrt(pow(x, 2) + pow(y, 2));
    return d;
}

/*** distance() returns the heuristic function of choice based on an input integer ***/
double distance (node u, node v, unsigned short choice) {
    if (choice == 1)      return haversine (u, v);
    else if (choice == 2) return spherical_law_cosines (u, v);
    else if (choice == 3) return equir_approx(u, v);
    ExitError("invalid choice of distance function", 5);
    return 0.f;
}
