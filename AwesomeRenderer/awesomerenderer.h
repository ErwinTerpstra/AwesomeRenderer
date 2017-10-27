#ifndef _AWESOME_RENDERER_H_
#define _AWESOME_RENDERER_H_

#define PI			3.14159265358979323846f
#define TWO_PI		6.28318530717958647693f
#define FOUR_PI		12.56637061435917295385f
#define HALF_PI		1.57079632679489661923f

#define INV_PI		(1.0f / PI)
#define INV_TWO_PI	(1.0f / TWO_PI)
#define INV_FOUR_PI	(1.0f / FOUR_PI)

#define AR_INLINE inline
#define AR_FORCE_INLINE AR_INLINE __forceinline

#include "debug.h"

// Data types
#include "extension.h"
#include "extensionprovider.h"
#include "typedefs.h"
#include "color.h"

#include "random.h"
#include "util.h"
#include "vectorutil.h"
#include "quaternionutil.h"
#include "sampleutil.h"
#include "colorutil.h"

#include "memory.h"
#include "alignmentallocator.h"

#endif