/* common/nxduo_refusal.h: the sysmodule's firmware refusal line, as the
 * Switch app and the overlay find it in its log. argv[1]: a folder for the
 * test logs (default: the current one). */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include "../nxduo_refusal.h"

/* the sysmodule's formats: LogLine's prefix (applet_mitm_log.cpp) and Main's
 * line (applet_mitm_main.cpp) */
#define PREFIX  "[%6llu.%03llu] "
#define REFUSE  "firmware %u.%u.%u is newer than this build knows (built against Atmosphere %d.%d.%d) - %s (NX Duo Tap %s)"
#define REFUSE2 "firmware %u.%u.%u is newer than this build knows (built against Atmosphere %d.%d.%d) - %s"   /* duo2 */
#define NOT_STARTING "not starting. allow_untested_firmware = 1 in config.ini overrides"
#define ANYWAY  "starting anyway (allow_untested_firmware = 1)"
#define VER     "0.7.22-duo3"

static int fails;
static char dir[512] = ".";

static const char *line_at(unsigned long long ms, unsigned a, unsigned b, unsigned c, const char *what, const char *ver)
{
    static char out[600];
    const int n = snprintf(out, sizeof(out), PREFIX, ms / 1000, ms % 1000);
    if (ver != NULL) snprintf(out + n, sizeof(out) - (size_t)n, REFUSE, a, b, c, 1, 12, 2, what, ver);
    else snprintf(out + n, sizeof(out) - (size_t)n, REFUSE2, a, b, c, 1, 12, 2, what);
    return out;
}

static void expect_line(const char *what, const char *s, const char *ver, int want, unsigned x, unsigned y, unsigned z)
{
    unsigned fw[3] = { 0, 0, 0 };
    const int got = nxduo_refusal_line(s, ver, fw);
    if (got != want || (want && (fw[0] != x || fw[1] != y || fw[2] != z))) {
        printf("FAIL %s: %d (%u.%u.%u), want %d\n  %s\n", what, got, fw[0], fw[1], fw[2], want, s);
        fails++;
    }
}

/* a log file holding exactly len bytes of data (NULs included) */
static void expect_log(const char *what, const char *data, size_t len, int want, unsigned x)
{
    char path[600];
    unsigned fw[3] = { 0, 0, 0 };
    FILE *f;
    int got;
    snprintf(path, sizeof(path), "%s/refusal_test.log", dir);
    f = fopen(path, "wb");
    if (f == NULL || fwrite(data, 1, len, f) != len) { printf("FAIL %s: cannot write %s\n", what, path); fails++; if (f) fclose(f); return; }
    fclose(f);
    got = nxduo_refusal_read(path, VER, fw);
    remove(path);
    if (got != want || (want && fw[0] != x)) { printf("FAIL %s: %d (%u), want %d\n", what, got, fw[0], want); fails++; }
}

static size_t append(char *buf, size_t at, const char *s, size_t cap)
{
    const size_t n = strlen(s);
    if (at + n + 1 < cap) { memcpy(buf + at, s, n); buf[at + n] = '\n'; return at + n + 1; }
    return at;
}

int main(int argc, char **argv)
{
    static char buf[12000];
    char s[700];
    size_t n;
    unsigned fw[3];
    if (argc > 1) snprintf(dir, sizeof(dir), "%s", argv[1]);

    /* the line itself */
    expect_line("duo3 refusal", line_at(21408, 24, 0, 0, NOT_STARTING, VER), VER, 1, 24, 0, 0);
    expect_line("micro version", line_at(31007, 23, 6, 1, NOT_STARTING, VER), VER, 1, 23, 6, 1);
    expect_line("parts at 255", line_at(21408, 255, 255, 255, NOT_STARTING, VER), VER, 1, 255, 255, 255);
    expect_line("started anyway", line_at(21408, 24, 0, 0, ANYWAY, VER), VER, 0, 0, 0, 0);
    expect_line("duo2 line, no version", line_at(21408, 24, 0, 0, NOT_STARTING, NULL), VER, 0, 0, 0, 0);
    expect_line("another build's line", line_at(21408, 24, 0, 0, NOT_STARTING, "0.7.22-duo2"), VER, 0, 0, 0, 0);
    expect_line("a longer version", line_at(21408, 24, 0, 0, NOT_STARTING, VER "1"), VER, 0, 0, 0, 0);
    expect_line("a shorter version", line_at(21408, 24, 0, 0, NOT_STARTING, VER), VER "1", 0, 0, 0, 0);
    expect_line("version without its start", line_at(21408, 24, 0, 0, NOT_STARTING, VER), "duo3", 0, 0, 0, 0);
    expect_line("ten-digit uptime", line_at(9999999999123ull, 24, 0, 0, NOT_STARTING, VER), VER, 1, 24, 0, 0);
    expect_line("eleven-digit uptime", line_at(99999999999123ull, 24, 0, 0, NOT_STARTING, VER), VER, 0, 0, 0, 0);
    expect_line("part over 255", line_at(21408, 256, 0, 0, NOT_STARTING, VER), VER, 0, 0, 0, 0);
    expect_line("four digits", line_at(21408, 1024, 0, 0, NOT_STARTING, VER), VER, 0, 0, 0, 0);
    snprintf(s, sizeof(s), "x%s", line_at(21408, 24, 0, 0, NOT_STARTING, VER));
    expect_line("not at the start", s, VER, 0, 0, 0, 0);
    snprintf(s, sizeof(s), "%s ", line_at(21408, 24, 0, 0, NOT_STARTING, VER));
    expect_line("more after the version", s, VER, 0, 0, 0, 0);
    snprintf(s, sizeof(s), "%s", line_at(21408, 24, 0, 0, NOT_STARTING, VER));
    s[strlen(s) - 1] = ']';
    expect_line("no closing parenthesis", s, VER, 0, 0, 0, 0);
    expect_line("two parts", "[    21.408] firmware 24.0 is newer than this build knows (built against Atmosphere 1.12.2) - not starting (NX Duo Tap " VER ")", VER, 0, 0, 0, 0);
    expect_line("empty part", "[    21.408] firmware 24..0 is newer than this build knows (built against Atmosphere 1.12.2) - not starting (NX Duo Tap " VER ")", VER, 0, 0, 0, 0);
    expect_line("no seconds", "[      .408] firmware 24.0.0 is newer than this build knows (built against Atmosphere 1.12.2) - not starting (NX Duo Tap " VER ")", VER, 0, 0, 0, 0);
    expect_line("two-digit milliseconds", "[    21.40] firmware 24.0.0 is newer than this build knows (built against Atmosphere 1.12.2) - not starting (NX Duo Tap " VER ")", VER, 0, 0, 0, 0);
    expect_line("bad Atmosphere version", "[    21.408] firmware 24.0.0 is newer than this build knows (built against Atmosphere 1.12) - not starting (NX Duo Tap " VER ")", VER, 0, 0, 0, 0);
    expect_line("the live gate's line", "[   212.004]    live: firmware 24.0.0 is untested (this build: 21.x-23.x) - not streaming. allow_untested_firmware = 1 in config.ini overrides.", VER, 0, 0, 0, 0);
    expect_line("empty line", "", VER, 0, 0, 0, 0);

    /* a boot's log: the launcher's lines first, one cut short by LogLine
     * (510 bytes, the last a NUL, then its '\n') */
    n = 0;
    n = append(buf, n, "[    21.402] launcher: Switch Frame Tap launcher, 21.3 s after boot: HOME menu up; system memory 3482 KB free", sizeof(buf));
    n = append(buf, n, "[    21.403] launcher: result=started", sizeof(buf));
    n = append(buf, n, "[    21.403] launcher: Started NX Duo Tap.", sizeof(buf));
    memset(s, 'x', 509);
    memcpy(s, "[    21.404] launcher: others=", 30);
    s[509] = 0;
    memcpy(buf + n, s, 510);
    n += 510;
    buf[n++] = '\n';
    n = append(buf, n, line_at(21408, 24, 0, 0, NOT_STARTING, VER), sizeof(buf));
    n = append(buf, n, "[    21.409] -> main:refused_firmware", sizeof(buf));
    expect_log("a boot's log", buf, n, 1, 24);
    expect_log("the refusal last, without its newline", buf, n - 1 - strlen("[    21.409] -> main:refused_firmware") - 1, 0, 0);

    /* CRLF, as if a PC had saved it */
    n = append(buf, 0, "[    21.403] launcher: result=started\r", sizeof(buf));
    snprintf(s, sizeof(s), "%s\r\n", line_at(21408, 23, 6, 0, NOT_STARTING, VER));
    memcpy(buf + n, s, strlen(s));
    n += strlen(s);
    expect_log("CRLF", buf, n, 1, 23);

    /* only the first 4 KiB count */
    memset(buf, 'y', 4096);
    for (n = 99; n < 4096; n += 100) buf[n] = '\n';
    buf[4095] = '\n';
    n = append(buf, 4096, line_at(21408, 24, 0, 0, NOT_STARTING, VER), sizeof(buf));
    expect_log("after the first 4 KiB", buf, n, 0, 0);
    memset(buf, 'y', 4000);
    buf[3999] = '\n';
    n = append(buf, 4000, line_at(21408, 24, 0, 0, NOT_STARTING, VER), sizeof(buf));
    expect_log("cut by the 4 KiB", buf, n, 0, 0);
    n = append(buf, 0, line_at(21408, 24, 0, 0, NOT_STARTING, "0.7.22-duo2"), sizeof(buf));
    expect_log("another build's log", buf, n, 0, 0);
    expect_log("empty log", buf, 0, 0, 0);
    if (nxduo_refusal_read("refusal_test.no-such-log", VER, fw)) { printf("FAIL no log\n"); fails++; }

    printf("%s (%d failure%s)\n", fails ? "FAILED" : "OK", fails, fails == 1 ? "" : "s");
    return fails != 0;
}
