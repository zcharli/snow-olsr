
#define INTERFACE_NAME "wlan0"
// Defines the max num threads for waiting on messages
#define MAX_SEM 25
#define MIN_SEM 0
// Defined for max MTU of interface
#define MAX_BUF 1500

/**
 * Below is OLSR dependent constants
 */

/**
 * SCALING FACTOR FOR VALIDITY TIME (18.1
 */
#define C_SCALE_FACTOR 1000.0f / 16.0f

/**
 * TIME INTERVALS 18.2, 18.3
 */
#define T_SECOND 1000 // 1 second
#define T_HELLO_INTERVAL 2 * T_SECOND
#define T_REFRESH_INTERVAL 2 * T_SECOND
#define T_TC_INTERVAL 5 * T_SECOND
#define T_MID_INTERVAL T_TC_INTERVAL
#define T_HNA_INTERVAL T_TC_INTERVAL
#define T_NEIGHB_HOLD_TIME 3 * T_REFRESH_INTERVAL
#define T_TOP_HOLD_TIME 3 * T_TC_INTERVAL
#define T_DUP_HOLD_TIME 30 * T_SECOND
#define T_MID_HOLD_TIME 3 * T_MID_INTERVAL
#define T_HNA_HOLD_TIME 3 * T_HNA_INTERVAL

/**
 * MESSAGE TYPES (18.4)
 */
#define M_HELLO_MESSAGE 1
#define M_TC_MESSAGE 2
#define M_MID_MESSAGE 3
#define M_HNA_MESSAGE 4

/**
 * LINK TYPES (18.5)
 */
#define L_UNSPEC_LINK 0
#define L_ASYM_LINK 1
#define L_SYM_LINK 2
#define L_LOST_LINK 3

/**
 * NEIGHBOR TYPES (18.6)
 */
#define N_NOT_NEIGH 0
#define N_SYM_NEIGH 1
#define N_MPR_NEIGH 2

/**
 * LINK HYSTERESIS (18.7)
 */
#define H_HYST_THRESHOLD_HIGH 0.8f
#define H_HYST_THRESHOLD_LOW 0.3f
#define H_HYST_SCALING 0.5f

/**
 * WILLINGNESS (18.8)
 */
#define W_WILL_NEVER 0
#define W_WILL_LOW 1
#define W_WILL_DEFAULT 3
#define W_WILL_HIGH 6
#define W_WILL_ALWAYS 7

/**
 * MISC CONSTs (18.9)
 */
#define S_TC_REDUNDANCY 0
#define S_MPR_COVERAGE 1
#define S_MAXJITTER T_HELLO_INTERVAL / 4.0f
#define S_MAX_SEQ_NUM   65535
