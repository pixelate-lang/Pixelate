# Events and mouse support

## Button clicks

Buttons support onclick blocks containing property assignments. Call the
window's `update()` method after changing state that should be redrawn.

~~~pxl
button clickMe = button {
    text: "Click me",

    onclick {
        greeting.text = "The button was clicked!";
        my_window.title = "Clicked";
        my_window.update();
    }
}
my_window.add(clickMe);
~~~

## Triggers

A trigger is a named object that must be attached to a window. Condition
triggers run on every tick while their condition is true.

~~~pxl
trigger mouse_trigger = trigger (mouseMove) {
    mouseX.text = f"Mouse X: {mouse.x}";
    mouseY.text = f"Mouse Y: {mouse.y}";
    my_window.update();
}
my_window.add(mouse_trigger);

Keyboard triggers use `onkey("...")`. Named keys include `space`, `esc`, `enter`,
`shift`, `alt`, `up`, `down`, `left`, `right`, and `tab`; one-character names
match case-sensitively. Use `onkeyci` to listen for either case.

~~~pxl
trigger key_trigger = trigger (onkey("a")) {
        status.text = "a was pressed!";
        my_window.update();
}
my_window.add(key_trigger);

trigger insensitive_key_trigger = trigger (onkeyci("w")) {
        status.text = "w or W was pressed!";
        my_window.update();
}
my_window.add(insensitive_key_trigger);
~~~

A condition trigger runs on every tick while its condition is true.

~~~pxl
trigger score_trigger = trigger (count == 10) {
    status.text = "Count reached ten.";
    my_window.update();
}
my_window.add(score_trigger);
~~~

## Mouse position

mouse.x and mouse.y are read-only positions relative to the Pixelate window.

## Cursor shapes

Change the cursor with:

~~~pxl
mouse.shape = HAND;
~~~

Supported shapes:

~~~text
ARROW, IBEAM, WAIT, CROSS, UPARROW, SIZE, ICON,
SIZENWSE, SIZENESW, SIZEWE, SIZENS, SIZEALL,
NO, HAND, APPSTARTING, HELP, PIN, PERSON
~~~

PIN and PERSON depend on cursor resources available on Windows.

## Close triggers

Close behavior is an ordinary special trigger attached to a window. Its body
runs when the user requests that window to close, before the window closes:

~~~pxl
trigger close = trigger(closeTrigger) {
    status.text = "Closing...";
    my_window.update();
}
my_window.add(close);
~~~

