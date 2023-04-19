#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "configLovyan.h"
#include <lvgl.h>
#include "ui.h"

// ----- PIN DEFINITIONS -----
#define potDir_out 25
#define fan_out 15
#define buzzer_out 4
#define modL_in 14
#define modR_in 26
#define modMPX_in 13
#define temp_in 36
#define potDir_in 39
#define potRef_in 34
// ---------------------------

// * ---- FLAG DEFINITIONS -----
bool flag1_temp_fan = 0;
bool flag2_temp_fan = 0;

bool flag1_temp_alarm = 0;
bool flag2_temp_alarm = 0;
// * ---------------------------

LGFX tft;

/*Change to your screen resolution*/
static const uint32_t screenWidth = 480;
static const uint32_t screenHeight = 320;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

void pll_setup(long frekvenc)
{
  char polje[5];
  long Quartz = 3200000;                  // 3.2Mhz crystal
  long fReferenca = Quartz / 512;         // Divider of Quartz fq/512 = 7.8125kHz (4MHz)  or 6.250kHz(3.2MHz)
  long fDivider = (frekvenc + 50000) / 8; // RF frequency input divider 15bits
  long div = fDivider / fReferenca;       // phase comparator calc

  polje[0] = 0xc1;                // the address of TSA5511 0xC2 8bytes wire library!-->  7bytes=0x61
  polje[1] = (div & 0xFF00) >> 8; // upper
  polje[2] = div & 0x00FF;        // lower
  polje[3] = 0x8E;                // 10001110, charge pumper
  polje[4] = 0x00;                // set pinouts etc

  Wire.beginTransmission(0x61);
  Wire.write(&polje[1]);
  Wire.write(&polje[2]);
  Wire.write(&polje[3]);
  Wire.write(&polje[4]);
  Wire.endTransmission();
  Serial.print("senal enviada al TSA, frec: ");
  Serial.println(frekvenc);
}

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  // tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
  tft.writePixels((lgfx::rgb565_t *)&color_p->full, w * h);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  uint16_t touchX, touchY;
  bool touched = tft.getTouch(&touchX, &touchY);
  if (!touched)
  {
    data->state = LV_INDEV_STATE_REL;
  }
  else
  {
    data->state = LV_INDEV_STATE_PR;

    /*Set the coordinates*/
    data->point.x = touchX;
    data->point.y = touchY;

    Serial.print("Data x ");
    Serial.println(touchX);

    Serial.print("Data y ");
    Serial.println(touchY);
  }
}

// * --- MAIN FUNCTION --- *
void main_func(void *pvParameters)
{
  // ---- CREATE STYLES ----

  // -----------------------
  while (1)
  {
    lv_color_t red1 = lv_color_hex(0xff0c08);

    // * --- MAP ANALOG INPUTS TO SLIDERS ---
    // TODO -> Declare map umbrals as variables
    int map_modL_in = map(analogRead(modL_in), 0, 4095, 0, 20);
    int map_modR_in = map(analogRead(modR_in), 0, 4095, 0, 20);
    int map_modMPX_in = map(analogRead(modMPX_in), 0, 4095, 0, 20);
    int map_temp_in = map(analogRead(temp_in), 0, 4095, 0, 150);
    int map_potDir_in = map(analogRead(potDir_in), 0, 4095, 0, 20);
    int map_potRef_in = map(analogRead(potRef_in), 0, 4095, 0, 20);
    char str_map_temp_in[16];
    lv_slider_set_value(ui_SliderModR, map_modR_in, LV_ANIM_OFF);
    lv_slider_set_value(ui_SliderModL, map_modL_in, LV_ANIM_OFF);
    lv_slider_set_value(ui_SliderModMPX, map_modMPX_in, LV_ANIM_OFF);

    // TODO: Find documentation about itoa function
    lv_label_set_text(ui_LabelTemperatureValue, itoa(map_temp_in, str_map_temp_in, 10));

    // * ---- TEMPERATURE CONDITIONALS -----

    if (map_temp_in >= 50 && flag1_temp_fan == 0)
    {
      lv_obj_clear_state(ui_LabelTemperatureValue, LV_STATE_DEFAULT);
      lv_obj_add_state(ui_LabelTemperatureValue, LV_STATE_USER_2);
      fan_rotate_Animation(ui_ImageFan, 0);
      flag1_temp_fan = 1;
      flag2_temp_fan = 0;
    }

    if (map_temp_in < 50)
    {
      if (flag2_temp_fan == 0 && flag1_temp_fan == 1)
      {
        lv_obj_clear_state(ui_LabelTemperatureValue, LV_STATE_USER_1);
        lv_obj_clear_state(ui_LabelTemperatureValue, LV_STATE_USER_2);
        lv_obj_add_state(ui_LabelTemperatureValue, LV_STATE_DEFAULT);
        lv_anim_del_all();
        lv_obj_fade_out(ui_ImageAlarm, 100, 0);
        flag1_temp_fan = 0;
        flag2_temp_fan = 1;
      }
    }

    if (map_temp_in >= 70)
    {
      if (flag1_temp_alarm == 0)
      {
        lv_obj_clear_state(ui_LabelTemperatureValue, LV_STATE_DEFAULT);
        lv_obj_clear_state(ui_LabelTemperatureValue, LV_STATE_USER_2);
        lv_obj_add_state(ui_LabelTemperatureValue, LV_STATE_USER_1);
        lv_obj_fade_in(ui_ImageAlarm, 100, 0);
        alarm_opacity_Animation(ui_ImageAlarm, 0);
        flag1_temp_alarm = 1;
        flag2_temp_alarm = 0;
      }
    }
    if (map_temp_in < 70)
    {
      if (flag2_temp_alarm == 0 && flag1_temp_alarm == 1)
      {
        lv_obj_clear_state(ui_LabelTemperatureValue, LV_STATE_USER_1);
        lv_obj_add_state(ui_LabelTemperatureValue, LV_STATE_USER_2);
        // TODO: add alarm functionality
        flag2_temp_alarm = 1;
        flag1_temp_alarm = 0;
      }
    }
  }
}

void setup()
{
  Serial.begin(115200);
  pll_setup(98100000);

  tft.begin();
  tft.setRotation(1);
  tft.setBrightness(255);
  uint16_t calData[8] = {239, 3926, 233, 265, 3856, 3896, 3714, 308};
  tft.setTouchCalibrate(calData);

  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);

  // ----- pinMode DECLARATIONS -----
  pinMode(potDir_out, OUTPUT);
  pinMode(fan_out, OUTPUT);
  pinMode(buzzer_out, OUTPUT);
  pinMode(modL_in, INPUT);
  pinMode(modR_in, INPUT);
  pinMode(modMPX_in, INPUT);
  pinMode(temp_in, INPUT);
  pinMode(potDir_in, INPUT);
  pinMode(potRef_in, INPUT);
  // --------------------------------

  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the (dummy) input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  ui_init();

  // * --- Dissapear alarm image ---
  lv_obj_fade_out(ui_ImageAlarm, 100, 0);

  xTaskCreatePinnedToCore(main_func,
                          "main_func",
                          4000,
                          NULL,
                          0,
                          NULL,
                          1);
}

void loop()
{
  lv_timer_handler(); /* let the GUI do its work */
  delay(5);
}
