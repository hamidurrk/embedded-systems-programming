/*
 * config.h
 *
 * Created: 4/11/2026 1:39:35 AM
 *  Author: Hamidur
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_

#define ELEVATOR_START_FLOOR      (0u)
#define ELEVATOR_MAX_FLOOR        (99u)
#define ELEVATOR_QUEUE_CAPACITY   (32u)

#define ELEVATOR_OBSTACLE_KEY     ('D')
#define ELEVATOR_OBSTACLE_FALLBACK_ENABLED  (1u)
#define ELEVATOR_OBSTACLE_THRESHOLD_CM      (10u)
#define ELEVATOR_OBSTACLE_STREAK_REQUIRED   (3u)

#define ELEVATOR_SLEEP_MODE_ENABLED         (1u)
#define ELEVATOR_SLEEP_TRIGGER_KEY          ('C')
#define ELEVATOR_SLEEP_TRIGGER_HOLD_MS      (2000u)

#define APP_TEST_MODE_BOOT_KEY    ('A')
#define APP_TEST_MODE_HOLD_MS     (2000u)
#define APP_LOOP_TICK_MS          (20u)




#endif /* CONFIG_H_ */