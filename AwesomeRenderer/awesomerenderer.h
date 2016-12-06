#ifndef _AWESOME_RENDERER_H_
#define _AWESOME_RENDERER_H_

#define PI			3.14159265358979323846
#define TWO_PI		6.28318530717958647693
#define FOUR_PI		12.56637061435917295385
#define HALF_PI		1.57079632679489661923
#define FRTHRD_PI	4.18879020478639098462

#define INV_PI		1.0 / PI
#define INV_TWO_PI	1.0 / TWO_PI

#define AR_INLINE inline
#define AR_FORCE_INLINE AR_INLINE __forceinline

#include "debug.h"

// Data types
#include "extension.h"
#include "extensionprovider.h"
#include "typedefs.h"
#include "color.h"

#include "util.h"

#endif