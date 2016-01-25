print('hello from weston-init');
// download immutable.js from somewhere and point this environment var at it
load_file(getenv("IMMUTABLE_JS"));

var weston = Function('return this')();
weston.keymap = Immutable.Map({"fgr":"dfhsgfh!"});

function convert_keyname(keyname) {
    keyname = Immutable.List(keyname);
    var keysym = keysym_from_name(keyname.last());
    var mods = keyname.pop().reduce(function (mods,n) {
        return mods + ({
            'shift': 8,
            'ctrl': 1,
            'alt': 2,
            'super': 4
        }[n.toLowerCase()])
    }, 0);
    return [mods, keysym];
}

function bind_key(keynames, fun) {
    // weston.keymap = weston.keymap.set(keynames, fun);
}

bind_key(["Ctrl", "Meta", "7"], function beep(e) { print(e); });
bind_key(["Ctrl", "Meta", "8"], function burp(e) { print(e); });
bind_key(["Ctrl", "Meta", "F1"], function barf(e) { print(e); });
print(weston.keymap);

print(convert_keyname(["Ctrl","Alt","F1"]));
print(convert_keyname(["Ctrl","Alt","F1"]));
print(convert_keyname(["Super","Left"]));
print(convert_keyname(["Shift","8"]));


weston.runKeyBinding = function find_key_binding(keycode, keysym, modifiers) {
    print("lookup ",keycode, " ", keysym, " ", modifiers);
    return false;
}
