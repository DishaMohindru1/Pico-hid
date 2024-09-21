#include "pico/stdlib.h"
#include "tusb.h"
#include "pico_hid.h"
#include "hardware/adc.h"

// Enums for DPAD, joystick directions, and button sources
// typedef enum {
//   DPAD_UP_ID = 0,
//   DPAD_RIGHT_ID,
//   DPAD_DOWN_ID,
//   DPAD_LEFT_ID
// } dpad_index;

typedef enum
{
  ADC_LEFT_JOY_X = 0,
  ADC_LEFT_JOY_Y,
} joy_direction;

typedef enum
{
  SRC_BUTTON,
  SRC_ADC
} button_source_kind;

// Structs for button, DPAD, joystick, and input sources
typedef struct
{
  uint32_t action;
  uint8_t gpio_pin;
} button_source;

typedef struct
{
  uint16_t value; // Holds the ADC value for the joystick axis
} joystick_adc_map_t;

static joystick_adc_map_t joy_map[2]; // Define joy_map for 2 axes

typedef struct
{
  uint8_t gpio_pin[4]; // Up, Right, Down, Left
} dpad_source;

typedef struct
{
  joy_direction direction;
  uint8_t adc_channel;
} adc_source;

typedef union
{
  adc_source adc_src;
  button_source button_src;
  // dpad_source dpad_src;
} input_source;

typedef struct
{
  button_source_kind source;
  input_source data;
} button_data;

// Button configuration
const button_data _button_config[] = {
    {SRC_BUTTON, {.button_src = {GAMEPAD_BUTTON_SOUTH, 7}}},
    {SRC_BUTTON, {.button_src = {GAMEPAD_BUTTON_EAST, 8}}},
    {SRC_BUTTON, {.button_src = {GAMEPAD_BUTTON_NORTH, 5}}},
    {SRC_BUTTON, {.button_src = {GAMEPAD_BUTTON_WEST, 6}}},
    {SRC_BUTTON, {.button_src = {GAMEPAD_BUTTON_MODE, 9}}},
    {SRC_BUTTON, {.button_src = {GAMEPAD_BUTTON_SELECT, 20}}},
    {SRC_BUTTON, {.button_src = {GAMEPAD_BUTTON_START, 21}}}};

const int _button_config_count = 6;

// Setup GPIO for buttons
void setup_controller_buttons(void)
{
  for (int i = 0; i < _button_config_count; i++)
  {
    // if (_button_config[i].source == SRC_DPAD) {
    //   for (int j = 0; j < 4; j++) {
    //     gpio_init(_button_config[i].data.dpad_src.gpio_pin[j]);
    //     gpio_set_dir(_button_config[i].data.dpad_src.gpio_pin[j], GPIO_IN);
    //     gpio_pull_up(_button_config[i].data.dpad_src.gpio_pin[j]);
    //   }
    // } else if
    // (_button_config[i].source == SRC_BUTTON)
    
      gpio_init(_button_config[i].data.button_src.gpio_pin);
      gpio_set_dir(_button_config[i].data.button_src.gpio_pin, GPIO_IN);
      gpio_pull_up(_button_config[i].data.button_src.gpio_pin);
    
  }
  // Initialize ADC for joystick
  adc_init();
  adc_gpio_init(26); // Joystick X-axis
  adc_gpio_init(27); // Joystick Y-axis
}

// Check if HID report is empty
bool is_empty(const hid_gamepad_report_t *report)
{
  return 0 == report->buttons + report->hat + report->x + report->y + report->z + report->rx + report->ry + report->rz;
}

// // Update DPAD values in HID report
// void update_dpad(hid_gamepad_report_t *report, const dpad_source *data) {
//   uint8_t result = GAMEPAD_HAT_CENTERED;

//   if (!gpio_get(data->gpio_pin[DPAD_UP_ID])) {
//     if (!gpio_get(data->gpio_pin[DPAD_LEFT_ID])) result = GAMEPAD_HAT_UP_LEFT;
//     else if (!gpio_get(data->gpio_pin[DPAD_RIGHT_ID])) result = GAMEPAD_HAT_UP_RIGHT;
//     else result = GAMEPAD_HAT_UP;
//   } else if (!gpio_get(data->gpio_pin[DPAD_DOWN_ID])) {
//     if (!gpio_get(data->gpio_pin[DPAD_LEFT_ID])) result = GAMEPAD_HAT_DOWN_LEFT;
//     else if (!gpio_get(data->gpio_pin[DPAD_RIGHT_ID])) result = GAMEPAD_HAT_DOWN_RIGHT;
//     else result = GAMEPAD_HAT_DOWN;
//   } else if (!gpio_get(data->gpio_pin[DPAD_LEFT_ID])) result = GAMEPAD_HAT_LEFT;
//   else if (!gpio_get(data->gpio_pin[DPAD_RIGHT_ID])) result = GAMEPAD_HAT_RIGHT;

//   report->hat = result;
// }

// Update button values in HID report
void update_button(hid_gamepad_report_t *report, const button_source *data)
{
  if (!gpio_get(data->gpio_pin))
  { // pulled down
    report->buttons |= data->action;
  }
}

// Update the HID report for the controller
void update_hid_report_controller(hid_gamepad_report_t *report)
{
  for (int i = 0; i < _button_config_count; i++)
  {
    update_button(report, &_button_config[i].data.button_src);
  }


// Read joystick ADC values
adc_select_input(0);           // X-axis (GPIO26)
joy_map[0].value = adc_read(); // X axis

adc_select_input(1);           // Y-axis (GPIO27)
joy_map[1].value = adc_read(); // Y axis

// Scale 12-bit ADC values to 8-bit range
report->x = joy_map[0].value / 16;
report->y = joy_map[1].value / 16;
}