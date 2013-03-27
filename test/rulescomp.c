/*
 * Copyright © 2009 Dan Nicholson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <linux/input.h>
#include <time.h>

#include "test.h"

#define BENCHMARK_ITERATIONS 1000

static int
test_rmlvo_va(struct xkb_context *context, const char *rules,
              const char *model, const char *layout,
              const char *variant, const char *options, va_list ap)
{
    struct xkb_keymap *keymap;
    int ret;

    keymap = test_compile_rules(context, rules, model, layout, variant,
                                options);
    if (!keymap)
        return 0;

    fprintf(stderr, "Compiled '%s' '%s' '%s' '%s' '%s'\n",
            strnull(rules), strnull(model), strnull(layout),
            strnull(variant), strnull(options));

    ret = test_key_seq_va(keymap, ap);

    xkb_keymap_unref(keymap);

    return ret;
}

static int
test_rmlvo(struct xkb_context *context, const char *rules,
           const char *model, const char *layout, const char *variant,
           const char *options, ...)
{
    va_list ap;
    int ret;

    va_start(ap, options);
    ret = test_rmlvo_va(context, rules, model, layout, variant, options, ap);
    va_end(ap);

    return ret;
}

static int
test_rmlvo_silent(struct xkb_context *context, const char *rules,
                  const char *model, const char *layout,
                  const char *variant, const char *options)
{
    struct xkb_keymap *keymap;

    keymap = test_compile_rules(context, rules, model, layout, variant,
                                options);
    if (keymap)
        xkb_keymap_unref(keymap);

    return keymap != NULL;
}

static int
test_rmlvo_env(struct xkb_context *ctx, const char *rules, const char *model,
               const char *layout, const char *variant, const char *options,
               ...)
{
    va_list ap;
    int ret;

    va_start (ap, options);

    if (!isempty(rules))
        setenv("XKB_DEFAULT_RULES", rules, 1);
    else
        unsetenv("XKB_DEFAULT_RULES");

    if (!isempty(model))
        setenv("XKB_DEFAULT_MODEL", model, 1);
    else
        unsetenv("XKB_DEFAULT_MODEL");

    if (!isempty(layout))
        setenv("XKB_DEFAULT_LAYOUT", layout, 1);
    else
        unsetenv("XKB_DEFAULT_LAYOUT");

    if (!isempty(variant))
        setenv("XKB_DEFAULT_VARIANT", variant, 1);
    else
        unsetenv("XKB_DEFAULT_VARIANT");

    if (!isempty(options))
        setenv("XKB_DEFAULT_OPTIONS", options, 1);
    else
        unsetenv("XKB_DEFAULT_OPTIONS");

    ret = test_rmlvo_va(ctx, NULL, NULL, NULL, NULL, NULL, ap);

    va_end(ap);

    return ret;
}

static void
benchmark(struct xkb_context *context)
{
    struct timespec start, stop, elapsed;
    enum xkb_log_level old_level = xkb_context_get_log_level(context);
    int old_verb = xkb_context_get_log_verbosity(context);
    int i;

    xkb_context_set_log_level(context, XKB_LOG_LEVEL_CRITICAL);
    xkb_context_set_log_verbosity(context, 0);

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (i = 0; i < BENCHMARK_ITERATIONS; i++)
        assert(test_rmlvo_silent(context, "evdev", "evdev", "us", "", ""));
    clock_gettime(CLOCK_MONOTONIC, &stop);

    xkb_context_set_log_level(context, old_level);
    xkb_context_set_log_verbosity(context, old_verb);

    elapsed.tv_sec = stop.tv_sec - start.tv_sec;
    elapsed.tv_nsec = stop.tv_nsec - start.tv_nsec;
    if (elapsed.tv_nsec < 0) {
        elapsed.tv_nsec += 1000000000;
        elapsed.tv_sec--;
    }

    fprintf(stderr, "compiled %d keymaps in %ld.%09lds\n",
            BENCHMARK_ITERATIONS, elapsed.tv_sec, elapsed.tv_nsec);
}

int
main(int argc, char *argv[])
{
    struct xkb_context *ctx = test_get_context(CONTEXT_ALLOW_ENVIRONMENT_NAMES);

    assert(ctx);

    if (argc > 1 && streq(argv[1], "bench")) {
        benchmark(ctx);
        return 0;
    }

#define KS(name) xkb_keysym_from_name(name, 0)

    assert(test_rmlvo(ctx, "evdev", "pc105", "us,il,ru,ca", ",,,multix", "grp:alts_toggle,ctrl:nocaps,compose:rwin",
                      KEY_Q,          BOTH, XKB_KEY_q,                    NEXT,
                      KEY_LEFTALT,    DOWN, XKB_KEY_Alt_L,                NEXT,
                      KEY_RIGHTALT,   DOWN, XKB_KEY_ISO_Next_Group,       NEXT,
                      KEY_RIGHTALT,   UP,   XKB_KEY_ISO_Level3_Shift,     NEXT,
                      KEY_LEFTALT,    UP,   XKB_KEY_Alt_L,                NEXT,
                      KEY_Q,          BOTH, XKB_KEY_slash,                NEXT,
                      KEY_LEFTSHIFT,  DOWN, XKB_KEY_Shift_L,              NEXT,
                      KEY_Q,          BOTH, XKB_KEY_Q,                    NEXT,
                      KEY_RIGHTMETA,  BOTH, XKB_KEY_Multi_key,            FINISH));
    assert(test_rmlvo(ctx, "evdev",  "pc105", "us,in", "", "grp:alts_toggle",
                      KEY_A,          BOTH, XKB_KEY_a,                    NEXT,
                      KEY_LEFTALT,    DOWN, XKB_KEY_Alt_L,                NEXT,
                      KEY_RIGHTALT,   DOWN, XKB_KEY_ISO_Next_Group,       NEXT,
                      KEY_RIGHTALT,   UP,   XKB_KEY_ISO_Level3_Shift,     NEXT,
                      KEY_LEFTALT,    UP,   XKB_KEY_Alt_L,                NEXT,
                      KEY_A,          BOTH, KS("U094b"),                  FINISH));
    assert(test_rmlvo(ctx, "evdev", "pc105", "us", "intl", "",
                      KEY_GRAVE,      BOTH,  XKB_KEY_dead_grave,          FINISH));
    assert(test_rmlvo(ctx, "evdev", "evdev", "us", "intl", "grp:alts_toggle",
                      KEY_GRAVE,      BOTH,  XKB_KEY_dead_grave,          FINISH));

    /* 20 is not a legal group; make sure this is handled gracefully. */
    assert(test_rmlvo(ctx, "evdev", "", "us:20", "", "",
                      KEY_A,          BOTH, XKB_KEY_a,                    FINISH));

    /* Don't choke on missing values in RMLVO. Should just skip them.
       Currently generates us,us,ca. */
    assert(test_rmlvo(ctx, "evdev", "", "us,,ca", "", "grp:alts_toggle",
                      KEY_A,          BOTH, XKB_KEY_a,                    NEXT,
                      KEY_LEFTALT,    DOWN, XKB_KEY_Alt_L,                NEXT,
                      KEY_RIGHTALT,   DOWN, XKB_KEY_ISO_Next_Group,       NEXT,
                      KEY_RIGHTALT,   UP,   XKB_KEY_ISO_Next_Group,       NEXT,
                      KEY_LEFTALT,    UP,   XKB_KEY_Alt_L,                NEXT,
                      KEY_LEFTALT,    DOWN, XKB_KEY_Alt_L,                NEXT,
                      KEY_RIGHTALT,   DOWN, XKB_KEY_ISO_Next_Group,       NEXT,
                      KEY_RIGHTALT,   UP,   XKB_KEY_ISO_Level3_Shift,     NEXT,
                      KEY_LEFTALT,    UP,   XKB_KEY_Alt_L,                NEXT,
                      KEY_APOSTROPHE, BOTH, XKB_KEY_dead_grave,           FINISH));

    assert(test_rmlvo(ctx, "", "", "", "", "",
                      KEY_A,          BOTH, XKB_KEY_a,                    FINISH));

    assert(!test_rmlvo(ctx, "does-not-exist", "", "", "", "",
                       KEY_A,          BOTH, XKB_KEY_a,                   FINISH));

    assert(test_rmlvo_env(ctx, "evdev", "", "us", "", "",
                          KEY_A,          BOTH, XKB_KEY_a,                FINISH));
    assert(test_rmlvo_env(ctx, "evdev", "", "us", "", "ctrl:nocaps",
                          KEY_CAPSLOCK,   BOTH, XKB_KEY_Control_L,        FINISH));

    /* Ignores multix and generates us,ca. */
    assert(test_rmlvo_env(ctx, "evdev", "", "us,ca", ",,,multix", "grp:alts_toggle",
                          KEY_A,          BOTH, XKB_KEY_a,                NEXT,
                          KEY_LEFTALT,    DOWN, XKB_KEY_Alt_L,            NEXT,
                          KEY_RIGHTALT,   DOWN, XKB_KEY_ISO_Next_Group,   NEXT,
                          KEY_RIGHTALT,   UP,   XKB_KEY_ISO_Level3_Shift, NEXT,
                          KEY_LEFTALT,    UP,   XKB_KEY_Alt_L,            NEXT,
                          KEY_GRAVE,      UP,   XKB_KEY_numbersign,       FINISH));

    assert(!test_rmlvo_env(ctx, "broken", "what-on-earth", "invalid", "", "",
                           KEY_A,          BOTH, XKB_KEY_a,               FINISH));

    xkb_context_unref(ctx);

    ctx = test_get_context(0);
    assert(test_rmlvo_env(ctx, "broken", "but", "ignored", "per", "ctx flags",
                          KEY_A,          BOTH, XKB_KEY_a,                FINISH));

    /* Test response to invalid flags. */
    {
        struct xkb_rule_names rmlvo = { NULL };
        assert(!xkb_keymap_new_from_names(ctx, &rmlvo, -1));
        assert(!xkb_keymap_new_from_names(ctx, &rmlvo, 5453));
    }

    xkb_context_unref(ctx);
}
