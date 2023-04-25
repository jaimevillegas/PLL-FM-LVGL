#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "configLovyan.h"
#include <lvgl.h>
#include "ui.h"
#include <Preferences.h>

#define EEPROM_SIZE 4096

Preferences preferences;

// ----- PIN DEFINITIONS -----
#define potDir_out 25
#define fan_out 15
#define buzzer_out 4
#define mpx_out 32
#define modL_in 14
#define modR_in 26
#define modMPX_in 13
#define temp_in 36
#define potDir_in 39
#define potRef_in 34
// ---------------------------

// * Frequency and roller variables
char valueRoller1_str[20];
int valueRoller1_int;
char valueRoller2_str[20];
int valueRoller2_int;
int valueFreqHz;
float valueFreqMhz;
char valueFreqMhz_str[28];
char valueRollerMPX_str[10];

char str_freq[10];

// * Mapping Variables
char str_map_temp_in[16];
char str_map_dir_in[16];
char str_map_ref_in[16];

// * ---- MAPPING FORMULAS -----
int coefficient = 1240;
int modRValue = 3.3 * coefficient; // First value given in volts
int modLValue = 3.3 * coefficient;
int modMPXValue = 3.3 * coefficient;
int potDirValue = 3.3 * coefficient;
int potRefValue = 3.3 * coefficient;

int map_modL_in;
int map_modR_in;
int map_modMPX_in;
int map_temp_in;
int map_potDir_in;
int map_potRef_in;

// * ---- FLAG DEFINITIONS -----
bool flag1_temp_fan = 0;
bool flag2_temp_fan = 0;

bool flag1_temp_alarm = 0;
bool flag2_temp_alarm = 0;

bool flag1_set_freq = 0;
bool flag2_set_freq = 0;

bool flag1_alarms = 0;
bool flag2_alarms = 0;
bool flag3_alarms = 0;

bool flag_inicio_alarma = 0;

bool breaker_alarma = 0;
bool breaker_alarma_icon = 0;
// * ---------------------------

unsigned long time1 = 0;
unsigned long time2 = 0;

LGFX tft;

/*Change to your screen resolution*/
static const uint32_t screenWidth = 480;
static const uint32_t screenHeight = 320;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

void pll_setup(long frekvenc)
{
  Serial.print("En PLL SETUP");
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
}

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
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

    // Serial.print("Data x ");
    // Serial.println(touchX);

    // Serial.print("Data y ");
    // Serial.println(touchY);
  }
}

// ! --- ALARMS ---
void alarmSystem()
{
  if (flag_inicio_alarma == 1)
  {
    // Serial.println("En alarmSystem ");
    // Serial.print("flag1_alarms: ");
    // Serial.println(flag1_alarms);
    // Serial.print("flag2_alarms: ");
    // Serial.println(flag2_alarms);
    if (flag1_alarms == 1)
    {
      // digitalWrite(buzzer_out, HIGH);
      // flag_inicio_alarma = 0;
      // TODO: poner blinking y un if anidado para preguntar si esta en low y poner inicio_alarma en 0
      time1 = millis();
      if (time1 - time2 > 1000)
      {
        time2 = time1;
        Serial.println("-------------Timer 1s");
        breaker_alarma = !breaker_alarma;
        breaker_alarma_icon = !breaker_alarma_icon;
        digitalWrite(buzzer_out, breaker_alarma);
        if (breaker_alarma_icon == 1)
        {
          lv_obj_fade_in(ui_ImageAlarm, 100, 0);
        }
        if (breaker_alarma_icon == 0)
        {
          lv_obj_fade_out(ui_ImageAlarm, 100, 0);
        }
      }
      // Serial.println(map_temp_in);
    }
    if (flag1_alarms == 0)
    {
      digitalWrite(buzzer_out, LOW);
      Serial.println("Buzzer LOW");
      flag_inicio_alarma = 0;
    }
  }
}
// ! --------------

// * --- MAIN FUNCTION --- *
void main_func(void *pvParameters)
{
  // * Inicializar Potencia
  Serial.println("En main_func");
  int savedPotDir = preferences.getInt("potDir", false);
  for (int i = 0; i <= savedPotDir; i++)
  {
    ledcWrite(0, i);
    delay(100);
  }

  lv_obj_clear_state(ui_LabelFreqValue, LV_STATE_USER_1);

  while (1)
  {
    alarmSystem();
    // * Button Ajustar Frecuencia y Pll_setup
    if (lv_obj_get_state(ui_btnAjustarFreq) == 35)
    {
      lv_roller_get_selected_str(ui_rollerFreq1, valueRoller1_str, 0);
      lv_roller_get_selected_str(ui_rollerFreq2, valueRoller2_str, 0);
      valueRoller1_int = atoi(valueRoller1_str);
      valueRoller2_int = atoi(valueRoller2_str);
      valueFreqHz = valueRoller1_int * 1000000 + valueRoller2_int * 100000;
      preferences.putInt("freq", valueFreqHz);
      valueFreqMhz = valueFreqHz / 1000000.0;
      dtostrf(valueFreqMhz, 3, 1, valueFreqMhz_str);
      lv_label_set_text(ui_LabelFreqValue, valueFreqMhz_str);
      pll_setup(valueFreqHz);
    }

    // * --- MAP ANALOG INPUTS TO SLIDERS ---
    map_modL_in = map(analogRead(modL_in), 0, modLValue, 0, 20);
    map_modR_in = map(analogRead(modR_in), 0, modRValue, 0, 20);
    map_modMPX_in = map(analogRead(modMPX_in), 0, modMPXValue, 0, 20);
    map_temp_in = map(analogRead(temp_in), 0, 4095, 0, 150);
    map_potDir_in = map(analogRead(potDir_in), 0, potDirValue, 0, 120);
    map_potRef_in = map(analogRead(potRef_in), 0, potRefValue, 0, 12);
    lv_slider_set_value(ui_SliderModR, map_modR_in, LV_ANIM_OFF);
    lv_slider_set_value(ui_SliderModL, map_modL_in, LV_ANIM_OFF);
    lv_slider_set_value(ui_SliderModMPX, map_modMPX_in, LV_ANIM_OFF);

    lv_label_set_text(ui_LabelTemperatureValue, itoa(map_temp_in, str_map_temp_in, 10));
    lv_label_set_text(ui_LabelPotDirValue, itoa(map_potDir_in, str_map_dir_in, 10));
    lv_label_set_text(ui_LabelPotRefValue, itoa(map_potRef_in, str_map_ref_in, 10));

    // * ---- TEMPERATURE CONDITIONALS -----
    // TODO: Set temperature formula to attach LM35

    if (map_temp_in >= 50 && flag1_temp_fan == 0)
    {
      lv_obj_clear_state(ui_LabelTemperatureValue, LV_STATE_DEFAULT);
      lv_obj_add_state(ui_LabelTemperatureValue, LV_STATE_USER_2);

      lv_obj_clear_state(ui_ImageTemperature, LV_STATE_DEFAULT);
      lv_obj_add_state(ui_ImageTemperature, LV_STATE_USER_2);
      fan_rotate_Animation(ui_ImageFan, 0);
      digitalWrite(fan_out, HIGH);
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

        lv_obj_clear_state(ui_ImageTemperature, LV_STATE_USER_1);
        lv_obj_clear_state(ui_ImageTemperature, LV_STATE_USER_2);
        lv_obj_add_state(ui_ImageTemperature, LV_STATE_DEFAULT);
        lv_anim_del_all();
        digitalWrite(fan_out, LOW);
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
        lv_obj_add_state(ui_LabelTemperatureValue, LV_STATE_USER_3);

        lv_obj_clear_state(ui_ImageTemperature, LV_STATE_DEFAULT);
        lv_obj_clear_state(ui_ImageTemperature, LV_STATE_USER_2);
        lv_obj_add_state(ui_ImageTemperature, LV_STATE_USER_3);
        Serial.println("ALARMA > 70");
        // alarm_opacity_Animation(ui_ImageAlarm, 0);
        flag1_alarms = 1;
        flag_inicio_alarma = 1;
        flag1_temp_alarm = 1;
        flag2_temp_alarm = 0;
      }
    }
    if (map_temp_in < 70)
    {
      if (flag2_temp_alarm == 0 && flag1_temp_alarm == 1)
      {
        lv_obj_clear_state(ui_LabelTemperatureValue, LV_STATE_USER_1);
        lv_obj_clear_state(ui_LabelTemperatureValue, LV_STATE_USER_3);
        lv_obj_add_state(ui_LabelTemperatureValue, LV_STATE_USER_2);

        lv_obj_clear_state(ui_ImageTemperature, LV_STATE_DEFAULT);
        lv_obj_clear_state(ui_ImageTemperature, LV_STATE_USER_3);
        lv_obj_add_state(ui_ImageTemperature, LV_STATE_USER_2);
        lv_obj_fade_out(ui_ImageAlarm, 100, 0);
        // TODO: add alarm functionality
        Serial.println("ALARMA < 70");
        flag1_alarms = 0;
        flag_inicio_alarma = 1;
        flag2_temp_alarm = 1;
        flag1_temp_alarm = 0;
      }
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Wire.begin();

  preferences.begin("storage", false);
  pll_setup(preferences.getInt("freq", false));

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
  pinMode(mpx_out, OUTPUT);
  pinMode(modL_in, INPUT);
  pinMode(modR_in, INPUT);
  pinMode(modMPX_in, INPUT);
  pinMode(temp_in, INPUT);
  pinMode(potDir_in, INPUT);
  pinMode(potRef_in, INPUT);
  // --------------------------------

  ledcSetup(0, 5000, 8);
  ledcAttachPin(potDir_out, 0);

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

  // * --- Set initial values from preferences ---
  float freqMhz = preferences.getInt("freq", false) / 1000000.0;
  dtostrf(freqMhz, 3, 1, valueFreqMhz_str);
  lv_label_set_text(ui_LabelFreqValue, valueFreqMhz_str);

  if (preferences.getChar("mpx", false) == 83)
  {
    digitalWrite(mpx_out, HIGH);
    lv_roller_set_selected(ui_rollerMPX, 0, LV_ANIM_OFF);
  }

  if (preferences.getChar("mpx", false) == 77)
  {
    digitalWrite(mpx_out, LOW);
    lv_roller_set_selected(ui_rollerMPX, 1, LV_ANIM_OFF);
  }

  // TODO: Set initial value for potDir
  lv_slider_set_value(ui_SliderPotDir, preferences.getInt("potDir", false), LV_ANIM_OFF);
  lv_label_set_text_fmt(ui_LabelPotDirValue, "%d", preferences.getInt("potDir", false));

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

  if (lv_obj_get_state(ui_btnAjustarPotencia) == 35)
  {
    int ui_SliderPotDirValue = lv_slider_get_value(ui_SliderPotDir);
    int map_uiSliderPotDirValue = map(ui_SliderPotDirValue, 0, 300, 0, 255);
    int savedPotDir = preferences.getInt("potDir", false);

    if (savedPotDir > map_uiSliderPotDirValue)
    {
      for (int i = savedPotDir; i >= map_uiSliderPotDirValue; i--)
      {
        ledcWrite(0, i);
        delay(50);
      }
    }
    else
    {

      for (int i = savedPotDir; i <= map_uiSliderPotDirValue; i++)
      {
        ledcWrite(0, i);
        delay(50);
      }
    }
    preferences.putInt("potDir", map_uiSliderPotDirValue);
  }

  lv_roller_get_selected_str(ui_rollerMPX, valueRollerMPX_str, 0);

  if (valueRollerMPX_str[0] == 'S')
  {
    preferences.putChar("mpx", 'S');
    digitalWrite(mpx_out, HIGH);
  }

  if (valueRollerMPX_str[0] == 'M')
  {
    preferences.putChar("mpx", 'M');
    digitalWrite(mpx_out, LOW);
  }

  delay(20);
}
