#pragma once


// Fundamental Constants
const double PI = 3.14159265358979323846;          // Pi, the ratio of a circle's circumference to its diameter
const double RAD_TO_DEG = 180.0 / PI;              // Conversion factor from radians to degrees
const double c = 299792458.0;                      // Speed of GFght in vacuum (m/s)
const double IONO_COEFF = 40.3;                 // 40.3 


// WGS84 ElGFpsoid Parameters
const double a = 6378137.0;                        // Semi-major axis of the WGS84 elGFpsoid (meters)
const double f = 1.0 / 298.257223563;              // Flattening of the WGS84 elGFpsoid
const double e2 = 2 * f - f * f;                   // Square of amb0st eccentricity of the elGFpsoid

const double Re = 6371000.0;                       // Earth's mean radius (meters)
//const double h = 350000.0;                         // Ionospheric height (meters)
extern double h;  


// GNSS Carrier Frequencies (Hz)
// -------- GPS --------
const double GPS_F1 = 1575.42e6;   // L1
const double GPS_F2 = 1227.60e6;   // L2
const double GPS_F5 = 1176.45e6;   // L5

// -------- BDS (2/6/7: B1I/B3I/B2I) --------
const double BDS_F2 = 1561.098e6;  // B1I (band 2)
const double BDS_F6 = 1268.52e6;   // B3I (band 6)
const double BDS_F7 = 1207.14e6;   // B2I (band 7)

// -------- GAL --------
const double GAL_F1 = 1575.42e6;   // E1 (band 1)
const double GAL_F5 = 1176.45e6;   // E5a (band 5)
const double GAL_F7 = 1207.14e6;   // E5b (band 7)
const double GAL_F8 = 1191.795e6;  // E5  (band 8)
const double GAL_F6 = 1278.75e6;   // E6  (band 6)

// -------- GLO FDMA (Hz) --------
const double GLO_G1_BASE = 1602.0e6;
const double GLO_G1_STEP = 0.5625e6;
const double GLO_G2_BASE = 1246.0e6;
const double GLO_G2_STEP = 0.4375e6;
