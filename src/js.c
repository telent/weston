/* Spaghetti Weston: drive your window mnager with javascript

use cases: what do we actually want to use js for?

- picking where to open new windows
- moving windows about to align with other windows
- creating key/touch bindings
- listening for acpi events?  (lid, battery etc)

when do we run js functions?

- evaluate a js script at startup to set defaults
- interactive js eval for side-effects/debugging (open a socket?)
- bind js functions to events
- is there any mileage in react-style diffing between current and desired
   window configuration?

*/

#include <stdlib.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>


#include "duktape.h"
#include "compositor.h"

#include "js.h"

static duk_context *ctx;

/*
when we bind the key we can use

(gdb) p xkb_keysym_from_name("x", 0)
$12 = 120
(gdb) p xkb_keysym_from_name("Escape", 0)
$17 = 65307


to convert a keysym name into a numeric keysym.  But the events that
come off the keyboard have keycodes not keysyms, and the mapping
from one to the other may change dynamically.  What happens if you
switch from one layout to another and your Super-X binding moves?
Presumably if you specified it as Super-X you want it also to
move, whereas if you looked up the keycode and did the binding that
way then you don't

Modifiers in weston are hardcoded: ctrl=1, alt=2, super=4, shift=8

 */


int js_run_key_binding(struct weston_keyboard *keyboard,
                       uint32_t time,
                       uint32_t key,
                       enum weston_keyboard_modifier modifier) {
        // key is a evdev keycode as seen in uevent.h, meaning we have to
        // add 8 to make it an X11 keycode for use with xkb

        int keysym = xkb_state_key_get_one_sym(keyboard->xkb_state.state,
                                               key+8);

        duk_push_global_object(ctx);
        duk_get_prop_string(ctx, -1, "runKeyBinding");
        duk_push_uint(ctx, key);
        duk_push_uint(ctx, keysym);
        duk_push_int(ctx, (int) modifier);
        if(duk_pcall(ctx, 3) != 0) {
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

static int keysym_from_name(duk_context *ctx) {
  const char *name = duk_safe_to_string(ctx, 0);
  duk_push_number(ctx, xkb_keysym_from_name(name, /* flags */ 0));
  return 1;
}



void duk_init(void) {
        ctx = duk_create_heap_default();

        duk_push_global_object(ctx);
        duk_push_c_function(ctx, load_js_file, 1);
        duk_put_prop_string(ctx, -2, "load_file");
        duk_push_c_function(ctx, env_lookup, 1);
        duk_put_prop_string(ctx, -2, "getenv");
        duk_push_c_function(ctx, keysym_from_name, 1);
        duk_put_prop_string(ctx, -2, "keysym_from_name");
        duk_pop(ctx);  /* pop global */

        if (duk_peval_file(ctx, "weston-init.js") != 0) {
                printf("Error: %s\n", duk_safe_to_string(ctx, -1));
        }
        duk_pop(ctx);  /* ignore result */
}

static int
open_repl_unix_socket(void)
{
        /* this is a straight copy of bind_to_unix_socket in
         * xwayland/launcher.c */

        struct sockaddr_un addr;
        socklen_t size, name_size;
        int fd;
        char *path = "/tmp/weston-js.sock";

        fd = socket(PF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0);
        if (fd < 0)
                return -1;

        addr.sun_family = AF_LOCAL;
        name_size = snprintf(addr.sun_path, sizeof addr.sun_path, path) + 1;
        size = offsetof(struct sockaddr_un, sun_path) + name_size;
        unlink(addr.sun_path);
        if (bind(fd, (struct sockaddr *) &addr, size) < 0) {
                weston_log("failed to bind to %s: %m\n", addr.sun_path);
                close(fd);
                return -1;
        }
        if (listen(fd, 1) < 0) {
                unlink(addr.sun_path);
                close(fd);
                return -1;
        }

        return fd;
}

struct sock_callback_data {
        struct wl_event_loop *loop;
        struct wl_event_source *source;
        int fd;
};

static int process_repl_connected_socket(int fd, uint32_t mask, void *data) {
        char buf[1000];
        int bytes=1;
        struct sock_callback_data *cb_data = (struct sock_callback_data *) data;
        bytes = read(fd, buf, (sizeof buf));
        if(bytes>0) {
                fprintf(stderr, "<< %.*s", bytes, buf);
                return 1;
        } else {
                /* when connection closed, need to remove this fd from
                   event loop */
                wl_event_source_remove(cb_data->source);
                close(fd);
                free(cb_data);
                return 0;
        }
}

static int process_repl_server_socket(int fd, uint32_t mask, void *data) {
        struct sockaddr_un peer;
        socklen_t socksize = sizeof (struct sockaddr_un);
        struct wl_event_loop *loop = (struct wl_event_loop *) data;

        int connected_sock = accept(fd, (struct sockaddr *) &peer, &socksize);
        struct sock_callback_data *cb_data=0;


        if(connected_sock > -1) {
                cb_data = (struct sock_callback_data *)
                        calloc(sizeof (struct sock_callback_data), 1);
                cb_data->fd = connected_sock;
                cb_data->loop = loop;
                cb_data->source =
                        wl_event_loop_add_fd(loop, connected_sock,
                                             WL_EVENT_READABLE,
                                             process_repl_connected_socket,
                                             (void *)cb_data);
        }
        return 1;
}



void duk_add_repl_socket(struct wl_event_loop *loop) {
        int sock_fd = open_repl_unix_socket();
        if(sock_fd>-1) {
                wl_event_loop_add_fd(loop, sock_fd,
                                     WL_EVENT_READABLE,
                                     process_repl_server_socket,
                                     (void *)loop);
        }
}
