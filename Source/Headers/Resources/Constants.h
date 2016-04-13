

#define INTERFACE_NAME "wlx1cbdb97eb663"
// Defines the max num threads for waiting on messages
#define MAX_SEM 25
#define MIN_SEM 0
// Defined for max MTU of interface
#define MAX_BUF 1500
#define WLAN_HEADER_LEN 14
#define WLAN_ADDR_LEN 6
#define HELLO_MSG_HEADER 12

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
#define T_SECOND 1 // 1 second
#define T_HELLO_INTERVAL 2000 // uisng milliseconds
#define T_REFRESH_INTERVAL 2
#define T_TC_INTERVAL 5
#define T_MID_INTERVAL 5
#define T_HNA_INTERVAL 5
#define T_NEIGHB_HOLD_TIME 6
#define T_TOP_HOLD_TIME 15
#define T_DUP_HOLD_TIME 30
#define T_MID_HOLD_TIME 15
#define T_HNA_HOLD_TIME 15

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
