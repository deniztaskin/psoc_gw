
NAME := apps_demo_psoc_gw

$(NAME)_SOURCES := psoc_gw.c \
                      wiced_bt_cfg.c
                      
$(NAME)_RESOURCES  += apps/aws/iot/rootca.cer \
                      apps/aws/iot/publisher/client.cer \
                      apps/aws/iot/publisher/privkey.cer

$(NAME)_COMPONENTS := protocols/AWS                           
$(NAME)_COMPONENTS += utilities/command_console         
$(NAME)_COMPONENTS += libraries/drivers/bluetooth/low_energy   


WIFI_CONFIG_DCT_H := wifi_config_dct.h

                 


 
GLOBAL_DEFINES += WICED_CONFIG_DISABLE_SSL_SERVER \
                  WICED_CONFIG_DISABLE_DTLS \
                  WICED_CONFIG_DISABLE_ENTERPRISE_SECURITY \
                  WICED_CONFIG_DISABLE_DES \
                  WICED_CONFIG_DISABLE_ADVANCED_SECURITY_CURVES

VALID_OSNS_COMBOS  := ThreadX-NetX_Duo
VALID_PLATFORMS := CY8CKIT_062 \
                   NEB1DX_01 \
                   CYW943907AEVAL1F \
                   CYW9MCU7X9N364 \
                   CYW943455EVB_02 \
                   psoc_gw*

ifeq ($(PLATFORM),$(filter $(PLATFORM), CYW9MCU7X9N364))
GLOBAL_DEFINES += PLATFORM_HEAP_SIZE=40*1024
USE_LIBC_PRINTF     := 0
endif


C_FLAGS += -DWICED_BT_TRACE_ENABLE
#C_FLAGS += -DENABLE_HCI_TRACE
