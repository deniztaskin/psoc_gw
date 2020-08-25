/*Social distancing also called “physical distancing,” means keeping space between yourself and
other people outside of your home.

To practice social or physical distancing:
-Stay at least 6 feet (about 2 arms’ length) from other people
-Do not gather in groups
-Stay out of crowded places and avoid mass gatherings

In this project, we will develop a gateway device that can count groups of people in a narrow area
and pushes data to the cloud. Thus, we will be able to follow the crowd density information centrally.

*/


#include "wiced.h"
#include "wiced_aws.h"
#include "aws_common.h"
#include "resources.h"
#include <string.h>
#include "wiced_bt_dev.h"
#include "wiced_bt_ble.h"
#include "wiced_bt_gatt.h"
#include "wiced_bt_cfg.h"
#include "wiced.h"
#include "bt_target.h"
#include "wiced_bt_stack.h"
#include "gattdefs.h"
#include "sdpdefs.h"
#include "command_console.h"
#include "wwd_debug.h"
#include "wiced_bt_dev.h"
#include "wiced_low_power.h"
#include "wiced_bt_uuid.h"


/******************************************************
 *                      Macros
 ******************************************************/

#define APPLICATION_DELAY_IN_MILLISECONDS   (1000)

#define APP_AWS_CONNACK_TIMEOUT             (3 * APPLICATION_DELAY_IN_MILLISECONDS)
#define APP_AWS_PUBLISH_ACK_TIMEOUT         (2 * APPLICATION_DELAY_IN_MILLISECONDS)
#define SCANNER_AWS_INITIALIZE_TIMEOUT       (30 * APPLICATION_DELAY_IN_MILLISECONDS)
#define SCANNER_PUBLISH_TIMEOUT              (5 *  APPLICATION_DELAY_IN_MILLISECONDS)
#define PUBLISHER_CERTIFICATES_MAX_SIZE            (0x7fffffff)
#define WICED_TOPIC                                "PSOC_GW"
#define APP_PUBLISH_RETRY_COUNT                    (5)

/******************************************************
 *               Variable Definitions
 ******************************************************/

static wiced_semaphore_t  end_of_scan_semaphore;
static wiced_semaphore_t  event_semaphore;
extern const wiced_bt_cfg_settings_t wiced_bt_cfg_settings;
extern const wiced_bt_cfg_buf_pool_t wiced_bt_cfg_buf_pools[];
static wiced_bool_t             is_connected = WICED_FALSE;
static wiced_aws_qos_level_t    qos = WICED_AWS_QOS_ATMOST_ONCE;
int scan_result; //Number of devices
char msg[33]; //Message to publish

static wiced_aws_thing_security_info_t my_publisher_security_creds =
{
    .private_key         = NULL,
    .key_length          = 0,
    .certificate         = NULL,
    .certificate_length  = 0,
};

static wiced_aws_endpoint_info_t my_publisher_aws_iot_endpoint = {
    .transport           = WICED_AWS_TRANSPORT_MQTT_NATIVE,
    .uri                 = "xxxxxxxxxxxxxxxxxxxx-ats.iot.us-east-2.amazonaws.com",
    .peer_common_name    = NULL,
    .ip_addr             = {0},
    .port                = WICED_AWS_IOT_DEFAULT_MQTT_PORT,
    .root_ca_certificate = NULL,
    .root_ca_length      = 0,
};

static wiced_aws_thing_info_t my_publisher_aws_config = {
    .name            = "PSOC_GW",
    .credentials     = &my_publisher_security_creds,
};

static wiced_aws_handle_t my_app_aws_handle;


/******************************************************
 *               Static Function Definitions
 ******************************************************/

// Every Ble event activates callback function
static wiced_result_t ble_scanner_management_callback( wiced_bt_management_evt_t event, wiced_bt_management_evt_data_t *p_event_data )
{
    wiced_result_t result = WICED_BT_SUCCESS;
    switch( event )
    {
        // Bluetooth stack initialized
        case BTM_ENABLED_EVT:
            WPRINT_APP_INFO( ("Ble INIT OK, Scanning Begins....") );
            break;

        case BTM_DISABLED_EVT:
            break;

        case BTM_BLE_SCAN_STATE_CHANGED_EVT: // Activate the semaphore when the scan is finished.
             if( p_event_data->ble_scan_state_changed == 2) wiced_rtos_set_semaphore( &end_of_scan_semaphore);
             break;
    }

    return result;
}
static wiced_result_t do_connect_and_acknowledge(wiced_aws_handle_t handle)
{
    wiced_result_t ret = WICED_ERROR;
    ret = wiced_aws_connect(handle);
    if (ret != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("[Application/AWS] Connect Failed(returned: %d)\n", ret));
        return ret;
    }

    // If Connect passed - wait for CONN-ACK
    ret = wiced_rtos_get_semaphore(&event_semaphore, APP_AWS_CONNACK_TIMEOUT);
    if (ret != WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("[Application/AWS] Error getting Conn-Acknowledgment(returned: %d)\n", ret));
        return ret;
    }

    if (!is_connected)
    {
        WPRINT_APP_INFO(("[Application/AWS] Invalid Semaphore signalling(Connection flags still not set!!)\n"));
        return WICED_TIMEOUT;
    }

    return WICED_SUCCESS;
}



static wiced_result_t get_aws_credentials_from_resources( void )
{
    uint32_t size_out = 0;
    wiced_result_t result = WICED_ERROR;

    wiced_aws_thing_security_info_t* security = &my_publisher_security_creds;
    uint8_t** root_ca_certificate = &my_publisher_aws_iot_endpoint.root_ca_certificate;

    if( security->certificate && security->private_key && (*root_ca_certificate) )
    {
        WPRINT_APP_INFO(("\n[Application/AWS] Security Credentials already set(not NULL). Abort Reading from Resources...\n"));
        return WICED_SUCCESS;
    }

    // Get AWS Root CA certificate filename: 'rootca.cer' located @ resources/apps/aws/iot folder
    result = resource_get_readonly_buffer( &resources_apps_DIR_aws_DIR_iot_DIR_rootca_cer, 0, PUBLISHER_CERTIFICATES_MAX_SIZE, &size_out, (const void **) root_ca_certificate);
    if( result != WICED_SUCCESS )
    {
        goto _fail_aws_certificate;
    }
    if( size_out < 64 )
    {
        WPRINT_APP_INFO( ( "\n[Application/AWS] Invalid Root CA Certificate! Replace the dummy certificate with AWS one[<YOUR_WICED_SDK>/resources/app/aws/iot/'rootca.cer']\n\n" ) );
        resource_free_readonly_buffer( &resources_apps_DIR_aws_DIR_iot_DIR_rootca_cer, (const void *)*root_ca_certificate );
        goto _fail_aws_certificate;
    }

    my_publisher_aws_iot_endpoint.root_ca_length = size_out;

    // Get Publisher's Certificate filename: 'client.cer' located @ resources/apps/aws/iot/pubisher folder
    result = resource_get_readonly_buffer( &resources_apps_DIR_aws_DIR_iot_DIR_publisher_DIR_client_cer, 0, PUBLISHER_CERTIFICATES_MAX_SIZE, &size_out, (const void **) &security->certificate );
    if( result != WICED_SUCCESS )
    {
        goto _fail_client_certificate;
    }
    if(size_out < 64)
    {
        WPRINT_APP_INFO( ( "\n[Application/AWS] Invalid Device Certificate! Replace the dummy certificate with AWS one[<YOUR_WICED_SDK>/resources/app/aws/iot/publisher/'client.cer']\n\n" ) );
        resource_free_readonly_buffer( &resources_apps_DIR_aws_DIR_iot_DIR_publisher_DIR_client_cer, (const void *)security->certificate );
        goto _fail_client_certificate;
    }

    security->certificate_length = size_out;

    // Get Publisher's Private Key filename: 'privkey.cer' located @ resources/apps/aws/iot/publisher folder
    result = resource_get_readonly_buffer( &resources_apps_DIR_aws_DIR_iot_DIR_publisher_DIR_privkey_cer, 0, PUBLISHER_CERTIFICATES_MAX_SIZE, &size_out, (const void **) &security->private_key );
    if( result != WICED_SUCCESS )
    {
        goto _fail_private_key;
    }
    if(size_out < 64)
    {
        WPRINT_APP_INFO( ( "\n[Application/AWS] Invalid Device Private-Key! Replace the dummy Private-key with AWS one[<YOUR_WICED_SDK>/resources/app/aws/iot/publisher/'privkey.cer'\n\n" ) );
        resource_free_readonly_buffer( &resources_apps_DIR_aws_DIR_iot_DIR_publisher_DIR_privkey_cer, (const void *)security->private_key );
        goto _fail_private_key;
    }
    security->key_length = size_out;

    return WICED_SUCCESS;

_fail_private_key:
    resource_free_readonly_buffer( &resources_apps_DIR_aws_DIR_iot_DIR_publisher_DIR_client_cer, (const void *)security->certificate );
_fail_client_certificate:
    resource_free_readonly_buffer( &resources_apps_DIR_aws_DIR_iot_DIR_rootca_cer, (const void *)*root_ca_certificate );
_fail_aws_certificate:
    return WICED_ERROR;
}

// Call back function to handle AWS events.
static void my_publisher_aws_callback( wiced_aws_handle_t aws, wiced_aws_event_type_t event, wiced_aws_callback_data_t* data )
{
    if( !aws || !data || (aws != my_app_aws_handle) )
    {
        WPRINT_APP_INFO(("[Application/AWS] Bad callback parameters[Invalid handle? %s(value: %ld), Data is NULL? %s]\n",
                     aws != my_app_aws_handle ? "Yes": "No", aws, data ? "No" : "Yes"));
        return;
    }

    switch ( event )
    {
        case WICED_AWS_EVENT_CONNECTED:
        {
            if( data->connection.status == WICED_SUCCESS )
            {
                WPRINT_APP_INFO(("[Application/AWS] Connection Acknowledgment Received\n"));
                is_connected = WICED_TRUE;
                wiced_rtos_set_semaphore(&event_semaphore);
            }
            break;
        }

        case WICED_AWS_EVENT_DISCONNECTED:
        {
            if( data->disconnection.status == WICED_SUCCESS )
            {
                WPRINT_APP_INFO(("[Application/AWS] Disconnection Received\n"));
                is_connected = WICED_FALSE;
            }
            break;
        }

        case WICED_AWS_EVENT_PUBLISHED:
        {
            if( data->publish.status == WICED_SUCCESS )
            {
                WPRINT_APP_INFO(("[Application/AWS] Publish Acknowledgment Received\n"));
                if (qos > WICED_AWS_QOS_ATMOST_ONCE)
                {
                    WPRINT_APP_DEBUG(("[Application/AWS] Signal App waiting for PUBACK\n"));
                    wiced_rtos_set_semaphore(&event_semaphore);
                }
            }
            break;
        }

        case WICED_AWS_EVENT_SUBSCRIBED:
        case WICED_AWS_EVENT_UNSUBSCRIBED:
        case WICED_AWS_EVENT_PAYLOAD_RECEIVED:
        default:
            break;
    }
}

/******************************************************
 *               Function Definitions
 ******************************************************/
// Every Ble scan event activates callback function
void ble_scanner_scan_result_cback( wiced_bt_ble_scan_results_t* p_scan_result, uint8_t* p_adv_data ) {
    if ( p_scan_result != NULL ) scan_result++; // Every result one point

}


void application_start( void )
{
    int pub_retries = 0;
    wiced_aws_handle_t aws_connection = 0;
    wiced_result_t ret = WICED_SUCCESS;


    wiced_core_init();
    wiced_init( );
    wiced_bt_stack_init( ble_scanner_management_callback , &wiced_bt_cfg_settings, wiced_bt_cfg_buf_pools ); // init ble stack


    // Bring up the network interface
    ret = wiced_network_up( WICED_AWS_DEFAULT_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL );
    if ( ret != WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ( "[Application/AWS] Not able to join the requested AP\n\n" ) );
        return;
    }

    ret = get_aws_credentials_from_resources();
    if( ret != WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ("[Application/AWS] Error fetching credentials from resources\n" ) );
        return;
    }

    int max_conn_retries = 0;
    int quit_app = WICED_FALSE;

    wiced_rtos_init_semaphore(&end_of_scan_semaphore);

    while (!quit_app)
    {
        wiced_aws_init(&my_publisher_aws_config , my_publisher_aws_callback);
        if( ret != WICED_SUCCESS )
        {
            WPRINT_APP_INFO( ( "[Application/AWS] Failed to Initialize AWS library\n\n" ) );
            return;
        }

        aws_connection = (wiced_aws_handle_t)wiced_aws_create_endpoint(&my_publisher_aws_iot_endpoint);
        if( !aws_connection )
        {
            WPRINT_APP_INFO( ( "[Application/AWS] Failed to create AWS connection handle\n\n" ) );
            return;
        }

        my_app_aws_handle = aws_connection;

        wiced_rtos_init_semaphore(&event_semaphore);

        WPRINT_APP_INFO(("[Application/AWS] Opening connection...\n"));

        pub_retries = 0;



        while (is_connected || !(max_conn_retries >= WICED_AWS_DEFAULT_CONNECTION_RETRIES))
        {
            if (!is_connected)
            {
                WPRINT_APP_INFO(("[Application/AWS] Try Connecting...\n"));
                ret = do_connect_and_acknowledge(aws_connection);
                if(ret != WICED_SUCCESS)
                {
                    max_conn_retries++;
                    continue;
                }
                else
                {
                    max_conn_retries = 0;
                    WPRINT_APP_INFO(("[Application/AWS] Connection Successful...\n"));
                }
            }


            wiced_bt_ble_scan( BTM_BLE_SCAN_TYPE_HIGH_DUTY, WICED_TRUE, ble_scanner_scan_result_cback );

            // Wait for Ble scan.....
            ret = wiced_rtos_get_semaphore(&end_of_scan_semaphore, SCANNER_PUBLISH_TIMEOUT);

            if (ret != WICED_SUCCESS) //Scan ends here....
            {
                continue;
            }

            sprintf(msg, "GW_ID:AWS01, Active_Scanned=%d",  scan_result);
            scan_result=0;

            WPRINT_APP_INFO(("[Application/AWS] Publishing... %s\n", msg)); // Publish the results to the cloud service
            pub_retries = 0;
            do
            {
                // Try publishing until it returns success or exceeds retry-count
                ret = wiced_aws_publish(aws_connection, WICED_TOPIC, (uint8_t *)msg, strlen( msg ), qos);
                pub_retries++ ;
            } while ( ( ret != WICED_SUCCESS ) && ( pub_retries < APP_PUBLISH_RETRY_COUNT ) );

            // if we did exceed retry-count => above function failed to publish anything. Force a disconnect. Set the flags accordingly
            if (pub_retries >= APP_PUBLISH_RETRY_COUNT)
            {
                WPRINT_APP_INFO(("[Application/AWS] Publishing failed(ret: %d)\n", ret));
                /* if we are still connected; Force a Disconnect */
                if (is_connected)
                {
                    wiced_aws_disconnect(aws_connection);
                }
                is_connected = 0;
                continue;
            }

            // Published - Check if we want an acknowledgment
            if (qos > WICED_AWS_QOS_ATMOST_ONCE)
            {
                ret = wiced_rtos_get_semaphore(&event_semaphore, APP_AWS_PUBLISH_ACK_TIMEOUT);
                if (ret != WICED_SUCCESS)
                {
                    WPRINT_APP_INFO(("[Application/AWS] Error Receiving Publish Ack(ret: %d)\n", ret));
                    /* if we are still connected; Force a Disconnect */
                    if (is_connected)
                    {
                        wiced_aws_disconnect(aws_connection);
                    }
                    is_connected = 0;
                }
            }
        }

        WPRINT_APP_INFO(("[Application/AWS] Closing connection...\r\n"));
        wiced_aws_disconnect(aws_connection);

        wiced_rtos_deinit_semaphore(&event_semaphore);

        WPRINT_APP_INFO(("[Application/AWS] Deinitializing AWS library...\r\n"));
        ret = wiced_aws_deinit();
    }

    wiced_rtos_deinit_semaphore(&end_of_scan_semaphore);
    return;
}









