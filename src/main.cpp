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
#define led_out 12
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
int modRValue = 0.5 * coefficient; // First value given in volts
int modLValue = 0.5 * coefficient;
int modMPXValue = 0.5 * coefficient;
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

bool flag1_potRef = 0;
bool flag2_potRef = 0;

bool flag_inicio_alarma = 0;

bool breaker_alarma = 0;
bool breaker_alarma_icon = 0;
bool flag_potencia_cero = 0;

// * ---- TIME COUNTERS -----
unsigned long time1 = 0;
unsigned long time2 = 0;
unsigned long time3 = 0;
unsigned long time4 = 0;

int potDir_InitialValue = 50;

// * ---- SCREEN DEFINITIONS -----
LGFX tft;

static const uint32_t screenWidth = 480;
static const uint32_t screenHeight = 320;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

// * Function to setup PLL
void pll_setup(long frekvenc)
{
  char polje[5];
  long Quartz = 3200000;            // 3.2Mhz crystal
  long fReferenca = Quartz / 512;   // Divider of Quartz fq/512 = 7.8125kHz (4MHz)  or 6.250kHz(3.2MHz)
  long fDivider = (frekvenc) / 8;   // RF frequency input divider 15bits
  long div = fDivider / fReferenca; // phase comparator calc

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

    ledcWrite(1, 255);
    time4 = millis();

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
    if (flag1_alarms == 1)
    {
      time1 = millis();
      ledcWrite(0, 50);
      if (time1 - time2 > 500)
      {
        time2 = time1;
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
    }
    if (flag1_alarms == 0)
    {
      digitalWrite(buzzer_out, LOW);
      flag_inicio_alarma = 0;
    }
  }
}

// * --- MAIN FUNCTION --- *
void main_func(void *pvParameters)
{
  // * Inicializar Potencia
  int savedPotDir = preferences.getInt("potDir", false);
  int potDirOutMap = map(savedPotDir, 0, 100, 60, 255);
  for (int i = potDir_InitialValue; i <= potDirOutMap; i++)
  {
    lv_label_set_text(ui_LabelPotDirValue, "0");
    lv_label_set_text(ui_LabelPotRefValue, "0");
    lv_label_set_text(ui_LabelTemperatureValue, "0");
    ledcWrite(0, i);
    delay(100);
  }

  // TODO: Enviar etiquetas de potencia a cero antes de iniciar potencia

  // * Buzzer después de inicializar potencia
  lv_obj_clear_state(ui_LabelFreqValue, LV_STATE_USER_1);
  digitalWrite(buzzer_out, 1);
  delay(200);
  digitalWrite(buzzer_out, 0);
  delay(200);
  digitalWrite(buzzer_out, 1);
  delay(200);
  digitalWrite(buzzer_out, 0);
  delay(200);
  digitalWrite(buzzer_out, 1);
  delay(200);
  digitalWrite(buzzer_out, 0);
  delay(200);

  while (1)
  {
    alarmSystem();
    // * Button Ajustar Frecuencia y Pll_setup
    if (lv_obj_get_state(ui_btnAjustarFreq) == 34)
    {
      // Cuando se presiona el button se manda potencia a cero
      ledcWrite(0, 0);
    }

    if (lv_obj_get_state(ui_btnAjustarFreq) == 35)
    {
      // Obtener valores de los roller, setear frecuencia y guardar en memoria
      lv_roller_get_selected_str(ui_rollerFreq1, valueRoller1_str, 0);
      lv_roller_get_selected_str(ui_rollerFreq2, valueRoller2_str, 0);
      valueRoller1_int = atoi(valueRoller1_str);
      valueRoller2_int = atoi(valueRoller2_str);
      valueFreqHz = valueRoller1_int * 1000000 + valueRoller2_int * 100000;
      preferences.putInt("freq", valueFreqHz);
      valueFreqMhz = valueFreqHz / 1000000.0;
      dtostrf(valueFreqMhz, 3, 1, valueFreqMhz_str);
      lv_label_set_text(ui_LabelFreqValue, valueFreqMhz_str);
      preferences.putInt("roller1", lv_roller_get_selected(ui_rollerFreq1));
      preferences.putInt("roller2", lv_roller_get_selected(ui_rollerFreq2));
      pll_setup(valueFreqHz);

      // Inicializar potencia de nuevo después de setear frecuencia
      potDirOutMap = map(savedPotDir, 0, 100, 60, 255);
      for (int i = potDir_InitialValue; i <= potDirOutMap; i++)
      {
        ledcWrite(0, i);
        delay(100);
      }
    }

    // * --- MAP ANALOG INPUTS TO SLIDERS ---
    map_modL_in = map(analogRead(modL_in), 0, modLValue, 0, 20);
    map_modR_in = map(analogRead(modR_in), 0, modRValue, 0, 20);
    map_modMPX_in = map(analogRead(modMPX_in), 0, modMPXValue, 0, 20);
    map_temp_in = map(analogRead(temp_in), 0, 4095, 0, 75);
    map_potDir_in = map(analogRead(potDir_in), 0, potDirValue, 0, 300);
    map_potRef_in = map(analogRead(potRef_in), 0, potRefValue, 0, 300);

    // * --- SET SLIDERS TO ANALOG INPUTS ---
    lv_slider_set_value(ui_SliderModR, map_modR_in, LV_ANIM_OFF);
    lv_slider_set_value(ui_SliderModL, map_modL_in, LV_ANIM_OFF);
    lv_slider_set_value(ui_SliderModMPX, map_modMPX_in, LV_ANIM_OFF);

    // * --- SET LABELS TO ANALOG INPUTS ---
    lv_label_set_text(ui_LabelTemperatureValue, itoa(map_temp_in, str_map_temp_in, 10));
    lv_label_set_text(ui_LabelPotDirValue, itoa(map_potDir_in, str_map_dir_in, 10));
    lv_label_set_text(ui_LabelPotRefValue, itoa(map_potRef_in, str_map_ref_in, 10));

    // * ---- TEMPERATURE CONDITIONALS -----
    if (map_temp_in >= 50 && flag1_temp_fan == 0)
    {
      // Cambia color del icono de temperatura y enciende ventilador
      lv_obj_clear_state(ui_LabelTemperatureValue, LV_STATE_DEFAULT);
      lv_obj_add_state(ui_LabelTemperatureValue, LV_STATE_USER_2);

      lv_obj_clear_state(ui_ImageTemperature, LV_STATE_DEFAULT);
      lv_obj_add_state(ui_ImageTemperature, LV_STATE_USER_2);
      fan_rotate_Animation(ui_ImageFan, 0);
      digitalWrite(fan_out, HIGH);
      flag1_temp_fan = 1;
      flag2_temp_fan = 0;
    }

    if (map_temp_in < 47)
    {
      if (millis() - time3 > 60000)
      {
        // Deja encendido el ventilador un minuto adicional
        digitalWrite(fan_out, LOW);
      }
      if (flag2_temp_fan == 0 && flag1_temp_fan == 1)
      {
        // restaura colores por defecto, apaga ventilador y alarmas
        lv_obj_clear_state(ui_LabelTemperatureValue, LV_STATE_USER_1);
        lv_obj_clear_state(ui_LabelTemperatureValue, LV_STATE_USER_2);
        lv_obj_add_state(ui_LabelTemperatureValue, LV_STATE_DEFAULT);

        lv_obj_clear_state(ui_ImageTemperature, LV_STATE_USER_1);
        lv_obj_clear_state(ui_ImageTemperature, LV_STATE_USER_2);
        lv_obj_add_state(ui_ImageTemperature, LV_STATE_DEFAULT);
        lv_anim_del_all();
        time3 = millis();

        if (flag_potencia_cero == 1)
        {
          // * Inicializar Potencia
          int savedPotDir = preferences.getInt("potDir", false);
          lv_obj_clear_state(ui_LabelFreqValue, LV_STATE_DEFAULT);
          lv_obj_add_state(ui_LabelFreqValue, LV_STATE_USER_1);
          for (int i = potDir_InitialValue; i <= savedPotDir; i++)
          {
            ledcWrite(0, i);
            delay(100);
          }
          lv_obj_clear_state(ui_LabelFreqValue, LV_STATE_USER_1);
          flag_potencia_cero = 0;
        }

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
        flag1_alarms = 1;
        flag_potencia_cero = 1;
        flag_inicio_alarma = 1;
        flag1_temp_alarm = 1;
        flag2_temp_alarm = 0;
      }
    }
    if ((map_temp_in < 70))
    {
      if (flag2_temp_alarm == 0 && flag1_temp_alarm == 1)
      {
        lv_obj_clear_state(ui_LabelTemperatureValue, LV_STATE_USER_1);
        lv_obj_clear_state(ui_LabelTemperatureValue, LV_STATE_USER_3);
        lv_obj_add_state(ui_LabelTemperatureValue, LV_STATE_USER_2);

        lv_obj_clear_state(ui_ImageTemperature, LV_STATE_DEFAULT);
        lv_obj_clear_state(ui_ImageTemperature, LV_STATE_USER_3);
        lv_obj_add_state(ui_ImageTemperature, LV_STATE_USER_2);
        // TODO: cambiar fade out
        lv_obj_fade_out(ui_ImageAlarm, 100, 0);
        flag1_alarms = 0;
        flag_inicio_alarma = 1;
        flag2_temp_alarm = 1;
        flag1_temp_alarm = 0;
      }
    }

    if (map_potRef_in > (map_potDir_in / 10.0))
    {
      if (flag1_potRef == 0)
      {
        flag_inicio_alarma = 1;
        flag_potencia_cero = 1;
        flag1_alarms = 1;
        flag2_potRef = 0;
        flag1_potRef = 1;
      }
    }

    if (map_potRef_in < (map_potDir_in / 10.0))
    {
      if (flag1_potRef == 1 && flag2_potRef == 0)
      {
        flag_inicio_alarma = 1;
        if (flag_potencia_cero == 1)
        {
          lv_obj_fade_out(ui_ImageAlarm, 100, 0);
          // * Inicializar Potencia
          int savedPotDir = preferences.getInt("potDir", false);
          lv_obj_clear_state(ui_LabelFreqValue, LV_STATE_DEFAULT);
          lv_obj_add_state(ui_LabelFreqValue, LV_STATE_USER_1);
          for (int i = potDir_InitialValue; i <= savedPotDir; i++)
          {
            ledcWrite(0, i);
            delay(100);
          }
          lv_obj_clear_state(ui_LabelFreqValue, LV_STATE_USER_1);
          flag_potencia_cero = 0;
        }
        flag1_alarms = 0;
        flag2_potRef = 1;
        flag1_potRef = 0;
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
  pinMode(led_out, OUTPUT);
  // --------------------------------

  ledcSetup(0, 5000, 8);
  ledcAttachPin(potDir_out, 0);

  ledcSetup(1, 5000, 8);
  ledcAttachPin(led_out, 1);
  ledcWrite(1, 255);
  time4 = millis();

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
    lv_obj_fade_in(ui_ImageStereo, 0, 0);
    lv_obj_fade_out(ui_ImageMPX, 0, 0);
  }

  if (preferences.getChar("mpx", false) == 77)
  {
    digitalWrite(mpx_out, LOW);
    lv_roller_set_selected(ui_rollerMPX, 1, LV_ANIM_OFF);
    lv_obj_fade_out(ui_ImageStereo, 0, 0);
    lv_obj_fade_in(ui_ImageMPX, 0, 0);
  }

  lv_roller_set_selected(ui_rollerFreq1, preferences.getInt("roller1", false), LV_ANIM_OFF);
  lv_roller_set_selected(ui_rollerFreq2, preferences.getInt("roller2", false), LV_ANIM_OFF);

  // TODO: Set initial value for potDir
  lv_slider_set_value(ui_SliderPotDir, preferences.getInt("potDir", false), LV_ANIM_OFF);
  lv_label_set_text_fmt(ui_LabelPotValue, "%d", preferences.getInt("potDir", false));

  lv_label_set_text(ui_LabelPotDirValue, "0");
  lv_label_set_text(ui_LabelPotRefValue, "0");
  lv_label_set_text(ui_LabelTemperatureValue, "0");
  // lv_label_set_text(ui_LabelPotRefValue, "0");

  // char valuePotDir_str[4];
  // dtostrf(lv_slider_get_value(ui_SliderPotDir), 3, 0, valuePotDir_str);
  // String strValuePotDir = valuePotDir_str + "W";
  // lv_label_set_text(ui_LabelPotValue, valuePotDir_str);

  xTaskCreatePinnedToCore(main_func,
                          "main_func",
                          4000,
                          NULL,
                          0,
                          NULL,
                          1);
}

int potDirOutMap;

void loop()
{
  lv_timer_handler(); /* let the GUI do its work */

  // Serial.print("Slider Value: ");
  // Serial.println(lv_slider_get_value(ui_SliderPotDir));

  if (millis() - time4 > 600000)
  {
    ledcWrite(1, 30);
  }
  // !---
  // lv_slider_set_value(ui_SliderPotDir, preferences.getInt("potDir", false), LV_ANIM_OFF);

  // Serial.println(lv_obj_get_state(ui_SliderPotDir));

  if (lv_obj_get_state(ui_SliderPotDir) == 4130)
  {
    lv_label_set_text_fmt(ui_LabelPotValue, "%d", lv_slider_get_value(ui_SliderPotDir));
  }

  if (lv_obj_get_state(ui_btnAjustarPotencia) == 35)
  {
    int ui_SliderPotDirValue = lv_slider_get_value(ui_SliderPotDir);
    int map_uiSliderPotDirValue = map(ui_SliderPotDirValue, 0, 100, 60, 255);
    int savedPotDir = preferences.getInt("potDir", false);

    Serial.print("SAVED POT DIR: ");
    Serial.println(savedPotDir);
    Serial.print("MAP UI SLIDER POT DIR VALUE: ");
    Serial.println(map_uiSliderPotDirValue);

    if (savedPotDir > map_uiSliderPotDirValue)
    {
      potDirOutMap = map(savedPotDir, 0, 100, 60, 255);
      for (int i = savedPotDir; i >= map_uiSliderPotDirValue; i--)
      {
        Serial.print("Enviando potencia... ");
        Serial.println(i);
        ledcWrite(0, i);
        delay(50);
      }
    }
    else
    {

      for (int i = savedPotDir; i <= map_uiSliderPotDirValue; i++)
      {
        Serial.print("Enviando potencia... ");
        Serial.println(i);
        ledcWrite(0, i);
        delay(50);
      }
    }
    Serial.print("Potencia obtenida del Slider: ");
    Serial.println(ui_SliderPotDirValue);
    Serial.print("Potencia enviada a memoria: ");
    preferences.putInt("potDir", ui_SliderPotDirValue);
    Serial.println(preferences.getInt("potDir", false));
  }

  lv_roller_get_selected_str(ui_rollerMPX, valueRollerMPX_str, 0);

  if (valueRollerMPX_str[0] == 'S')
  {
    preferences.putChar("mpx", 'S');
    digitalWrite(mpx_out, HIGH);
    // lv_obj_clear_state(ui_ImageStereo, LV_STATE_DEFAULT);
    // lv_obj_clear_state(ui_ImageStereo, LV_STATE_USER_1);
    // lv_obj_add_state(ui_ImageStereo, LV_STATE_USER_2);
    // lv_obj_clear_state(ui_ImageMPX, LV_STATE_DEFAULT);
    // lv_obj_clear_state(ui_ImageMPX, LV_STATE_USER_2);
    // lv_obj_add_state(ui_ImageMPX, LV_STATE_USER_1);
    lv_obj_set_style_opa(ui_ImageStereo, 255, 0);
    lv_obj_set_style_opa(ui_ImageMPX, 0, 0);

    //! -----
    // lv_label_set_text(ui_LabelStereoMpx, "-STEREO-");
    // lv_obj_clear_state(ui_LabelFreqValue, LV_STATE_DEFAULT);
    // lv_obj_clear_state(ui_LabelFreqValue, LV_STATE_USER_2);
    // lv_obj_add_state(ui_LabelFreqValue, LV_STATE_USER_1);
  }

  if (valueRollerMPX_str[0] == 'M')
  {
    preferences.putChar("mpx", 'M');
    digitalWrite(mpx_out, LOW);
    // lv_obj_clear_state(ui_ImageStereo, LV_STATE_DEFAULT);
    // lv_obj_clear_state(ui_ImageStereo, LV_STATE_USER_2);
    // lv_obj_add_state(ui_ImageStereo, LV_STATE_USER_1);
    // lv_obj_clear_state(ui_ImageMPX, LV_STATE_DEFAULT);
    // lv_obj_clear_state(ui_ImageMPX, LV_STATE_USER_1);
    // lv_obj_add_state(ui_ImageMPX, LV_STATE_USER_2);
    lv_obj_set_style_opa(ui_ImageStereo, 0, 0);
    lv_obj_set_style_opa(ui_ImageMPX, 255, 0);
    // lv_obj_fade_in(ui_ImageStereo, 0, 0);
    // lv_obj_fade_out(ui_ImageMPX, 0, 0);
    //! ---
    // lv_label_set_text(ui_LabelStereoMpx, "-MPX-");
    // lv_obj_clear_state(ui_LabelFreqValue, LV_STATE_DEFAULT);
    // lv_obj_clear_state(ui_LabelFreqValue, LV_STATE_USER_1);
    // lv_obj_add_state(ui_LabelFreqValue, LV_STATE_USER_2);
  }

  delay(20);
}
