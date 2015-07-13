// BEGIN - YOU CAN MODIFY THIS SECTION //
// There are 3 option in manual configuration:
// 1. SHOW_V ON -> Switch Arduino to Voltometer mode to read voltage values for particular gear
// 2. MANUAL_DEF -> Use standard algorythm to compare gears voltage with manually defined gears table
// 3. MANUAL_RANGES -> Use manual comparison based on manually defined voltage range table
// You can use only one of definition at the same time!

//===================================
// Show voltage flag
//#define SHOW_V ON

//===================================
// Manual Gears definition flag
//#define MANUAL_DEF ON

// Gears definitions
#define GEAR_1 1.39
#define GEAR_2 1.83
#define GEAR_3 2.52
#define GEAR_4 3.26
#define GEAR_5 4.12
#define GEAR_6 4.57

//===================================
// Manual Gears range definition flag
//#define MANUAL_RANGES ON

// Gears ranges definitions
#define G1_UP    1.56
#define G2_UP    2.14
#define G3_UP    2.78
#define G4_UP    3.55
#define G5_UP    4.19
#define G6_UP    4.63

//Feature list:
//#define BT_SERIAL_MODULE ON //Bleutooth Serial Link
//#define TEMP_HUM_MODULE ON //Temerature and Humidity module
//#define AQ_VOLTAGE_MODULE ON //Voltage sensor module for AQ

// END - YOU CAN MODIFY THIS SECTION //
