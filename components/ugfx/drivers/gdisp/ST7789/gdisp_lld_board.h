#ifndef _GDISP_LLD_BOARD_H
#define _GDISP_LLD_BOARD_H

#include "driver/st7789.h"

#define init_board(g)           st7789_init_board()
#define setpin_reset(g, rst)    st7789_setpin_reset(rst)
#define write_cmd(g, cmd)       st7789_write_cmd(cmd)
#define write_data(g, data)     st7789_write_data(data)
#define refresh_gram(g, gram)   st7789_refresh_gram(gram)

#endif /* _GDISP_LLD_BOARD_H */
