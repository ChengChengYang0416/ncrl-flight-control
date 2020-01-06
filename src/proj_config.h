#ifndef __PROJ_CONFIG_H__
#define __PROJ_CONFIG_H__

/* uav info */
#define UAV_ID 0

/* uav type */
#define UAV_TYPE_QUADROTOR 0
#define SELECT_UAV_TYPE UAV_TYPE_QUADROTOR

/* ahrs */
#define AHRS_COMPLEMENTARY_FILTER 0
#define AHRS_EKF 1
#define AHRS_MADGWICK_FILTER 2
#define SELECT_AHRS AHRS_COMPLEMENTARY_FILTER

/* quadrotor parameters */
#define QUADROTOR_USE_PID 0
#define QUADROTOR_USE_GEOMETRY 1
#define SELECT_CONTROLLER QUADROTOR_USE_PID

/* localization sensor */
#define LOCALIZATION_USE_GPS 0
#define LOCALIZATION_USE_OPTITRACK 1
#define SELECT_LOCALIZATION LOCALIZATION_USE_OPTITRACK

#endif