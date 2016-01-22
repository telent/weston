print('hello from weston-init');

var weston = this;
weston.keymap = {};

function bind_key(mods, keysym, fun) {
    keymap[[keysym]+mods] = fun;
    print("bound");
    print(keymap);
}
print( this);
print(weston);
bind_key(["Ctrl", "Meta"], "7", function beep(e) { print(e); });
bind_key(["Ctrl", "Meta"], "8", function beep(e) { print(e); });

