#pragma once

#include <stdint.h>

#include <param/param.h>

/* System registers */
uint32_t _GPIO1__DR;
uint32_t _GPIO1__GDIR;
uint32_t _GPIO2__DR;
uint32_t _GPIO2__GDIR;
uint32_t _GPIO3__GDIR;
uint32_t _GPIO4__GDIR;
uint32_t _GPIO5__GDIR;
uint32_t _MUB__SR;
uint32_t _RDC__STAT;
uint32_t _TMU__TSCR;
uint32_t _TMU__TRITSR;
uint32_t _TMU__TRATSR;

extern param_t GPC__SLPCR;
extern param_t GPC__LPCR_A53_BSC;
extern param_t GPC__LPCR_A53_AD;
extern param_t GPC__LPCR_M7;
extern param_t GPC__MLPCR;
extern param_t GPC_PGC__A53SCU_PUPSCR;
extern param_t GPIO1__DR;
extern param_t GPIO1__GDIR;
extern param_t GPIO2__DR;
extern param_t GPIO2__GDIR;
extern param_t GPIO3__GDIR;
extern param_t GPIO4__GDIR;
extern param_t GPIO5__GDIR;
extern param_t MUB__SR;
extern param_t RDC__STAT;
extern param_t TMU__TSCR;
extern param_t TMU__TRITSR;
extern param_t TMU__TRATSR;

#define PARAMID_GPC__SLPCR             1
#define PARAMID_GPC__LPCR_A53_BSC      2
#define PARAMID_GPC__LPCR_A53_AD       3
#define PARAMID_GPC__LPCR_M7           4
#define PARAMID_GPC__MLPCR             5
#define PARAMID_GPC_PGC__A53SCU_PUPSCR 6
#define PARAMID_GPIO1__DR              7
#define PARAMID_GPIO1__GDIR            8
#define PARAMID_GPIO2__DR              9
#define PARAMID_GPIO2__GDIR            10
#define PARAMID_GPIO3__GDIR            11
#define PARAMID_GPIO4__GDIR            12
#define PARAMID_GPIO5__GDIR            13
#define PARAMID_MUB__SR                14
#define PARAMID_RDC__STAT              15
#define PARAMID_TMU__TSCR              16
#define PARAMID_TMU__TRITSR            17
#define PARAMID_TMU__TRATSR            18

/* Application parameters */
extern param_t wake_a53;
extern param_t a53_status;
extern param_t tmu_reading;
extern param_t gnss_lat;
extern param_t gnss_lon;
extern param_t gnss_age;
extern param_t gnss_date;
extern param_t gnss_time;
extern param_t gnss_speed;
extern param_t gnss_alt;
extern param_t gnss_course;

#define PARAMID_WAKE_A53    21
#define PARAMID_A53_STATUS  22
#define PARAMID_TMU_READING 23
#define PARAMID_GNSS_LAT    24
#define PARAMID_GNSS_LON    25
#define PARAMID_GNSS_AGE    26
#define PARAMID_GNSS_DATE   27
#define PARAMID_GNSS_TIME   28
#define PARAMID_GNSS_SPEED  29
#define PARAMID_GNSS_ALT    30
#define PARAMID_GNSS_COURSE 31

/* General purpose registers */
uint8_t _p_uint8[32];
uint16_t _p_uint16[32];
uint32_t _p_uint32[32];
uint64_t _p_uint64[32];
int8_t _p_int8[32];
int16_t _p_int16[32];
int32_t _p_int32[32];
int64_t _p_int64[32];
float _p_float[32];
double _p_double[16];

extern param_t p_uint8;
extern param_t p_uint16;
extern param_t p_uint32;
extern param_t p_uint64;
extern param_t p_int8;
extern param_t p_int16;
extern param_t p_int32;
extern param_t p_int64;
extern param_t p_float;
extern param_t p_double;

#define PARAM_ID_UINT8  41
#define PARAM_ID_UINT16 42
#define PARAM_ID_UINT32 43
#define PARAM_ID_UINT64 44
#define PARAM_ID_INT8   45
#define PARAM_ID_INT16  46
#define PARAM_ID_INT32  47
#define PARAM_ID_INT64  48
#define PARAM_ID_FLOAT  49
#define PARAM_ID_DOUBLE 50
