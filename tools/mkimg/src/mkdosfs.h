#define ENDIAN_16(x)   ((((x) & 0xFF) << 8) | (((x) & 0xFF00) >> 8))
#define ENDIAN_32(x)   ((((x) & 0xFF) << 24) | (((x) & 0xFF00) << 8) | (((x) & 0xFF0000) >> 8) | (((x) & 0xFF000000) >> 24))
#define POWERofTWO(x) (!((x) & ((x) - 1)) && (x))
#define TRUE   1
#define FALSE  0
typedef   signed  char      bit8s;
typedef unsigned  char      bit8u;
typedef   signed short      bit16s;
typedef unsigned short      bit16u;
typedef   signed  long      bit32s;
typedef unsigned  long      bit32u;
typedef   signed  long long  bit64s;
typedef unsigned  long long  bit64u;
