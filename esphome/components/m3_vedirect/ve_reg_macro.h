#define PASTE_(x, y) x##y
#define PASTE(x, y) PASTE_(x, y)
#define PASTE3_(x, y, z) x##y##z
#define PASTE3(x, y, z) PASTE3_(x, y, z)
#define Y(...) __VA_ARGS__
#define N(...)
#define IF(x) x  // alternate method similar to IFNOT()

#define NOT_N Y
#define NOT_Y N
#define IF_NOT(x) PASTE(NOT_, x)
#define NOT(x) PASTE(NOT_, x)

#define N_OR_N N
#define N_OR_Y Y
#define Y_OR_N Y
#define Y_OR_Y Y
#define OR(x, y) PASTE3(x, _OR_, y)

#define N_AND_N N
#define N_AND_Y N
#define Y_AND_N N
#define Y_AND_Y Y
#define AND(x, y) PASTE3(x, _AND_, y)

#define N_XOR_N N
#define N_XOR_Y Y
#define Y_XOR_N Y
#define Y_XOR_Y N
#define XOR(x, y) PASTE3(x, _XOR_, y)

#define N_NOR_N Y
#define N_NOR_Y N
#define Y_NOR_N N
#define Y_NOR_Y N
#define NOR(x, y) PASTE3(x, _NOR_, y)

#define N_NAND_N Y
#define N_NAND_Y Y
#define Y_NAND_N Y
#define Y_NAND_Y N
#define NAND(x, y) PASTE3(x, _NAND_, y)

#define N_XNOR_N Y
#define N_XNOR_Y N
#define Y_XNOR_N N
#define Y_XNOR_Y Y
#define XNOR(x, y) PASTE3(x, _XNOR_, y)

#define IF2(x, y, z) PASTE3(x, y, z)

#if 1
#define FLAVOR_MPPT_BS
#define FLAVOR_MPPT_RS
#define FLAVOR_INV_PHNX
#define FLAVOR_BMV
#define FLAVOR_BMV71
#endif

// Inverter flavors
#ifdef FLAVOR_INV_PHNX
#define FLAVOR_INV
#define DEF_INV_PHNX Y
#else
#define DEF_INV_PHNX N
#endif

// Charger flavors
#ifdef FLAVOR_CHG_PHNX
#define FLAVOR_CHG
#define DEF_CHG_PHNX Y
#else
#define DEF_CHG_PHNX N
#endif

// MPPT charger flavors
#ifdef FLAVOR_MPPT_BS  // BlueSolar MPPT
#define FLAVOR_MPPT
#define DEF_MPPT_BS Y
#else
#define DEF_MPPT_BS N
#endif

#ifdef FLAVOR_MPPT_RS
#define FLAVOR_MPPT
#define DEF_MPPT_RS Y
#else
#define DEF_MPPT_RS N
#endif

// Battery Monitor flavors
#ifdef FLAVOR_BMV60
#define FLAVOR_BMV
#define DEF_BMV60 Y
#else
#define DEF_BMV60 N
#endif

#ifdef FLAVOR_BMV70
#define FLAVOR_BMV
#define DEF_BMV70 Y
#else
#define DEF_BMV70 N
#endif

#ifdef FLAVOR_BMV71
#define FLAVOR_BMV
#define DEF_BMV71 Y
#else
#define DEF_BMV71 N
#endif

// define some 'groups'
#ifdef FLAVOR_MPPT  // any MPPT charger flavor
#define FLAVOR_CHG
#define DEF_MPPT Y
#else
#define DEF_MPPT N
#endif

#ifdef FLAVOR_BMV  // any BMV flavor
#define DEF_BMV Y
#else
#define DEF_BMV N
#endif

#ifdef FLAVOR_CHG  // any charger flavor
#define DEF_CHG Y
#else
#define DEF_CHG N
#endif

#ifdef FLAVOR_INV  // any inverter flavor
#define DEF_INV Y
#else
#define DEF_INV N
#endif