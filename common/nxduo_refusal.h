/*
 * NX Paradox: the line the sysmodule logs when it does not start because the
 * firmware is newer than the Atmosphere it was built against knows
 * (tier4/applet-mitm/source/applet_mitm_main.cpp, Main) - for the Switch app
 * and the overlay, which then say so instead of "Starting soon" for good.
 * C and C++.
 *
 *   [    21.408] firmware 24.0.0 is newer than this build knows (built against
 *   Atmosphere 1.12.2) - not starting. allow_untested_firmware = 1 in
 *   config.ini overrides (NX Duo Tap 0.7.22-duo3)                 (one line)
 *
 * It counts only when it ends with the reader's own version: a log left by
 * another build (replaced without a restart, or not started since) does not.
 * Each start of the module begins a new log, with this line after the
 * launcher's few, well inside the first 4 KiB read here. Plain stdio on
 * sdmc:, which the caller must have mounted (the overlay: doWithSDCardHandle).
 */
#ifndef NXDUO_REFUSAL_H
#define NXDUO_REFUSAL_H

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

/* a release install's log; with a test arm file, the test log (applet_mitm_log.cpp) */
#define NXDUO_REFUSAL_LOG      "sdmc:/config/switch-frame-tap/log.txt"
#define NXDUO_REFUSAL_TEST_LOG "sdmc:/applet-mitm.log"
#define NXDUO_REFUSAL_ARM_FILE "sdmc:/applet-mitm.armed"

/* "X.Y.Z" at *s, each part 1-3 digits and at most 255; *s moves past it */
static inline int nxduo_refusal_version(const char **s, unsigned v[3])
{
    for (int i = 0; i < 3; i++) {
        int n = 0;
        if (i > 0 && *(*s)++ != '.') return 0;
        for (v[i] = 0; n < 3 && **s >= '0' && **s <= '9'; n++) v[i] = v[i] * 10 + (unsigned)(*(*s)++ - '0');
        if (n == 0 || (**s >= '0' && **s <= '9') || v[i] > 255) return 0;
    }
    return 1;
}

/* one line of the log, without its '\n': 1 and the firmware in fw when it is
 * that refusal, written by version ver (fw means nothing after a 0) */
static inline int nxduo_refusal_line(const char *s, const char *ver, unsigned fw[3])
{
    static const char mid[] = " is newer than this build knows (built against Atmosphere ";
    static const char end[] = ") - not starting";
    static const char tag[] = " (NX Duo Tap ";
    const size_t vl = strlen(ver), tl = sizeof(tag) - 1;
    unsigned ams[3];
    size_t n;
    /* "[%6llu.%03llu] ": the seconds since boot */
    if (*s++ != '[') return 0;
    while (*s == ' ') s++;
    for (n = 0; *s >= '0' && *s <= '9'; n++) s++;
    if (n == 0 || n > 10 || *s++ != '.') return 0;
    for (n = 0; n < 3; n++, s++) if (*s < '0' || *s > '9') return 0;
    if (strncmp(s, "] firmware ", 11) != 0) return 0;
    s += 11;
    if (!nxduo_refusal_version(&s, fw) || strncmp(s, mid, sizeof(mid) - 1) != 0) return 0;
    s += sizeof(mid) - 1;
    if (!nxduo_refusal_version(&s, ams) || strncmp(s, end, sizeof(end) - 1) != 0) return 0;
    s += sizeof(end) - 1;
    /* ... and it ends with " (NX Duo Tap <ver>)" */
    n = strlen(s);
    if (n < tl + vl + 1) return 0;
    s += n - (tl + vl + 1);
    return strncmp(s, tag, tl) == 0 && strncmp(s + tl, ver, vl) == 0 && s[tl + vl] == ')';
}

/* the first 4 KiB of the log at path: 1 and the firmware in fw when they hold
 * that refusal by version ver; 0 for no log, no such line or anything odd. A
 * line the module cut short has a NUL before its '\n', which only ends it
 * early; a line cut by the 4 KiB is not read. */
static inline int nxduo_refusal_read(const char *path, const char *ver, unsigned fw[3])
{
    static char b[4096 + 1];
    size_t n, i = 0;
    FILE *f = fopen(path, "rb");
    if (f == NULL) return 0;
    n = fread(b, 1, sizeof(b) - 1, f);
    fclose(f);
    b[n] = 0;
    while (i < n) {
        char *nl = (char *)memchr(b + i, '\n', n - i);
        if (nl == NULL) break;
        *nl = 0;
        if (nl > b + i && nl[-1] == '\r') nl[-1] = 0;
        if (nxduo_refusal_line(b + i, ver, fw)) return 1;
        i = (size_t)(nl - b) + 1;
    }
    return 0;
}

/* the same, in the log this install writes */
static inline int nxduo_refusal_find(const char *ver, unsigned fw[3])
{
    struct stat st;
    return nxduo_refusal_read(stat(NXDUO_REFUSAL_ARM_FILE, &st) == 0 ? NXDUO_REFUSAL_TEST_LOG : NXDUO_REFUSAL_LOG, ver, fw);
}

#ifdef __SWITCH__
#include <switch.h>
/* the console's language is Spanish (Spain or Latin America) - the "set"
 * service, open just for this; sm must be up (the overlay: doWithSmSession) */
static inline int nxduo_system_spanish(void)
{
    u64 code = 0;
    SetLanguage lang = SetLanguage_ENUS;
    int es = 0;
    if (R_FAILED(setInitialize())) return 0;
    if (R_SUCCEEDED(setGetSystemLanguage(&code)) && R_SUCCEEDED(setMakeLanguage(code, &lang)))
        es = lang == SetLanguage_ES || lang == SetLanguage_ES419;
    setExit();
    return es;
}
#endif

#endif
