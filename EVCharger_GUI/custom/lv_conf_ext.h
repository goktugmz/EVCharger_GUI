/*
* Copyright 2024 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

/*
 * lv_conf_ext.h for custom lvconf file.
 * example :
 *	#undef LV_FONT_FMT_TXT_LARGE
 *  #define LV_FONT_FMT_TXT_LARGE 1
 */

#ifndef LV_CONF_EXT_H
#define LV_CONF_EXT_H

/* common code  begin  */

/* LVGL'de mini sprintf yerine derleyicinin sprintf/snprintf'ini kullan */
#ifdef LV_USE_SPRINTF
# undef LV_USE_SPRINTF
#endif
#define LV_USE_SPRINTF 1

/* Bazı sürümlerde bu makro kullanılıyor: mini sprintf'i kapat */
#ifdef LV_SPRINTF_CUSTOM
# undef LV_SPRINTF_CUSTOM
#endif
#define LV_SPRINTF_CUSTOM 0

/* common code end */



#if LV_USE_GUIDER_SIMULATOR
/* code for simulator begin  */


/* code for simulator end */
#else
/* code for board begin */


/* code for board end */	
#endif



#endif  /* LV_CONF_EXT_H */	