#pragma once

#include "include/defines.h"
#include "application.h"
#include "log.h"



extern void vp_start(vp_game *game);

int main()
{
    logger_init();
    vp_game game;
    vp_start(&game);
    application_create(&game);
}

