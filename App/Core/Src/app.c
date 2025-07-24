#include "app.h"

MicroROSContext* micro_ros_context;
std_msgs__msg__Int32 msg;

bool micro_ros_init_successful;
enum states state;

void timer_callback(rcl_timer_t *timer, int64_t last_call_time) {
    (void) last_call_time;
    if (timer != NULL)
    {
        rcl_publish(&micro_ros_context->publisher, &msg, NULL);
        msg.data++;
    }
}

bool create_entities()
{
    if(micro_ros_context == NULL)
    {
        micro_ros_context = (MicroROSContext*)malloc(sizeof(MicroROSContext));
    }
    else {
        destroy_entities();
    }
    
    micro_ros_context->allocator = rcl_get_default_allocator();

    // create init_options
    RCCHECK(rclc_support_init(&micro_ros_context->support, 0, NULL, &micro_ros_context->allocator));

    // create node
    RCCHECK(rclc_node_init_default(&micro_ros_context->node, "int32_publisher_rclc", "", &micro_ros_context->support));

    // create publisher
    RCCHECK(rclc_publisher_init_best_effort(
        &micro_ros_context->publisher,
        &micro_ros_context->node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
        "std_msgs_msg_Int32"));

    // create timer,
    const unsigned int timer_timeout = 1000;
    RCCHECK(rclc_timer_init_default(
        &micro_ros_context->timer,
        &micro_ros_context->support,
        RCL_MS_TO_NS(timer_timeout),
        timer_callback));

    // create executor
    micro_ros_context->executor = rclc_executor_get_zero_initialized_executor();
    RCCHECK(rclc_executor_init(
        &micro_ros_context->executor,
        &micro_ros_context->support.context,
        1,
        &micro_ros_context->allocator));

    RCCHECK(rclc_executor_add_timer(
        &micro_ros_context->executor,
        &micro_ros_context->timer));

    return true;
}

void destroy_entities()
{
    // for (int i =0; i < 10; i++) {
    //     HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
    //     vTaskDelay(100);
    //     HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
    //     vTaskDelay(100);
    // }

    if (micro_ros_context != NULL)
    {
        rmw_context_t* rmw_context = rcl_context_get_rmw_context(&micro_ros_context->support.context);
        (void) rmw_uros_set_context_entity_destroy_session_timeout(rmw_context, 0);

        RCCHECK(rcl_publisher_fini(&micro_ros_context->publisher, &micro_ros_context->node));
        RCCHECK(rcl_timer_fini(&micro_ros_context->timer));
        RCCHECK(rclc_executor_fini(&micro_ros_context->executor));
        RCCHECK(rcl_node_fini(&micro_ros_context->node));
        RCCHECK(rclc_support_fini(&micro_ros_context->support));
        
        free(micro_ros_context);
        micro_ros_context = NULL;  // 避免野指针
    }
}

void keep_connect()
{
    while(1)
    {
        // if (rmw_uros_ping_agent(100, 1)) {
        //     blink();
        // }
        switch(state)
        {
            case WAITING_AGENT:
                EXECUTE_EVERY_N_MS(500, state = (RMW_RET_OK == rmw_uros_ping_agent(100, 1)) ? AGENT_AVAILABLE: WAITING_AGENT;);
                break;
            case AGENT_AVAILABLE:
                state = (true == create_entities()) ? AGENT_CONNECTED : WAITING_AGENT;
                if (state == WAITING_AGENT) {
                    destroy_entities();
                }
                break;
            case AGENT_CONNECTED:
                if (RMW_RET_OK == rmw_uros_ping_agent(500, 10)) {
                    state = AGENT_CONNECTED;
                    rclc_executor_spin_some(&micro_ros_context->executor, RCL_MS_TO_NS(100));
                    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
                }
                else {
                    state = AGENT_DISCONNECTED;
                    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
                }
                break;
            case AGENT_DISCONNECTED:
                destroy_entities();
                state = WAITING_AGENT;
                break;
            default:
                break;
        }
        // blink();
        vTaskDelay(500);
    }
}

void app_init(){
    micro_ros_context = (MicroROSContext*)malloc(sizeof(MicroROSContext));
    state = WAITING_AGENT;
    msg.data = 0;
}

