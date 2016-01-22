print('hello from weston-init');
// download immutable.js from somewhere and point this environment var at it
load_file(getenv("IMMUTABLE_JS"));

var weston = Function('return this')();
weston.keymap = Immutable.Map({"fgr":"dfhsgfh!"});

function bind_key(mods, keysym, fun) {
    weston.keymap = weston.keymap.set([keysym]+mods, fun);
}

bind_key(["Ctrl", "Meta"], "7", function beep(e) { print(e); });
bind_key(["Ctrl", "Meta"], "8", function burp(e) { print(e); });
print(weston.keymap);

weston.runKeyBinding = function find_key_binding(key, modifiers) {
    print("lookup ",key, " ", modifiers);
    return false;
}
