#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include "configLovyan.h"
#include <lvgl.h>
#include "ui.h"

LGFX tft;

/*Change to your screen resolution*/
static const uint32_t screenWidth = 480;
static const uint32_t screenHeight = 320;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * 10];

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

// ---- EXAMPLES CODE -----
static lv_style_t style_btn;
static lv_style_t style_btn_pressed;
static lv_style_t style_btn_red;

static lv_color_t darken(const lv_color_filter_dsc_t *dsc, lv_color_t color, lv_opa_t opa)
{
  LV_UNUSED(dsc);
  return lv_color_darken(color, opa);
}

static void style_init(void)
{
  /* Create a simple button style */
  lv_style_init(&style_btn);
  lv_style_set_radius(&style_btn, 10);
  lv_style_set_bg_opa(&style_btn, LV_OPA_COVER);
  lv_style_set_bg_color(&style_btn, lv_palette_lighten(LV_PALETTE_GREY, 3));
  lv_style_set_bg_grad_color(&style_btn, lv_palette_main(LV_PALETTE_GREY));
  lv_style_set_bg_grad_dir(&style_btn, LV_GRAD_DIR_VER);

  lv_style_set_border_color(&style_btn, lv_color_black());
  lv_style_set_border_opa(&style_btn, LV_OPA_20);
  lv_style_set_border_width(&style_btn, 2);

  lv_style_set_text_color(&style_btn, lv_color_black());

  /*Create a style for the pressed state.
   *Use a color filter to simply modify all colors in this state*/
  static lv_color_filter_dsc_t color_filter;
  lv_color_filter_dsc_init(&color_filter, darken);
  lv_style_init(&style_btn_pressed);
  lv_style_set_color_filter_dsc(&style_btn_pressed, &color_filter);
  lv_style_set_color_filter_opa(&style_btn_pressed, LV_OPA_20);

  /*Create a red style. Change only some colors.*/
  lv_style_init(&style_btn_red);
  lv_style_set_bg_color(&style_btn_red, lv_palette_main(LV_PALETTE_RED));
  lv_style_set_bg_grad_color(&style_btn_red, lv_palette_lighten(LV_PALETTE_RED, 3));
}

/**
 * Create styles from scratch for buttons.
 */
void lv_example_get_started_2(void)
{
  /*Initialize the style*/
  style_init();

  /*Create a button and use the new styles*/
  lv_obj_t *btn = lv_btn_create(lv_scr_act());
  /* Remove the styles coming from the theme
   * Note that size and position are also stored as style properties
   * so lv_obj_remove_style_all will remove the set size and position too */
  lv_obj_remove_style_all(btn);
  lv_obj_set_pos(btn, 10, 10);
  lv_obj_set_size(btn, 120, 50);
  lv_obj_add_style(btn, &style_btn, 0);
  lv_obj_add_style(btn, &style_btn_pressed, LV_STATE_PRESSED);

  /*Add a label to the button*/
  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, "Button");
  lv_obj_center(label);

  /*Create another button and use the red style too*/
  lv_obj_t *btn2 = lv_btn_create(lv_scr_act());
  lv_obj_remove_style_all(btn2); /*Remove the styles coming from the theme*/
  lv_obj_set_pos(btn2, 10, 80);
  lv_obj_set_size(btn2, 120, 50);
  lv_obj_add_style(btn2, &style_btn, 0);
  lv_obj_add_style(btn2, &style_btn_red, 0);
  lv_obj_add_style(btn2, &style_btn_pressed, LV_STATE_PRESSED);
  lv_obj_set_style_radius(btn2, LV_RADIUS_CIRCLE, 0); /*Add a local style too*/

  label = lv_label_create(btn2);
  lv_label_set_text(label, "Button 2");
  lv_obj_center(label);
}

static lv_obj_t *label;

static void slider_event_cb(lv_event_t *e)
{
  lv_obj_t *slider = lv_event_get_target(e);

  /*Refresh the text*/
  lv_label_set_text_fmt(label, "%" LV_PRId32, lv_slider_get_value(slider));
  lv_obj_align_to(label, slider, LV_ALIGN_OUT_TOP_MID, 0, -15); /*Align top of the slider*/
}

/**
 * Create a slider and write its value on a label.
 */
void lv_example_get_started_3(void)
{
  /*Create a slider in the center of the display*/
  lv_obj_t *slider = lv_slider_create(lv_scr_act());
  lv_obj_set_width(slider, 200);                                              /*Set the width*/
  lv_obj_center(slider);                                                      /*Align to the center of the parent (screen)*/
  lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL); /*Assign an event function*/

  /*Create a label above the slider*/
  label = lv_label_create(lv_scr_act());
  lv_label_set_text(label, "0");
  lv_obj_align_to(label, slider, LV_ALIGN_OUT_TOP_MID, 0, -15); /*Align top of the slider*/
}

// ------------------------

void setup()
{
  Serial.begin(115200);

  tft.begin();
  tft.setRotation(1);
  tft.setBrightness(255);
  uint16_t calData[8] = {239, 3926, 233, 265, 3856, 3896, 3714, 308};
  // uint16_t calData[5] = {275, 3620, 264, 3532, 1};
  tft.setTouchCalibrate(calData);

  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);

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

  // lv_example_get_started_3();
  ui_init();
}

void loop()
{
  lv_timer_handler(); /* let the GUI do its work */
  delay(5);
}
