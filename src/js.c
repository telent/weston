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


#include "duktape.h"

static int bind_key_event(duk_context *ctx) {
	int i;
	// int n = duk_get_top(ctx);  /* #args */
#include "js.h"

        const char *keysym = duk_safe_to_string(ctx, 0);
        const char *defn = duk_safe_to_string(ctx, 1);
        fprintf(stderr, "bound %s to %s\n", keysym, defn);
	duk_push_number(ctx, 42);
	return 1;  /* one return value */
static duk_context *ctx;

}


void duk_init(void) {
        ctx = duk_create_heap_default();
        duk_eval_string(ctx, "print('Hello world!');");
        
        duk_push_global_object(ctx);
        duk_push_c_function(ctx, bind_key_event, DUK_VARARGS);
        duk_put_prop_string(ctx, -2, "bind_key");
        duk_pop(ctx);  /* pop global */
        
        if (duk_peval_file(ctx, "weston-init.js") != 0) {
                printf("Error: %s\n", duk_safe_to_string(ctx, -1));
        }
        duk_pop(ctx);  /* ignore result */
}

