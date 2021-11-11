#pragma once


#if 0
#ifndef VIPOC_H

//TODO(Vasko): change this when linux support is available
#ifndef _WIN32
#error Vipoc currently only supports windows
#endif

#include "include/defines.h"







#ifdef VIPOC_EXPORT
#define VP_API __declspec(dllexport)
#else
#define VP_API __declspec(dllimport)
#endif






#include "include/Core.h"




// USER ACCESS LIBRARIES
#include "log.h"


#define VIPOC_H
#endif

#endif