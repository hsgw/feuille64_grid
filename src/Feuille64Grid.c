/*
             LUFA Library
     Copyright (C) Dean Camera, 2021.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2021  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#include "Feuille64Grid.h"
#include "qmk/gpio.h"
#include "qmk/matrix.h"
#include "leds.h"
#include "grid_defs.h"

#define CDC_SEND_DATA(data, length) CDC_Device_SendData(&VirtualSerial_CDC_Interface, data, length)
#define CDC_SEND_BYTE(data) CDC_Device_SendByte(&VirtualSerial_CDC_Interface, data)
#define CDC_RECIEVE_BYTE() CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface)

/** LUFA CDC Class driver interface configuration and state information. This
 * structure is passed to all CDC Class driver functions, so that multiple
 * instances of the same class within a device can be differentiated from one
 * another.
 */
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface = {
    .Config =
        {
            .ControlInterfaceNumber = INTERFACE_ID_CDC_CCI,
            .DataINEndpoint =
                {
                    .Address = CDC_TX_EPADDR,
                    .Size    = CDC_TXRX_EPSIZE,
                    .Banks   = 1,
                },
            .DataOUTEndpoint =
                {
                    .Address = CDC_RX_EPADDR,
                    .Size    = CDC_TXRX_EPSIZE,
                    .Banks   = 1,
                },
            .NotificationEndpoint =
                {
                    .Address = CDC_NOTIFICATION_EPADDR,
                    .Size    = CDC_NOTIFICATION_EPSIZE,
                    .Banks   = 1,
                },
        },
};

const uint8_t grid_version[8] = GRID_VERSION;

uint8_t grid_id[GRID_DEVICE_ID_LENGTH] = {'f', 'e', 'u', 'i', 'l', 'l', 'e'};
uint8_t grid_number                    = 1;
uint8_t grid_offset_x                  = 0;
uint8_t grid_offset_y                  = 0;

void matrix_button_changed(uint8_t row, uint8_t col, bool is_pressed) {
    if (row != 0) {
        uint8_t data[3] = {is_pressed ? GRID_DEVICE_KEY_PRESSED : GRID_DEVICE_KEY_RELEASED, col, row - 1};
        CDC_SEND_DATA(data, 3);
        CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
    }
}

void serial_process(void) {
    while (true) {
        int16_t cmd_byte = CDC_RECIEVE_BYTE();
        if (cmd_byte < 0) return;

        switch ((uint8_t)cmd_byte) {
            case GRID_CMD_QUERY: {
                // send [cmd, device id, grid num]
                // device id : [null, "led-grid", "key-grid", "digital-out", "digital-in", "encoder", "analog-in", "analog-out", "tilt", "led-ring"]
                uint8_t data[3] = {GRID_REPORT_QUERY, 1, GRID_COL / GRID_ROW};
                CDC_SEND_DATA(data, 3);
            } break;
            case GRID_CMD_ID: {
                // send [cmd, device id(32byte)]
                uint8_t data[GRID_DEVICE_ID_LENGTH + 1];
                data[0] = GRID_CMD_ID;
                memcpy(data + 1, grid_id, GRID_DEVICE_ID_LENGTH);
                CDC_SEND_BYTE(0x02);
                CDC_SEND_DATA(grid_id, GRID_DEVICE_ID_LENGTH);
            } break;
            case GRID_CMD_SET_ID: {
                // recieve 32byte id
                for (uint8_t i = 0; i < GRID_DEVICE_ID_LENGTH; i++) {
                    grid_id[i] = CDC_RECIEVE_BYTE();
                }
                CDC_SEND_BYTE(0xBB);
            } break;
            case GRID_CMD_OFFSET: {
                // send [cmd, grid number ,x offset, y offset]
                uint8_t data[4] = {GRID_REPORT_OFFSET, grid_number, grid_offset_x, grid_offset_y};
                CDC_SEND_DATA(data, 4);
            } break;
            case GRID_CMD_SET_OFFSET: {
                // recieve [number, x offset, y offset]
                grid_number   = CDC_RECIEVE_BYTE();
                grid_offset_x = CDC_RECIEVE_BYTE();
                grid_offset_y = CDC_RECIEVE_BYTE();
            } break;
            case GRID_CMD_SIZE: {
                // send [cmd, x, y]
                uint8_t data[3] = {GRID_REPORT_SIZE, GRID_COL, GRID_ROW};
                CDC_SEND_DATA(data, 3);
            } break;
            case GRID_CMD_SET_SIZE: {
                // ignored
                // recieve [col, row]
                CDC_RECIEVE_BYTE();
                CDC_RECIEVE_BYTE();
            } break;
            case GRID_CMD_ADDR: {
                // ignored
            } break;
            case GRID_CMD_SET_ADDR: {
                // ignored
                // [target addr, new addr]
                CDC_RECIEVE_BYTE();
                CDC_RECIEVE_BYTE();
            } break;
            case GRID_CMD_VERSION: {
                // send [cmd, version[8]]
                uint8_t data[9];
                data[0] = GRID_REPORT_VERSION;
                memcpy(data + 1, grid_version, 8);
                CDC_SEND_DATA(data, 9);
            } break;
            case GRID_CMD_LED_ON: {
                uint8_t col = CDC_RECIEVE_BYTE();
                uint8_t row = CDC_RECIEVE_BYTE();
                leds_set(row, col, 0);
            } break;
            case GRID_CMD_LED_OFF: {
                uint8_t col = CDC_RECIEVE_BYTE();
                uint8_t row = CDC_RECIEVE_BYTE();
                leds_set(row, col, 15);
            } break;
            case GRID_CMD_LED_ALL_ON: {
                leds_set_all(15);
            } break;
            case GRID_CMD_LED_ALL_OFF: {
                leds_set_all(0);
            } break;
            case GRID_CMD_LED_MAP: {
                uint8_t x             = CDC_RECIEVE_BYTE();
                uint8_t y             = CDC_RECIEVE_BYTE();
                bool    ignore_update = false;
                if (x != 0 && y != 0) ignore_update = true;
                for (int8_t row = 0; row < 8; row++) {
                    // int16_t data = -1;
                    // while(true){
                    //     data = CDC_RECIEVE_BYTE();
                    //     if(data<0) {
                    //         CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
                    //         continue;
                    //     }
                    //     break;
                    // }
                    // for (int8_t col = 0; col < 8; col++) {
                    //     if(!ignore_update) leds_set(row, col, (((uint8_t)data) & _BV(col)) == 0 ? 0 : 15);
                    // }

                    uint8_t data = CDC_RECIEVE_BYTE();
                    for (int8_t col = 0; col < 8; col++) {
                        if (!ignore_update) leds_set(row, col, ((data & _BV(col)) == 0 ? 0 : 15));
                    }
                }
            } break;
            case GRID_CMD_LED_ROW: {
                uint8_t x             = CDC_RECIEVE_BYTE();
                uint8_t y             = CDC_RECIEVE_BYTE();
                bool    ignore_update = false;

                if (x != 0 || y > 7) ignore_update = true;
                uint8_t data = CDC_RECIEVE_BYTE();
                for (int8_t col = 0; col < 8; col++) {
                    if (!ignore_update) leds_set(y, col, (data & _BV(col)) == 0 ? 0 : 15);
                }
            } break;
            case GRID_CMD_LED_COL: {
                uint8_t x             = CDC_RECIEVE_BYTE();
                uint8_t y             = CDC_RECIEVE_BYTE();
                bool    ignore_update = false;

                if (x > 7 || y != 0) ignore_update = true;
                uint8_t data = CDC_RECIEVE_BYTE();
                for (int8_t row = 0; row < 8; row++) {
                    if (!ignore_update) leds_set(row, x, (data & _BV(row)) == 0 ? 0 : 15);
                }
            } break;
            case GRID_CMD_LED_INTENSITY: {
                uint8_t row        = CDC_RECIEVE_BYTE();
                uint8_t col        = CDC_RECIEVE_BYTE();
                uint8_t brightness = CDC_RECIEVE_BYTE();
                leds_set(row, col, brightness);
            } break;
            case GRID_CMD_LED_INTENSITY_ALL: {
                uint8_t brightness = CDC_RECIEVE_BYTE();
                leds_set_all(brightness);
            } break;
            case GRID_CMD_LED_INTENSITY_MAP: {
                uint8_t x             = CDC_RECIEVE_BYTE();
                uint8_t y             = CDC_RECIEVE_BYTE();
                bool    ignore_update = false;

                if (x != 0 && y != 0) ignore_update = true;

                for (int8_t row = 0; row < 8; row++) {
                    for (int8_t col = 0; col < 4; col++) {
                        // uint8_t data = CDC_RECIEVE_BYTE();
                        // if(!ignore_update) {
                        //     leds_set(row, col*2, data & 0xFF);
                        //     leds_set(row, col*2+1, data >> 4);
                        // }
                        // temp[row*4+col+1] = data;
                        int16_t data = -1;
                        while (true) {
                            data = CDC_RECIEVE_BYTE();
                            if (data < 0) continue;
                            break;
                        }
                        if (!ignore_update) {
                            leds_set(row, col * 2, ((uint8_t)data) >> 4);
                            leds_set(row, col * 2 + 1, ((uint8_t)data) & 0xF);
                        }
                    }
                }
            } break;
            case GRID_CMD_LED_INTENSITY_ROW: {
                uint8_t x             = CDC_RECIEVE_BYTE();
                uint8_t y             = CDC_RECIEVE_BYTE();
                bool    ignore_update = false;

                if (x > 7 || y != 0) ignore_update = true;
                for (int8_t col = 0; col < 4; col++) {
                    uint8_t data = CDC_RECIEVE_BYTE();
                    if (!ignore_update) {
                        leds_set(y, col * 2, ((uint8_t)data) >> 4);
                        leds_set(y, col * 2 + 1, ((uint8_t)data) & 0xF);
                    }
                }
            } break;
            case GRID_CMD_LED_INTENSITY_COL: {
                uint8_t x             = CDC_RECIEVE_BYTE();
                uint8_t y             = CDC_RECIEVE_BYTE();
                bool    ignore_update = false;

                if (x != 0 || y > 7) ignore_update = true;
                for (int8_t row = 0; row < 4; row++) {
                    uint8_t data = CDC_RECIEVE_BYTE();
                    if (!ignore_update) {
                        leds_set(row, x * 2, ((uint8_t)data) >> 4);
                        leds_set(row, x * 2 + 1, ((uint8_t)data) & 0xF);
                    }
                }
            } break;
            default:
                break;
        }
    }
}

/** Main program entry point. This routine contains the overall program flow,
 * including initial setup of all components and the main program loop.
 */
int main(void) {
    SetupHardware();
    GlobalInterruptEnable();

    setPinOutput(D1);

    for (;;) {
        // scan matrix and send data to host
        matrix_update();
        serial_process();

        CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
        // handled by interrupt
        // USB_USBTask();

        writePin(D1, true);
        leds_update();
        writePin(D1, false);
    }
}

/** Configures the board hardware and chip peripherals */
void SetupHardware(void) {
    /* Disable watchdog if enabled by bootloader/fuses */
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    /* Disable JTAG */
    MCUCR |= _BV(JTD);
    MCUCR |= _BV(JTD);

    /* Disable clock division */
    clock_prescale_set(clock_div_1);

    /* Hardware Initialization */
    leds_init();
    matrix_init();
    USB_Init();
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void) {}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void) {}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void) {
    bool ConfigSuccess = true;

    ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void) { CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface); }

/** CDC class driver callback function the processing of changes to the virtual
 *  control lines sent from the host..
 *
 *  \param[in] CDCInterfaceInfo  Pointer to the CDC class interface
 * configuration structure being referenced
 */
void EVENT_CDC_Device_ControLineStateChanged(USB_ClassInfo_CDC_Device_t *const CDCInterfaceInfo) {
    /* You can get changes to the virtual CDC lines in this callback; a common
       use-case is to use the Data Terminal Ready (DTR) flag to enable and
       disable CDC communications in your application when set to avoid the
       application blocking while waiting for a host to become ready and read
       in the pending data from the USB endpoints.
    */
    bool HostReady = (CDCInterfaceInfo->State.ControlLineStates.HostToDevice & CDC_CONTROL_LINE_OUT_DTR) != 0;

    (void)HostReady;
}
