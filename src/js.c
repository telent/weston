/* JS support in maxwell

use cases: what do we actually want to use js for?

- picking where to open new windows
- moving windows about to align with other windows
- creating key/touch bindings
- listening for acpi events?  (lid, battery etc)

when do we run js functions?

- evaluate a js script at startup to set defaults
- interactive js eval for side-effects/debugging (open a socket?)
- bind js functions to events

*/

#include <stdlib.h>

#include "duktape.h"
#include "compositor.h"

#include "js.h"

static duk_context *ctx;

int js_run_key_binding(struct weston_keyboard *keyboard,
                       uint32_t time,
                       uint32_t key,
                       enum weston_keyboard_modifier modifier) {
        duk_push_global_object(ctx);
        duk_get_prop_string(ctx, -1, "runKeyBinding");
        duk_push_uint(ctx, key);
        duk_push_int(ctx, (int) modifier);
        if(duk_pcall(ctx, 2) != 0) {
          fprintf(stderr, "error: %s", duk_safe_to_string(ctx, -1));
        }
        return 0;
}

static int load_js_file(duk_context *ctx) {
        const char *name = duk_safe_to_string(ctx, 0);
        fprintf(stderr, ";; loading %s\n", name);
        if (duk_peval_file(ctx, name) != 0) {
                printf("Error: %s\n", duk_safe_to_string(ctx, -1));
        }
        duk_pop(ctx);  /* ignore result */

        return 0;  /* undefined */
}

static int env_lookup(duk_context *ctx) {
        const char *name = duk_safe_to_string(ctx, 0);
        const char *val = getenv(name);
        duk_push_string(ctx, val);

        return 1;
}


void duk_init(void) {
        ctx = duk_create_heap_default();

        duk_push_global_object(ctx);
        duk_push_c_function(ctx, load_js_file, 1);
        duk_put_prop_string(ctx, -2, "load_file");
        duk_push_c_function(ctx, env_lookup, 1);
        duk_put_prop_string(ctx, -2, "getenv");
        duk_pop(ctx);  /* pop global */

        if (duk_peval_file(ctx, "weston-init.js") != 0) {
                printf("Error: %s\n", duk_safe_to_string(ctx, -1));
        }
        duk_pop(ctx);  /* ignore result */
}
