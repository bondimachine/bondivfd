/*
 * vfd_char.c - Definicion de caracteres custom en tiempo de ejecucion. Esta
 * aparte de vfd.c para que el firmware del 80C537, que solo usa los
 * caracteres de glyphs.h, no lo tenga que linkear.
 */
#include "vfd.h"
#define GLYPHS_DEFINE_DOT_MAP
#include "glyphs.h"

uint8_t vfd_define_char(uint8_t slot, const uint8_t rows[7])
{
    uint8_t data[6] = { 0, 0, 0, 0, 0, 0 };
    uint8_t n, m;

    if (slot >= VFD_UDC_SLOTS || vfd_room() < 8)
        return 0;
    for (n = 0; n < 35; n++) {
        if (rows[n / 5] & (0x10 >> (n % 5))) {
            m = dot_map[n];
            data[m >> 4] |= (uint8_t)(1 << (m & 0x0F));
        }
    }
    vfd_write(VFD_LOAD_UDC);
    vfd_write(slot);
    for (n = 0; n < 6; n++)
        vfd_write(data[n]);         /* bit E en 0: un caracter por carga */
    return 1;
}
