#pragma once

// Every force include must be processed before the precompiled header is
// compiled, just as it is when no precompiled header is involved.
#ifndef FORCED_INCLUDED
#  error "forced.h was not force-included before the precompiled header"
#endif
#ifndef FORCED2_INCLUDED
#  error "forced2.h was not force-included before the precompiled header"
#endif
#ifndef FORCED3_INCLUDED
#  error "forced3.h was not force-included before the precompiled header"
#endif
#ifndef FORCED4_INCLUDED
#  error "forced4.h was not force-included before the precompiled header"
#endif
#ifndef FORCED5_INCLUDED
#  error "forced5.h was not force-included before the precompiled header"
#endif

#define PCH_INCLUDED 1
