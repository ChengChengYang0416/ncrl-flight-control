#include "optitrack.h"
#include "position_sensor.h"
#include "altitude_est.h"
#include "proj_config.h"
#include "ms5611.h"

bool is_xy_position_info_available(void)
{
#if (SELECT_POSITION_SENSOR == POSITION_SENSOR_USE_OPTITRACK)
	return optitrack_available();
#elif (SELECT_POSITION_SENSOR == POSITION_SENSOR_USE_GPS)
	//XXX
#else
	return false;
#endif
}

bool is_height_info_available(void)
{
#if (SELECT_HEIGHT_SENSOR == HEIGHT_SENSOR_USE_OPTITRACK)
	return optitrack_available();
#elif (SELECT_HEIGHT_SENSOR == HEIGHT_SENSOR_USE_BAROMETER)
	return ms5611_available();
#else
	return false;
#endif
}

void get_enu_position(float *pos)
{
	/* x-y position */
#if (SELECT_POSITION_SENSOR == POSITION_SENSOR_USE_OPTITRACK)
	optitrack_read_pos_x(&pos[0]);
	optitrack_read_pos_y(&pos[1]);
#elif (SELECT_POSITION_SENSOR == POSITION_SENSOR_USE_GPS)
	//XXX
#else
	pos[0] = 0.0f;
	pos[1] = 0.0f;
#endif

	/* z position */
#if (SELECT_HEIGHT_SENSOR == HEIGHT_SENSOR_USE_OPTITRACK)
	optitrack_read_pos_z(&pos[2]);
#elif (SELECT_HEIGHT_SENSOR == HEIGHT_SENSOR_USE_BAROMETER)
	pos[2] = ms5611_get_relative_altitude();
#else
	pos[2] = 0.0f;
#endif
}

float get_enu_height(void)
{
#if (SELECT_HEIGHT_SENSOR == HEIGHT_SENSOR_USE_OPTITRACK)
	float height;
	optitrack_read_pos_z(&height);
	return height;
#elif (SELECT_HEIGHT_SENSOR == HEIGHT_SENSOR_USE_BAROMETER)
	return ms5611_get_relative_altitude();
#else
	return 0.0f;
#endif
}

void get_wgs84_position(float *latitude, float *longtitude, float *height)
{
}

void get_enu_velocity(float *vel)
{
	/* x-y velocity */
#if (SELECT_POSITION_SENSOR == POSITION_SENSOR_USE_OPTITRACK)
	optitrack_read_vel_x(&vel[0]);
	optitrack_read_vel_y(&vel[1]);
#elif (SELECT_POSITION_SENSOR == POSITION_SENSOR_USE_GPS)
	//XXX
#else
	vel[0] = 0.0f;
	vel[1] = 0.0f;
#endif

	/* z velocity */
#if (SELECT_HEIGHT_SENSOR == HEIGHT_SENSOR_USE_OPTITRACK)
	optitrack_read_vel_z(&vel[2]);
#elif (SELECT_HEIGHT_SENSOR == HEIGHT_SENSOR_USE_BAROMETER)
	vel[2] = get_fused_barometer_relative_altitude_rate();
#else
	vel[2] = 0.0f;
#endif
}