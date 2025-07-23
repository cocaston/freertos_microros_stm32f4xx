#include "app.h"

rclc_support_t support;
rcl_node_t node;
rcl_timer_t timer;
rclc_executor_t executor;
rcl_allocator_t allocator;
rcl_publisher_t publisher;
std_msgs__msg__Int32 msg;

bool micro_ros_init_successful;
enum states state;

void timer_callback(rcl_timer_t *timer, int64_t last_call_time) {
    (void) last_call_time;
    if (timer != NULL)
    {
        rcl_publish(&publisher, &msg, NULL);
        msg.data++;
    }
}

bool create_entities()
{
    allocator = rcl_get_default_allocator();

    RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
    RCCHECK(rclc_node_init_default(&node, "int32_publisher_rclc", "", &support));
    RCCHECK(rclc_publisher_init_best_effort(
        &publisher,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
        "std_msgs_msg_Int32"
    ));

    const unsigned int timer_timeout = 1000;
    RCCHECK(rclc_timer_init_default(
        &timer,
        &support,
        RCL_MS_TO_NS(timer_timeout),
        timer_callback
    ));

    executor = rclc_executor_get_zero_initialized_executor();
    RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));
    RCCHECK(rclc_executor_add_timer(&executor, &timer));
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
    return true;
}

void destroy_entities()
{
    rmw_context_t* rmw_context = rcl_context_get_rmw_context(&support.context);
    (void) rmw_uros_set_context_entity_destroy_session_timeout(rmw_context, 0);

    rcl_publisher_fini(&publisher, &node);
    rcl_timer_fini(&timer);
    rclc_executor_fini(&executor);
    rcl_node_fini(&node);
    rclc_support_fini(&support);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
}

void keep_connect()
{
    while(1)
    {
        switch(state)
        {
            case WAITING_AGENT:
                EXECUTE_EVERY_N_MS(500, state = (RMW_RET_OK == rmw_uros_ping_agent(100, 1)) ? AGENT_AVAILABLE: WAITING_AGENT;);
                break;
            case AGENT_AVAILABLE:
                state = (true == create_entities()) ? AGENT_AVAILABLE : WAITING_AGENT;
                if(state == WAITING_AGENT) {
                    destroy_entities();
                };
                break;
            case AGENT_CONNECTED:
                EXECUTE_EVERY_N_MS(200, state = (RMW_RET_OK == rmw_uros_ping_agent(100, 1)) ? AGENT_CONNECTED : AGENT_DISCONNECTED;);
                if (state == AGENT_CONNECTED) 
                {
                    for(;;)
                    {
                        rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));
                        vTaskDelay(500);
                    }
                }
                break;
            case AGENT_DISCONNECTED:
                destroy_entities();
                state = WAITING_AGENT;
                break;
            default:
                break;
        }
    }
}

// void blink()
// {
//     while(1)
//         HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, (state == AGENT_CONNECTED) ? GPIO_PIN_RESET : GPIO_PIN_SET);
// }

void app_init(){


}

