#!/usr/bin/env bash
# NX Paradox: the firmware window NX Duo Tap streams on
# (tier4/applet-mitm/source/nxduo_live_window.h) must reach the newest
# firmware major the Atmosphere it is built against supports. Below that, a
# console on that major runs the module but never streams (applet_mitm_nv.cpp
# RunLive), and only its log says why.
#
#   bash tools/check_live_window.sh ref/Atmosphere-1.12.2
#
# tools/make_release.sh runs it on the tree AMS_TREE picks (a relative path is
# taken from the repository root, as tier4/applet-mitm/build.sh does).
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
AMS="${1:?usage: check_live_window.sh ATMOSPHERE_TREE}"
case "$AMS" in /*) ;; *) AMS="$REPO/$AMS" ;; esac
LIVE_H="$REPO/tier4/applet-mitm/source/nxduo_live_window.h"
API_H="$AMS/libraries/libvapours/include/vapours/ams/ams_api_version.h"

# the number in "#define NAME <number>" (LF or CRLF), empty when there is none
num() { [ -f "$2" ] && tr -d '\r' < "$2" | sed -nE "s/^#define[[:space:]]+$1[[:space:]]+([0-9]+)([[:space:]].*)?\$/\1/p" | head -1; return 0; }

live_min="$(num NXDUO_LIVE_MAJOR_MIN "$LIVE_H")"
live_max="$(num NXDUO_LIVE_MAJOR_MAX "$LIVE_H")"
if [ -z "$live_min" ] || [ -z "$live_max" ] || [ "$live_min" -gt "$live_max" ]; then
    echo "the firmware window is unreadable: $LIVE_H needs #define NXDUO_LIVE_MAJOR_MIN <n> and NXDUO_LIVE_MAJOR_MAX <n>, MIN <= MAX"
    exit 1
fi
hos_major="$(num ATMOSPHERE_SUPPORTED_HOS_VERSION_MAJOR "$API_H")"
hos_minor="$(num ATMOSPHERE_SUPPORTED_HOS_VERSION_MINOR "$API_H")"
hos_micro="$(num ATMOSPHERE_SUPPORTED_HOS_VERSION_MICRO "$API_H")"
if [ -z "$hos_major" ]; then
    echo "no ATMOSPHERE_SUPPORTED_HOS_VERSION_MAJOR in $API_H - is $AMS an Atmosphere checkout?"
    exit 1
fi
ams="$(num ATMOSPHERE_RELEASE_VERSION_MAJOR "$API_H").$(num ATMOSPHERE_RELEASE_VERSION_MINOR "$API_H").$(num ATMOSPHERE_RELEASE_VERSION_MICRO "$API_H")"
hos="$hos_major.${hos_minor:-?}.${hos_micro:-?}"

if [ "$live_max" -lt "$hos_major" ]; then
    echo "the firmware window stops at $live_max.x (NXDUO_LIVE_MAJOR_MAX in tier4/applet-mitm/source/nxduo_live_window.h),"
    echo "but Atmosphere $ams ($AMS) supports firmware $hos: on $hos_major.x NX Duo Tap would run and never stream."
    echo "Test capture on $hos_major.x, then raise NXDUO_LIVE_MAJOR_MAX to $hos_major."
    exit 1
fi
echo "firmware window $live_min.x-$live_max.x reaches firmware $hos, the newest Atmosphere $ams supports"
