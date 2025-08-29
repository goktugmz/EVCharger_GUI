#include "custom.h"
#include "config.h"
#include "lvgl.h"
#include <stdio.h>

#undef  lv_snprintf
#define lv_snprintf  snprintf   // float için libc snprintf kullan

/*Global */
static EVConfig   g_cfg;
static bool       charging        = false;
static float      energy_kwh      = 0.0f;
static int        current_limit_a = 16;
static uint32_t   seconds         = 0;
static lv_timer_t *sim_timer      = NULL;
static lv_ui     *g_ui            = NULL;

/*pointer için güvenlik makrosu*/
#define SAFE(o) do{ if(!(o)) return; }while(0)

/* UI güncelle*/
static void update_ui(void) {
    SAFE(g_ui);
    char buf[64];

    /* status + renk değişimi */
    if (charging) {
        lv_obj_set_style_text_color(g_ui->screen_lbl_status, lv_color_hex(0x00E676), 0); // yeşil
        lv_label_set_text(g_ui->screen_lbl_status, "Status: CHARGING");
    } else {
        lv_obj_set_style_text_color(g_ui->screen_lbl_status, lv_color_hex(0xFF3D00), 0); // kırmızı
        lv_label_set_text(g_ui->screen_lbl_status, "Status: IDLE");
    }

    /* energy */
    lv_snprintf(buf, sizeof(buf), "Energy: %.3f kWh", energy_kwh);
    lv_label_set_text(g_ui->screen_lbl_kwh, buf);

    /* cost */
    lv_snprintf(buf, sizeof(buf), "Cost: %.2f TL", energy_kwh * g_cfg.price_per_kwh);
    lv_label_set_text(g_ui->screen_lbl_cost, buf);

    /* time */
    uint32_t h = seconds/3600, m = (seconds%3600)/60, s = seconds%60;
    lv_snprintf(buf, sizeof(buf), "Time: %02u:%02u:%02u", (unsigned)h,(unsigned)m,(unsigned)s);
    lv_label_set_text(g_ui->screen_lbl_time, buf);

    /* current */
    lv_snprintf(buf, sizeof(buf), "Current: %d A", current_limit_a);
    lv_label_set_text(g_ui->screen_lbl_current, buf);

    /* şarj yüzdesi arc'da */
    int32_t percent = (int32_t)((energy_kwh / g_cfg.full_kwh) * 100.0f);
    if (percent < 0)   percent = 0;
    if (percent > 100) percent = 100;
    lv_arc_set_value(g_ui->screen_arc_progress, percent);
}

/*1s sim tick*/
static void sim_tick_cb(lv_timer_t *t) {
    LV_UNUSED(t);
    if (!charging) return;
    float power_kw = (g_cfg.voltage_v * current_limit_a) / 1000.0f;
    energy_kwh += power_kw / 3600.0f;
    seconds += 1;
    update_ui();
}

/*Events*/
void start_btn_cb(lv_event_t * e){
    LV_UNUSED(e);
    charging = true;
    if (!sim_timer) sim_timer = lv_timer_create(sim_tick_cb, 1000, NULL);
    else            lv_timer_resume(sim_timer);
    update_ui();
}

void stop_btn_cb(lv_event_t * e){
    LV_UNUSED(e);
    charging = false;
    if (sim_timer) lv_timer_pause(sim_timer);
    update_ui();
}

void reset_btn_cb(lv_event_t * e){
    LV_UNUSED(e);
    charging  = false;
    energy_kwh = 0.0f;
    seconds    = 0;
    if (sim_timer) lv_timer_pause(sim_timer);
    update_ui();
}

void current_slider_cb(lv_event_t * e){
    lv_obj_t *slider = lv_event_get_target(e);
    current_limit_a = (int)lv_slider_get_value(slider);
    update_ui();
}

/* Switch: renkler stilden (stateful) gelir; callback sadece iş yapar */
void lock_switch_cb(lv_event_t * e){
    lv_obj_t *sw = lv_event_get_target(e);
    bool locked = lv_obj_has_state(sw, LV_STATE_CHECKED);

    SAFE(g_ui);
    if (locked) {
        /* ON: kilitli -> şarjı durdur, butonları kilitle, status güncelle */
        charging = false;
        if (sim_timer) lv_timer_pause(sim_timer);
        lv_obj_add_state(g_ui->screen_btn_start, LV_STATE_DISABLED);
        lv_obj_add_state(g_ui->screen_btn_stop,  LV_STATE_DISABLED);
        lv_label_set_text(g_ui->screen_lbl_status, "Status: LOCKED");
    } else {
        /* OFF: kilit açık -> butonları aç, status'u update_ui belirlesin */
        lv_obj_clear_state(g_ui->screen_btn_start, LV_STATE_DISABLED);
        lv_obj_clear_state(g_ui->screen_btn_stop,  LV_STATE_DISABLED);
        update_ui();
    }
}

/*Başlangıç*/
void custom_init(lv_ui *ui) {
    g_ui = ui;

    /* Config */
    config_defaults(&g_cfg);
    config_load_file("config.txt", &g_cfg);

    /* Slider & Arc */
    lv_slider_set_range(ui->screen_sld_current, g_cfg.min_current, g_cfg.max_current);
    current_limit_a = (g_cfg.min_current + g_cfg.max_current) / 2;
    lv_slider_set_value(ui->screen_sld_current, current_limit_a, LV_ANIM_OFF);

    lv_arc_set_range(ui->screen_arc_progress, 0, 100);
    lv_arc_set_value(ui->screen_arc_progress, 0);

    /* Başlangıç label değerleri */
    lv_label_set_text(ui->screen_lbl_status,  g_cfg.lock_on_start ? "Status: LOCKED":"Status: IDLE");
    lv_label_set_text(ui->screen_lbl_kwh,     "Energy: 0.000 kWh");
    lv_label_set_text(ui->screen_lbl_cost,    "Cost: 0.00 TL");
    lv_label_set_text(ui->screen_lbl_time,    "Time: 00:00:00");
    lv_label_set_text(ui->screen_lbl_current, "Current: -- A");

    /* StateCont görünümü */
    lv_obj_set_style_radius  (ui->screen_StateCont, 12, LV_PART_MAIN);
    lv_obj_set_style_bg_color(ui->screen_StateCont, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa  (ui->screen_StateCont, LV_OPA_50,             LV_PART_MAIN); // %50 şeffaf
    lv_obj_set_style_pad_all (ui->screen_StateCont, 0,   LV_PART_MAIN);
    lv_obj_set_style_pad_row (ui->screen_StateCont, 35,  LV_PART_MAIN);
    lv_obj_set_flex_flow     (ui->screen_StateCont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align    (ui->screen_StateCont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    /* Label ortak stilleri */
    lv_obj_set_style_text_color(ui->screen_lbl_current, lv_color_white(), 0);
    lv_obj_set_style_text_color(ui->screen_lbl_cost,    lv_color_white(), 0);
    lv_obj_set_style_text_color(ui->screen_lbl_status,  lv_color_white(), 0);
    lv_obj_set_style_text_color(ui->screen_lbl_time,    lv_color_white(), 0);
    lv_obj_set_style_text_color(ui->screen_lbl_kwh,     lv_color_white(), 0);

    /* StateCont içindeki sıra */
    lv_obj_move_to_index(ui->screen_lbl_kwh,     1);
    lv_obj_move_to_index(ui->screen_lbl_status,  2);
    lv_obj_move_to_index(ui->screen_lbl_cost,    3);
    lv_obj_move_to_index(ui->screen_lbl_time,    4);
    lv_obj_move_to_index(ui->screen_lbl_current, 0);

    /* Switch durum-bağımlı (stateful) renkler*/
    lv_obj_t *sw = ui->screen_swt_lock;

    /* Başlangıçta kapalı*/
    lv_obj_clear_state(sw, LV_STATE_CHECKED);

    /* kilit OFF=Kırmızı, ON=Yeşil */
    lv_obj_set_style_bg_opa  (sw, LV_OPA_COVER,  LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(sw, lv_color_hex(0xE53935),  LV_PART_INDICATOR | LV_STATE_DEFAULT); // kırmızı
    lv_obj_set_style_bg_color(sw, lv_color_hex(0x00C853),  LV_PART_INDICATOR | LV_STATE_CHECKED); // yeşil

    /*Event bağlama*/
    lv_obj_add_event_cb(ui->screen_btn_start,   start_btn_cb,     LV_EVENT_CLICKED,       NULL);
    lv_obj_add_event_cb(ui->screen_btn_stop,   stop_btn_cb,      LV_EVENT_CLICKED,       NULL);
    lv_obj_add_event_cb(ui->screen_btn_reset,   reset_btn_cb,      LV_EVENT_CLICKED,       NULL);
    lv_obj_add_event_cb(ui->screen_sld_current, current_slider_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(ui->screen_swt_lock,   lock_switch_cb,   LV_EVENT_VALUE_CHANGED, NULL);

    /* Buton/Slider stilleri */
    lv_obj_set_style_radius    (ui->screen_btn_start, 16, 0);
    lv_obj_set_style_bg_color  (ui->screen_btn_start, lv_color_hex(0x2196F3), 0);
    lv_obj_set_style_text_color(ui->screen_btn_start, lv_color_white(), 0);
    lv_obj_set_style_shadow_width(ui->screen_btn_start, 8, 0);

    lv_obj_set_style_radius    (ui->screen_btn_stop, 16, 0);
    lv_obj_set_style_bg_color  (ui->screen_btn_stop,  lv_color_hex(0xE53935), 0);
    lv_obj_set_style_text_color(ui->screen_btn_stop,  lv_color_white(), 0);

    lv_obj_set_style_radius    (ui->screen_btn_reset, 16, 0);
    lv_obj_set_style_bg_color  (ui->screen_btn_reset, lv_color_hex(0x9E9E9E), 0);
    lv_obj_set_style_text_color(ui->screen_btn_reset, lv_color_white(), 0);

    lv_obj_set_style_bg_color  (ui->screen_sld_current, lv_color_hex(0x444444),  LV_PART_MAIN);
    lv_obj_set_style_bg_color  (ui->screen_sld_current, lv_color_hex(0x00BFFF),  LV_PART_INDICATOR);
    lv_obj_set_style_bg_color  (ui->screen_sld_current, lv_color_white(),        LV_PART_KNOB);

    lv_obj_set_style_bg_color  (ui->screen_arc_progress,  lv_color_hex(0x263238), LV_PART_MAIN);
    lv_obj_set_style_arc_color (ui->screen_arc_progress,  lv_color_hex(0x00C853), LV_PART_INDICATOR);

    update_ui();
}
