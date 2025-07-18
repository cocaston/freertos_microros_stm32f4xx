#include "app.h"

void timer_callback(rcl_timer_t *timer, int64_t last_call_time) {
    (void) last_call_time;
    if (timer != NULL)
    {
        rcl_publish(&publisher, &msg, NULL);
        msg.data++;
    }
}

bool create_entities(void*)
{
    allocator = rcl_get_default_allocator();

    RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
    RCCHECK(rcl_node_init_default(&node, "int32_publisher_rclc", "", &support));
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

    return true;
}

void destroy_entities(void*)
{
    rmw_context_t* rmw_context = rcl_context_get_rmw_context(&support.context);
    (void) rmw_uros_set_context_entity_destroy_session_timeout(rmw_context, 0);

    rcl_publisher_fini(&publisher, &node);
    rcl_timer_fini(&timer);
    rcl_executor_fini(&executor);
    rcl_node_fini(&node);
    rcl_support_fini(&support);
}

void keep_connect(void*)
{
    while(1)
    {
        switch(state)
        {
            case WAITING_AGENT:
                EXECUTE_EVERY_N_MS(500, state = (RMW_RET_OK == rmw_uros_ping_agent(100, 1)) ? AGENT_AVAILABLE: WAITING_AGENT;);
                break;
            case AGENT_AVAILABLE:
                state = (true == create_entities(NULL)) ? AGENT_AVAILABLE : WAITING_AGENT;
                if(state == WAITING_AGENT) {
                    destroy_entities(NULL);
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
                destroy_etities(NULL);
                state = WAITING_AGENT;
                break;
            default:
                break;
        }
    }
}


void app_init(){
    set_microros_transports();

    state = WAITING_AGENT;
    msg.data = 0;
}

void blink(void *)
{
    while(1)
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, (state == AGENT_CONNECTED) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}