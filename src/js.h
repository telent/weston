void duk_init(void);
int js_run_key_binding(struct weston_keyboard *keyboard,
                       uint32_t time,
                       uint32_t key,
                       enum weston_keyboard_modifier modifier);
