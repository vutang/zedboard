/*
TSL2591 Datasheet
https://cdn-shop.adafruit.com/datasheets/TSL25911_Datasheet_EN_v1.pdf*/

/*TSL2591 Register map*/
enum {
  TSL2591_REGISTER_ENABLE             = 0x00, // Enable register
  TSL2591_REGISTER_CONTROL            = 0x01, // Control register
  TSL2591_REGISTER_THRESHOLD_AILTL    = 0x04, // ALS low threshold lower byte
  TSL2591_REGISTER_THRESHOLD_AILTH    = 0x05, // ALS low threshold upper byte
  TSL2591_REGISTER_THRESHOLD_AIHTL    = 0x06, // ALS high threshold lower byte
  TSL2591_REGISTER_THRESHOLD_AIHTH    = 0x07, // ALS high threshold upper byte
  TSL2591_REGISTER_THRESHOLD_NPAILTL  = 0x08, // No Persist ALS low threshold lower byte
  TSL2591_REGISTER_THRESHOLD_NPAILTH  = 0x09, // No Persist ALS low threshold higher byte
  TSL2591_REGISTER_THRESHOLD_NPAIHTL  = 0x0A, // No Persist ALS high threshold lower byte
  TSL2591_REGISTER_THRESHOLD_NPAIHTH  = 0x0B, // No Persist ALS high threshold higher byte
  TSL2591_REGISTER_PERSIST_FILTER     = 0x0C, // Interrupt persistence filter
  TSL2591_REGISTER_PACKAGE_PID        = 0x11, // Package Identification
  TSL2591_REGISTER_DEVICE_ID          = 0x12, // Device Identification
  TSL2591_REGISTER_DEVICE_STATUS      = 0x13, // Internal Status
  TSL2591_REGISTER_CHAN0_LOW          = 0x14, // Channel 0 data, low byte
  TSL2591_REGISTER_CHAN0_HIGH         = 0x15, // Channel 0 data, high byte
  TSL2591_REGISTER_CHAN1_LOW          = 0x16, // Channel 1 data, low byte
  TSL2591_REGISTER_CHAN1_HIGH         = 0x17, // Channel 1 data, high byte
};

/*
	Command & Normal Operation
	1010 0000: bits 7 and 5 for 'command normal'
*/
#define TSL2591_COMMAND_BIT       (0xA0) 

/*Flag for ENABLE register to enable*/
#define TSL2591_ENABLE_POWERON    (0x01)
/*ALS Enable. This field activates ALS function. 
Writing a one activates the ALS. Writing a zero disables the ALS*/
#define TSL2591_ENABLE_AEN        (0x02)
/*ALS Interrupt Enable. When asserted permits ALS interrupts 
to be generated, subject to the persist filter.*/
#define TSL2591_ENABLE_AIEN       (0x10)
/*No Persist Interrupt Enable. When asserted NP Threshold conditions 
will generate an interrupt, bypassing the persist filter*/
#define TSL2591_ENABLE_NPIEN      (0x80)    

int tsl2591_dev_open();
int tsl2591_dev_close();

int tsl2591_enable();