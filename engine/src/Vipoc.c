#include "Vipoc.h"

void vp_init(vp_game game)
{
    logger_init();
    application_create(&game);
};
