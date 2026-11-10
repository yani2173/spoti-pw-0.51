#import <Foundation/Foundation.h>

// Installs once, only when the pinned 0.51.0 lyrics class has the verified callback signature.
void SGInstallPerformance(void);

extern NSString *const SGPerformanceLightModeKey;
void SGSetLightMode(BOOL on);
extern NSString *const SGPerformanceAutomaticLightModeKey;
void SGSetAutomaticLightMode(BOOL on);
NSString *SGPerformanceLightModeStatus(void);

// One main-thread observer of the effective (manual or automatic) Light mode. It is called
// immediately with the current state, then after every change, including the saved state at startup.
void SGPerformanceSetLightModeObserver(void (*observer)(BOOL lightMode));

// The sung-word effects module is built only into the iOS add-on. It reports what it is doing
// through this provider so Mod Settings can show it; without one the status says so.
void SGPerformanceSetWordEffectsStatus(NSString *(*provider)(void));
NSString *SGPerformanceWordEffectsStatus(void);
