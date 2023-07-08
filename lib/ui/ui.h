// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.2.3
// LVGL version: 8.2.0
// Project name: SquareLine_Project

#ifndef _SQUARELINE_PROJECT_UI_H
#define _SQUARELINE_PROJECT_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined __has_include
#if __has_include("lvgl.h")
#include "lvgl.h"
#elif __has_include("lvgl/lvgl.h")
#include "lvgl/lvgl.h"
#else
#include "lvgl.h"
#endif
#else
#include "lvgl.h"
#endif

#include "ui_events.h"
void fan_rotate_Animation(lv_obj_t * TargetObject, int delay);
void fan_static_Animation(lv_obj_t * TargetObject, int delay);
void alarm_opacity_Animation(lv_obj_t * TargetObject, int delay);
void ui_event_LoadingScreen(lv_event_t * e);
extern lv_obj_t * ui_LoadingScreen;
extern lv_obj_t * ui_LabelIniciando;
extern lv_obj_t * ui_SpinnerIniciando;
void ui_event_ConfigScreen(lv_event_t * e);
extern lv_obj_t * ui_ConfigScreen;
extern lv_obj_t * ui_Panel4;
void ui_event_btnInfo(lv_event_t * e);
extern lv_obj_t * ui_btnInfo;
extern lv_obj_t * ui_lblPotTitle;
extern lv_obj_t * ui_rollerMPX;
extern lv_obj_t * ui_panelRollersFreq;
extern lv_obj_t * ui_rollerFreq1;
extern lv_obj_t * ui_lblDotFreq;
extern lv_obj_t * ui_rollerFreq2;
void ui_event_SliderPotDir(lv_event_t * e);
extern lv_obj_t * ui_SliderPotDir;
void ui_event_btnBack(lv_event_t * e);
extern lv_obj_t * ui_btnBack;
void ui_event_btnAjustarPotencia(lv_event_t * e);
extern lv_obj_t * ui_btnAjustarPotencia;
extern lv_obj_t * ui_lblBtnAjustarFreq1;
void ui_event_btnAjustarFreq(lv_event_t * e);
extern lv_obj_t * ui_btnAjustarFreq;
extern lv_obj_t * ui_Panel2;
extern lv_obj_t * ui_LabelPotValue;
extern lv_obj_t * ui_LabelPotValue1;
extern lv_obj_t * ui_MainScreen;
extern lv_obj_t * ui_PanelVuContainer;
extern lv_obj_t * ui_PanelModR;
extern lv_obj_t * ui_LabelModR;
extern lv_obj_t * ui_SliderModR;
extern lv_obj_t * ui_PanelModL;
extern lv_obj_t * ui_LabelModL;
extern lv_obj_t * ui_SliderModL;
extern lv_obj_t * ui_PanelModMPX;
extern lv_obj_t * ui_LabelModMPX;
extern lv_obj_t * ui_SliderModMPX;
extern lv_obj_t * ui_PanelPotRef;
extern lv_obj_t * ui_LabelPotRef;
extern lv_obj_t * ui_LabelPotRefValue;
extern lv_obj_t * ui_PanelPotDir;
extern lv_obj_t * ui_LabelPotDir;
extern lv_obj_t * ui_LabelPotDirValue;
extern lv_obj_t * ui_PanelFreqContainer;
extern lv_obj_t * ui_LabelFreqValue;
extern lv_obj_t * ui_LabelMHz;
extern lv_obj_t * ui_LabelMHz1;
extern lv_obj_t * ui_PanelIconsContainer;
extern lv_obj_t * ui_ImageAlarm;
extern lv_obj_t * ui_ImageTemperature;
extern lv_obj_t * ui_ImageFan;
extern lv_obj_t * ui_LabelTemperatureValue;
void ui_event_ImgButton3(lv_event_t * e);
extern lv_obj_t * ui_ImgButton3;
extern lv_obj_t * ui_ImageStereo;
extern lv_obj_t * ui_ImageMPX;
extern lv_obj_t * ui_AboutScreen;
void ui_event_ImgButton5(lv_event_t * e);
extern lv_obj_t * ui_ImgButton5;
extern lv_obj_t * ui_LabelIniciando1;
extern lv_obj_t * ui____initial_actions0;


LV_IMG_DECLARE(ui_img_btn_sq_info_inact_png);    // assets\btn_sq_info_inact.png
LV_IMG_DECLARE(ui_img_btn_sq_info_act_png);    // assets\btn_sq_info_act.png
LV_IMG_DECLARE(ui_img_indicator_large_hor_png);    // assets\indicator_large_hor.png
LV_IMG_DECLARE(ui_img_btn_sq_back_inact_png);    // assets\btn_sq_back_inact.png
LV_IMG_DECLARE(ui_img_btn_sq_back_act_png);    // assets\btn_sq_back_act.png
LV_IMG_DECLARE(ui_img_setfreq_inact_png);    // assets\setFreq_inact.png
LV_IMG_DECLARE(ui_img_setfreq_act_png);    // assets\setFreq_act.png
LV_IMG_DECLARE(ui_img_indicator_reduced_hor_png);    // assets\indicator_reduced_hor.png
LV_IMG_DECLARE(ui_img_alarm_png);    // assets\alarm.png
LV_IMG_DECLARE(ui_img_temp_png);    // assets\Temp.png
LV_IMG_DECLARE(ui_img_fan_png);    // assets\Fan.png
LV_IMG_DECLARE(ui_img_config_inact_png);    // assets\config_inact.png
LV_IMG_DECLARE(ui_img_config_act_png);    // assets\config_act.png
LV_IMG_DECLARE(ui_img_stereo_icon_png);    // assets\STEREO_ICON.png
LV_IMG_DECLARE(ui_img_mpx_icon_png);    // assets\MPX_ICON.png


LV_FONT_DECLARE(ui_font_Mitr);
LV_FONT_DECLARE(ui_font_MitrMedium);
LV_FONT_DECLARE(ui_font_MitrSmall);


void ui_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
