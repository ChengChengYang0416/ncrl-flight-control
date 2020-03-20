#include <string.h>
#include "arm_math.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "delay.h"
#include "bound.h"
#include "led.h"
#include "uart.h"
#include "pwm.h"
#include "sbus_receiver.h"
#include "mpu6500.h"
#include "ms5611.h"
#include "motor.h"
#include "optitrack.h"
#include "lpf.h"
#include "imu.h"
#include "ahrs.h"
#include "multirotor_pid_ctrl.h"
#include "multirotor_geometry_ctrl.h"
#include "motor_thrust.h"
#include "fc_task.h"
#include "sys_time.h"
#include "proj_config.h"
#include "debug_link.h"
#include "perf.h"
#include "perf_list.h"

#define FLIGHT_CTL_PRESCALER_RELOAD 10

extern optitrack_t optitrack;

SemaphoreHandle_t flight_ctl_semphr;

imu_t imu;
ahrs_t ahrs;
radio_t rc;

void flight_ctl_semaphore_handler(void)
{
	static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(flight_ctl_semphr, &xHigherPriorityTaskWoken);
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

void rc_safety_protection(void)
{
	radio_t rc;

	float time_last = 0.0f;
	float time_current = 0.0f;

	led_off(LED_R);
	led_off(LED_G);
	led_off(LED_B);

	do {
		time_current = get_sys_time_ms();
		if(time_current - time_last > 100.0f) {
			led_toggle(LED_R);
			time_last = time_current;
		}
		read_rc(&rc);
	} while(rc_safety_check(&rc) == 1);
}

void rc_yaw_setpoint_handler(float *desired_yaw, float rc_yaw_cmd, float dt)
{
	/* changing yaw setpoint if yaw joystick exceed the +-5 degree zone */
	if(rc_yaw_cmd > +5.0f || rc_yaw_cmd < -5.0f) {
		*desired_yaw += rc_yaw_cmd * dt;
		/* signal bounding */
		if(*desired_yaw > +180.0f) {
			*desired_yaw -= 360.0f;
		} else if(*desired_yaw < -180.0f) {
			*desired_yaw += 360.0f;
		}
	}
}

void task_flight_ctrl(void *param)
{
	mpu6500_init(&imu);
	ms5611_init();
	motor_init();

	ahrs_init(imu.accel_raw);

	multirotor_pid_controller_init();
	geometry_ctrl_init();

	led_off(LED_R);
	led_off(LED_G);
	led_on(LED_B);

	rc_safety_protection();

	float desired_yaw = 0.0f;

	while(1) {
		perf_start(FLIGHT_CONTROL_TASK);
		while(xSemaphoreTake(flight_ctl_semphr, 9) == pdFALSE);

		//gpio_toggle(MOTOR7_FREQ_TEST);

		read_rc(&rc);
		rc_yaw_setpoint_handler(&desired_yaw, -rc.yaw, 0.0025);

		perf_start(AHRS);
		ahrs_estimate(&ahrs, imu.accel_lpf, imu.gyro_lpf);
		perf_end(AHRS);

		perf_start(FLIGHT_CONTROLLER);
#if (SELECT_CONTROLLER == QUADROTOR_USE_PID)
		multirotor_pid_control(&imu, &ahrs, &rc, &desired_yaw);
#elif (SELECT_CONTROLLER == QUADROTOR_USE_GEOMETRY)
		multirotor_geometry_control(&imu, &ahrs, &rc, &desired_yaw);
#endif
		perf_end(FLIGHT_CONTROLLER);

		perf_end(FLIGHT_CONTROL_TASK);
		taskYIELD();
	}
}

void send_imu_debug_message(debug_msg_t *payload)
{
	pack_debug_debug_message_header(payload, MESSAGE_ID_IMU);
	pack_debug_debug_message_float(&imu.accel_raw[0], payload);
	pack_debug_debug_message_float(&imu.accel_raw[1], payload);
	pack_debug_debug_message_float(&imu.accel_raw[2], payload);
	pack_debug_debug_message_float(&imu.accel_lpf[0], payload);
	pack_debug_debug_message_float(&imu.accel_lpf[1], payload);
	pack_debug_debug_message_float(&imu.accel_lpf[2], payload);
	pack_debug_debug_message_float(&imu.gyro_raw[0], payload);
	pack_debug_debug_message_float(&imu.gyro_raw[1], payload);
	pack_debug_debug_message_float(&imu.gyro_raw[2], payload);
	pack_debug_debug_message_float(&imu.gyro_lpf[0], payload);
	pack_debug_debug_message_float(&imu.gyro_lpf[1], payload);
	pack_debug_debug_message_float(&imu.gyro_lpf[2], payload);
}

void send_attitude_euler_debug_message(debug_msg_t *payload)
{
	pack_debug_debug_message_header(payload, MESSAGE_ID_ATTITUDE_EULER);
	pack_debug_debug_message_float(&ahrs.attitude.roll, payload);
	pack_debug_debug_message_float(&ahrs.attitude.pitch, payload);
	pack_debug_debug_message_float(&ahrs.attitude.yaw, payload);
}

void send_attitude_quaternion_debug_message(debug_msg_t *payload)
{
	pack_debug_debug_message_header(payload, MESSAGE_ID_ATTITUDE_QUAT);
	pack_debug_debug_message_float(&ahrs.q[0], payload);
	pack_debug_debug_message_float(&ahrs.q[1], payload);
	pack_debug_debug_message_float(&ahrs.q[2], payload);
	pack_debug_debug_message_float(&ahrs.q[3], payload);
}

void send_attitude_imu_debug_message(debug_msg_t *payload)
{
	pack_debug_debug_message_header(payload, MESSAGE_ID_ATTITUDE_IMU);
	pack_debug_debug_message_float(&ahrs.attitude.roll, payload);
	pack_debug_debug_message_float(&ahrs.attitude.pitch, payload);
	pack_debug_debug_message_float(&ahrs.attitude.yaw, payload);
	pack_debug_debug_message_float(&imu.accel_lpf[0], payload);
	pack_debug_debug_message_float(&imu.accel_lpf[1], payload);
	pack_debug_debug_message_float(&imu.accel_lpf[2], payload);
	pack_debug_debug_message_float(&imu.gyro_lpf[0], payload);
	pack_debug_debug_message_float(&imu.gyro_lpf[1], payload);
	pack_debug_debug_message_float(&imu.gyro_lpf[2], payload);
}
