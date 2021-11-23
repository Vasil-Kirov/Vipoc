#pragma once

#include "application.h"
#include "input.h"
#include "log.h"
#include "renderer/renderer.h"



extern void vp_start(vp_game *game);

int main()
{
    logger_init();
    vp_game game;
    vp_start(&game);
    application_create(&game);
}

