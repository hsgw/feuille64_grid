#pragma once

// reference: https://monome.org/docs/serialosc/serial.txt

// device to host
#define GRID_REPORT_BUTTON_RELEASED 0x20
#define GRID_REPORT_BUTTON_PRESSED 0x21
#define GRID_REPORT_QUERY 0x00
#define GRID_REPORT_ID 0x01
#define GRID_REPORT_OFFSET 0x02
#define GRID_REPORT_SIZE 0x03
#define GRID_REPORT_VERSION 0x0F

// host to device
#define GRID_CMD_QUERY 0x00
#define GRID_CMD_ID 0x01
#define GRID_CMD_SET_ID 0x02
#define GRID_CMD_OFFSET 0x03
#define GRID_CMD_SET_OFFSET 0x04
#define GRID_CMD_SIZE 0x05
#define GRID_CMD_SET_SIZE 0x06
#define GRID_CMD_ADDR 0x07
#define GRID_CMD_SET_ADDR 0x08
#define GRID_CMD_VERSION 0x0F

#define GRID_CMD_LED_ON 0x10
#define GRID_CMD_LED_OFF 0x11
#define GRID_CMD_LED_ALL_OFF 0x12
#define GRID_CMD_LED_ALL_ON 0x13
#define GRID_CMD_LED_MAP 0x14
#define GRID_CMD_LED_ROW 0x15
#define GRID_CMD_LED_COL 0x16
#define GRID_CMD_LED_INTENSITY_OVERALL 0x17
#define GRID_CMD_LED_INTENSITY 0x18
#define GRID_CMD_LED_INTENSITY_ALL 0x19
#define GRID_CMD_LED_INTENSITY_MAP 0x1A
#define GRID_CMD_LED_INTENSITY_ROW 0x1B
#define GRID_CMD_LED_INTENSITY_COL 0x1C

// device to device
#define GRID_DEVICE_KEY_RELEASED 0x20
#define GRID_DEVICE_KEY_PRESSED 0x21

#define GRID_DEVICE_ID_LENGTH 32
