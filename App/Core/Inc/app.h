#ifndef APP_H
#define APP_H
#include "usart.h"
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <uxr/client/transport.h>
#include <uxr/client/config.h>
#include <std_msgs/msg/int32.h>
#include <rmw_microros/rmw_microros.h>
#include "main.h"


#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){return false;}}
#define EXECUTE_EVERY_N_MS(MS, X)  do { \
  static volatile int64_t init = -1; \
  if (init == -1) { init = uxr_millis();} \
  if (uxr_millis() - init > MS) { X; init = uxr_millis();} \
} while (0)\

typedef struct{
  rclc_support_t support;
  rcl_node_t node;
  rcl_timer_t timer;
  rclc_executor_t executor;
  rcl_allocator_t allocator;
  rcl_publisher_t publisher;
} MicroROSContext;

enum states{
    WAITING_AGENT,
    AGENT_AVAILABLE,
    AGENT_CONNECTED,
    AGENT_DISCONNECTED
};

extern MicroROSContext* micro_ros_context;
extern std_msgs__msg__Int32 msg;
extern bool micro_ros_init_successful;
extern enum states state;

void app_init();
void timer_callback(rcl_timer_t *timer, int64_t last_call_time);
bool create_entities();
void destroy_entities();
void keep_connect();
void blink();
#endif