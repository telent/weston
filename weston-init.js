print('hello from weston-init');
// download immutable.js from somewhere and point this environment var at it
load_file(getenv("IMMUTABLE_JS"));

var weston = Function('return this')();
weston.keymap = Immutable.Map({});

KeySpec = Immutable.Record({modifiers: 0, keysym: null});

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
    return new KeySpec({modifiers: mods, keysym: keysym});
}

function bind_key(keyname, fun) {
    weston.keymap = weston.keymap.set(convert_keyname(keyname), fun);
}

weston.runKeyBinding = function find_key_binding(keycode, keysym, modifiers) {
    print("lookup ",keycode, " ", keysym, " ", modifiers);
    var ks = new KeySpec({keysym: keysym, modifiers: modifiers})
    var handler = weston.keymap.get(ks);
    if(handler){
        print("got", handler);
    }
    return false;
}

bind_key(["Ctrl", "alt", "7"], function beep(e) { print(e); });
bind_key(["Ctrl", "alt", "8"], function burp(e) { print(e); });
bind_key(["Ctrl", "alt", "0"], function barf(e) { print(e); });
bind_key(["Ctrl", "alt", "q"], function barf(e) { print(e); });
print(weston.keymap);
