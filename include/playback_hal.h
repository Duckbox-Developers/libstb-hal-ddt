#include <config.h>
#if HAVE_DUCKBOX_HARDWARE
#include "../libduckbox/playback_libeplayer3.h"
#elif HAVE_SPARK_HARDWARE
#include "../libspark/playback_libeplayer3.h"
#elif HAVE_ARM_HARDWARE
#include "../libarmbox/playback_libeplayer3.h"
#elif HAVE_MIPS_HARDWARE
#include "../libmipsbox/playback_libeplayer3.h"
#elif HAVE_GENERIC_HARDWARE
#if BOXMODEL_RASPI
#include "../libraspi/playback_lib.h"
#else
#include "../libgeneric-pc/playback_lib.h"
#endif
#else
#error no valid hardware defined
#endif
