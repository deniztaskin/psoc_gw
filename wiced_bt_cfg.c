/*
 * Copyright 2017, Cypress Semiconductor Corporation or a subsidiary of Cypress Semiconductor 
 *  Corporation. All rights reserved. This software, including source code, documentation and  related 
 * materials ("Software"), is owned by Cypress Semiconductor  Corporation or one of its 
 *  subsidiaries ("Cypress") and is protected by and subject to worldwide patent protection  
 * (United States and foreign), United States copyright laws and international treaty provisions. 
 * Therefore, you may use this Software only as provided in the license agreement accompanying the 
 * software package from which you obtained this Software ("EULA"). If no EULA applies, Cypress 
 * hereby grants you a personal, nonexclusive, non-transferable license to  copy, modify, and 
 * compile the Software source code solely for use in connection with Cypress's  integrated circuit 
 * products. Any reproduction, modification, translation, compilation,  or representation of this 
 * Software except as specified above is prohibited without the express written permission of 
 * Cypress. Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO  WARRANTY OF ANY KIND, EXPRESS 
 * OR IMPLIED, INCLUDING,  BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS FOR A PARTICULAR PURPOSE. Cypress reserves the right to make changes to 
 * the Software without notice. Cypress does not assume any liability arising out of the application 
 * or use of the Software or any product or circuit  described in the Software. Cypress does 
 * not authorize its products for use in any products where a malfunction or failure of the 
 * Cypress product may reasonably be expected to result  in significant property damage, injury 
 * or death ("High Risk Product"). By including Cypress's product in a High Risk Product, the 
 *  manufacturer of such system or application assumes  all risk of such use and in doing so agrees 
 * to indemnify Cypress against all liability.
 */

/** @file
 *
 * Runtime Bluetooth stack configuration parameters
 *
 */
#include "wiced_bt_dev.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_gatt.h"
#include "wiced_bt_cfg.h"



/*****************************************************************************
 * wiced_bt core stack configuration
 ****************************************************************************/
const wiced_bt_cfg_settings_t wiced_bt_cfg_settings =
{
        .max_number_of_buffer_pools = WICED_BT_CFG_NUM_BUF_POOLS,
        .addr_resolution_db_size = 3,
    .br_edr_scan_cfg =                                                                               // BR/EDR scan settings
    {
        .inquiry_scan_type               = BTM_SCAN_TYPE_STANDARD,                                   // Inquiry scan type (BTM_SCAN_TYPE_STANDARD or BTM_SCAN_TYPE_INTERLACED)
        .inquiry_scan_interval           = WICED_BT_CFG_DEFAULT_INQUIRY_SCAN_INTERVAL,               // Inquiry scan interval  (0 to use default)
        .inquiry_scan_window             = WICED_BT_CFG_DEFAULT_INQUIRY_SCAN_WINDOW,                 // Inquiry scan window (0 to use default)

        .page_scan_type                  = BTM_SCAN_TYPE_STANDARD,                                   // Page scan type (BTM_SCAN_TYPE_STANDARD or BTM_SCAN_TYPE_INTERLACED)
        .page_scan_interval              = WICED_BT_CFG_DEFAULT_PAGE_SCAN_INTERVAL,                  // Page scan interval  (0 to use default)
        .page_scan_window                = WICED_BT_CFG_DEFAULT_PAGE_SCAN_WINDOW                     // Page scan window (0 to use default)
    },

    .ble_scan_cfg =                                                                                  // BLE scan settings
    {
        .scan_mode                       = BTM_BLE_SCAN_MODE_PASSIVE,                                 // BLE scan mode (BTM_BLE_SCAN_MODE_PASSIVE, BTM_BLE_SCAN_MODE_ACTIVE, or BTM_BLE_SCAN_MODE_NONE)

        .high_duty_scan_interval         = WICED_BT_CFG_DEFAULT_HIGH_DUTY_SCAN_INTERVAL,             // High duty scan interval
        .high_duty_scan_window           = WICED_BT_CFG_DEFAULT_HIGH_DUTY_SCAN_WINDOW,               // High duty scan window
        .high_duty_scan_duration         = 5,                                                        // High duty scan duration in seconds (0 for infinite)

        .low_duty_scan_interval          = WICED_BT_CFG_DEFAULT_LOW_DUTY_SCAN_INTERVAL,              // Low duty scan interval
        .low_duty_scan_window            = WICED_BT_CFG_DEFAULT_LOW_DUTY_SCAN_WINDOW,                // Low duty scan window
        .low_duty_scan_duration          = 5,                                                        // Low duty scan duration in seconds (0 for infinite)

        /* Connection scan intervals */
        .high_duty_conn_scan_interval    = WICED_BT_CFG_DEFAULT_HIGH_DUTY_CONN_SCAN_INTERVAL,        // High duty cycle connection scan interval
        .high_duty_conn_scan_window      = WICED_BT_CFG_DEFAULT_HIGH_DUTY_CONN_SCAN_WINDOW,          // High duty cycle connection scan window
        .high_duty_conn_duration         = 30,                                                       // High duty cycle connection duration in seconds (0 for infinite)

        .low_duty_conn_scan_interval     = WICED_BT_CFG_DEFAULT_LOW_DUTY_CONN_SCAN_INTERVAL,         // Low duty cycle connection scan interval
        .low_duty_conn_scan_window       = WICED_BT_CFG_DEFAULT_LOW_DUTY_CONN_SCAN_WINDOW,           // Low duty cycle connection scan window
        .low_duty_conn_duration          = 30,                                                       // Low duty cycle connection duration in seconds (0 for infinite)

        /* Connection configuration */
        .conn_min_interval               = WICED_BT_CFG_DEFAULT_CONN_MIN_INTERVAL,                    // Minimum connection interval, 24 * 1.25 = 30ms.
        .conn_max_interval               = WICED_BT_CFG_DEFAULT_CONN_MAX_INTERVAL,                    // Maximum connection interval, 40 * 1.25 = 50ms.
        .conn_latency                    = WICED_BT_CFG_DEFAULT_CONN_LATENCY,                         // Connection latency, ~1sec
        .conn_supervision_timeout        = WICED_BT_CFG_DEFAULT_CONN_SUPERVISION_TIMEOUT              // Connection link supervsion timeout
    },



    .gatt_cfg =                                                                                      // GATT configuration
    {
        .appearance                     = APPEARANCE_GENERIC_TAG,                                    // GATT appearance (see gatt_appearance_e)
        .client_max_links               = 3,                                                         // Client config: maximum number of servers that local client can connect to
        .server_max_links               = 1,                                                         // Server config: maximum number of remote clients connections allowed by the local
        .max_attr_len                   = 512                                                        // Maximum attribute length; gki_cfg must have a corresponding buffer pool that can hold this length
    },

    .rfcomm_cfg =                                                                                    // RFCOMM configuration
    {
        .max_links                      = 0,                                                         // Maximum number of simultaneous connected remote devices*/
        .max_ports                      = 0                                                          // Maximum number of simultaneous RFCOMM ports
    },

    .l2cap_application =                                                                             // Application managed l2cap protocol configuration
    {
        .max_links                      = 0,                                                         // Maximum number of application-managed l2cap links (BR/EDR and LE)

        /* BR EDR l2cap configuration */
        .max_psm                        = 0,                                                         // Maximum number of application-managed BR/EDR PSMs
        .max_channels                   = 0,                                                         // Maximum number of application-managed BR/EDR channels

        /* LE L2cap connection-oriented channels configuration */
        .max_le_psm                     = 0,                                                         // Maximum number of application-managed LE PSMs
        .max_le_channels                = 0,                                                         // Maximum number of application-managed LE channels
    },

    .avdt_cfg =                                                                                      // Audio/Video Distribution configuration
    {
        .max_links                      = 0,                                                         // Maximum simultaneous audio/video links
    },

    . avrc_cfg =                                                                                     // Audio/Video Remote Control configuration
    {
        .roles                          = 0,                                                         // Mask of local roles supported (AVRC_CONN_INITIATOR|AVRC_CONN_ACCEPTOR)
        .max_links                      = 0                                                          // Maximum simultaneous remote control links
    },
    .addr_resolution_db_size            = 5//,                                                         // LE Address Resolution DB settings - effective only for pre 4.2 controller
    //.max_mtu_size                       = 517,                                                        // Maximum MTU size for GATT connections, should be between 23 and (max_attr_len + 5 )
    //.max_pwr_db_val                     = 12
};

/*****************************************************************************
 * wiced_bt_stack buffer pool configuration
 *
 * Configure buffer pools used by the stack
 *
 * Pools must be ordered in increasing buf_size.
 * If a pool runs out of buffers, the next  pool will be used
 *****************************************************************************/



const wiced_bt_cfg_buf_pool_t wiced_bt_cfg_buf_pools[WICED_BT_CFG_NUM_BUF_POOLS] =

{

/*  { buf_size, buf_count } */

    { 64,       10   },      /* can be used when attribute length is less than or equal to 64 and buffers are available */

    { 158,      12  },      /* can be used by when attribute length is less than or equal to 158 bytes */

    { 360,      12  },      /* will only be used when previous buffer-pools are not available */

    { 660,      10  },      /* will only be used when previous buffer-pools are not available */

};

