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
