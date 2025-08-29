#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

typedef struct {
    float price_per_kwh;  // tl/kWh
    int   voltage_v;      // Volt
    int   min_current;    // Min akım (A)
    int   max_current;    // Max akım (A)
    float full_kwh;       // %100 kabul edilecek enerji
    bool  lock_on_start;  // açılışta kilitli mi?
} EVConfig;

/* Varsayılanları yükle */
void config_defaults(EVConfig *cfg);

/* config.txt dosyasından okur (bulamazsa defaults kalır)
"C:\NXP\GUI-Guider-Projects\deneme2\lvgl-simulator\config.txt" adresi bu */
bool config_load_file(const char *path, EVConfig *cfg);

/* İsteğe bağlı debug amaçlı dosyaya yazar */
bool config_save_file(const char *path, const EVConfig *cfg);

#endif
