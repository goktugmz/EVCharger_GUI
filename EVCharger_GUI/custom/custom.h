#ifndef CUSTOM_H
#define CUSTOM_H

#include "gui_guider.h"
#include "config.h"

/* Event callback’ler */
void start_btn_cb(lv_event_t * e);
void stop_btn_cb(lv_event_t * e);
void reset_btn_cb(lv_event_t * e);
void current_slider_cb(lv_event_t * e);
void lock_switch_cb(lv_event_t * e);

/* Başlatma */
void custom_init(lv_ui *ui);

#endif
