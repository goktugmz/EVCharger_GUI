#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static void trim(char *s) {
    int n = (int)strlen(s);
    while (n>0 && (s[n-1]=='\r' || s[n-1]=='\n' || isspace((unsigned char)s[n-1]))) {
        s[--n] = 0;
    }
    int i=0;
    while (s[i] && isspace((unsigned char)s[i])) i++;
    if (i>0) memmove(s, s+i, strlen(s+i)+1);
}
//default değerler
void config_defaults(EVConfig *cfg) {
    cfg->price_per_kwh = 6.50f;
    cfg->voltage_v     = 230;
    cfg->min_current   = 6;
    cfg->max_current   = 32;
    cfg->full_kwh      = 10.0f;
    cfg->lock_on_start = false;
}

static void set_kv(EVConfig *cfg, const char *k, const char *v) {
    if      (!strcmp(k, "price_per_kwh")) cfg->price_per_kwh = (float)atof(v);
    else if (!strcmp(k, "voltage_v"))     cfg->voltage_v     = atoi(v);
    else if (!strcmp(k, "min_current"))   cfg->min_current   = atoi(v);
    else if (!strcmp(k, "max_current"))   cfg->max_current   = atoi(v);
    else if (!strcmp(k, "full_kwh"))      cfg->full_kwh      = (float)atof(v);
    else if (!strcmp(k, "lock_on_start")) {
        if (!strcmp(v, "true") || !strcmp(v, "1")) cfg->lock_on_start = true;
        else cfg->lock_on_start = false;
    }
}

bool config_load_file(const char *path, EVConfig *cfg) {
    FILE *f = fopen(path, "r");
    if (!f) return false;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        trim(line);
        if (line[0]=='#' || line[0]==';' || line[0]==0) continue;
        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = 0;
        char *key = line;
        char *val = eq + 1;
        trim(key); trim(val);
        set_kv(cfg, key, val);
    }
    fclose(f);
    return true;
}

bool config_save_file(const char *path, const EVConfig *cfg) {
    FILE *f = fopen(path, "w");
    if (!f) return false;

    fprintf(f, "price_per_kwh=%.3f\n", cfg->price_per_kwh);
    fprintf(f, "voltage_v=%d\n",        cfg->voltage_v);
    fprintf(f, "min_current=%d\n",      cfg->min_current);
    fprintf(f, "max_current=%d\n",      cfg->max_current);
    fprintf(f, "full_kwh=%.3f\n",       cfg->full_kwh);
    fprintf(f, "lock_on_start=%s\n",    cfg->lock_on_start ? "true":"false");

    fclose(f);
    return true;
}
