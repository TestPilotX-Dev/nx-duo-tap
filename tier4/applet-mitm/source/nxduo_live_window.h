/*
 * NX Paradox: the firmware window NX Duo Tap streams on, in one place. A
 * release install captures only on firmware majors MIN to MAX
 * (applet_mitm_nv.cpp RunLive; allow_untested_firmware = 1 lifts it). Shared
 * by the sysmodule (RunLive's gate, the firmware line in applet_mitm_main.cpp),
 * the Switch app (manager) and the overlay. C and C++.
 *
 * tools/make_release.sh stops a build whose Atmosphere supports a firmware
 * major above MAX (tools/check_live_window.sh): raise MAX once capture is
 * tested on that major. Plain decimal numbers - NXDUO_LIVE_TEXT spells them.
 */
#ifndef NXDUO_LIVE_WINDOW_H
#define NXDUO_LIVE_WINDOW_H

#define NXDUO_LIVE_MAJOR_MIN 21
#define NXDUO_LIVE_MAJOR_MAX 23

/* "21.x-23.x", a string literal for log lines and texts */
#define NXDUO_LIVE_STR_(n) #n
#define NXDUO_LIVE_STR(n)  NXDUO_LIVE_STR_(n)
#define NXDUO_LIVE_TEXT    NXDUO_LIVE_STR(NXDUO_LIVE_MAJOR_MIN) ".x-" NXDUO_LIVE_STR(NXDUO_LIVE_MAJOR_MAX) ".x"

#endif
