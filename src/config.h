// config.h
#pragma once

// Uncomment to enable uncapped presentation.
// Only active in debug builds.
//#define PRESENT_MODE_UNCAPPED

#define IN_TITLE_SHOW_FPS

/**
* DO NOT MODIFY BELOW
* Sets release build flags
*/
#ifdef NDEBUG
#undef PRESENT_MODE_UNCAPPED
//#undef IN_TITLE_SHOW_FPS
#endif