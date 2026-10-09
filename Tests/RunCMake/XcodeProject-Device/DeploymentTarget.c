#include <Availability.h>
#include <TargetConditionals.h>

#if TARGET_OS_OSX
#  if __MAC_OS_X_VERSION_MIN_REQUIRED != EXPECT_VERSION
#    error macOS deployment version mismatch
#  endif
#elif TARGET_OS_VISION
#  if __VISION_OS_VERSION_MIN_REQUIRED != EXPECT_VERSION
#    error visionOS deployment version mismatch
#  endif
#elif TARGET_OS_IOS
#  if __IPHONE_OS_VERSION_MIN_REQUIRED != EXPECT_VERSION
#    error iOS deployment version mismatch
#  endif
#elif TARGET_OS_WATCH
#  if __WATCH_OS_VERSION_MIN_REQUIRED != EXPECT_VERSION
#    error watchOS deployment version mismatch
#  endif
#elif TARGET_OS_TV
#  if __TV_OS_VERSION_MIN_REQUIRED != EXPECT_VERSION
#    error tvOS deployment version mismatch
#  endif
#else
#  error unknown OS
#endif

void foo(void)
{
}
