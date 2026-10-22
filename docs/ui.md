# Windows, Linux, and widgets

## Window options

The `window` constructor supports:

| Property | Description |
| --- | --- |
| title | Window title text. |
| width | Window width in pixels. |
| height | Window height in pixels. |
| iconPath | Path to an .ico file. Relative paths are resolved from the current directory, then beside the executable. Backend support may vary. |
| background | RGB color with channels from 0 to 255. |

Defaults are 800x600, Pixelate App, pixelate.ico, and a white background. Place
the default icon beside `pixelate.exe` when launching Pixelate from `PATH`.

~~~pxl
window my_window = window {
    title: "My App",
    width: 900,
    height: 650,
    iconPath: "pixelate.ico",
    background: 245 247 252,
}
my_window.run();
~~~

## Labels, paragraphs, buttons, textboxes, and textareas

Labels, paragraphs, buttons, textboxes, and textareas support text, x, y, width, height, font, and fontSize.

Font constants include `DEFAULT`, `ARIAL`, `CALIBRI`, `CONSOLAS`, `COURIERNEW`,
`TIMESNEWROMAN`, `TAHOMA`, and `VERDANA`.

The default font size is 16 pixels. Set `fontSize` explicitly to use another size.

`label` is intended for short, single-line text. `par` renders text from the
top-left of its bounds, honors explicit newlines, and wraps long lines.

`textarea` is a multiline editable text input. It accepts Enter for newlines and
shows a vertical scrollbar when the content is taller than the control.

~~~pxl
label greeting = label {
    text: "Hello!",
    x: 50,
    y: 50,
    width: 300,
    height: 40,
}

button clickMe = button {
    text: "Click me",
    x: 50,
    y: 120,
    width: 200,
    height: 50,
}

textbox nameInput = textbox {
    text: "",
    placeholder: "Type here",
    x: 50,
    y: 190,
    width: 300,
    height: 30,
}

textarea notesInput = textarea {
    text: "",
    placeholder: "Write notes...",
    x: 50,
    y: 240,
    width: 400,
    height: 140,
}
my_window.add(greeting);
my_window.add(clickMe);
my_window.add(nameInput);
my_window.add(notesInput);
~~~

Text styling can be set in a widget declaration. Font names are translated by
each platform backend. On Linux, a requested Windows font may resolve to a
system fallback if that font is not installed:

~~~pxl
par body = par {
    text: "This paragraph uses Arial at 18 pixels.",
    font: ARIAL,
    fontSize: 18,
    x: 50,
    y: 400,
    width: 500,
    height: 80,
}
~~~

It can also be changed at runtime with `body.font = CONSOLAS;` or
`body.fontSize = 20;`, followed by `my_window.update();`.

The current font and size can be used in conditions:

~~~pxl
if (body.font == ARIAL) {
    body.text = "This is using Arial.";
}

if (body.fontSize >= 18) {
    body.text = "This is large text.";
}
~~~

Numeric font sizes can be incremented directly, for example
`body.fontSize++;`.

They can also be decremented with `body.fontSize--;`.

Widgets are declared as objects and added to a window with `window.add(widget)`.
Use `window.remove(widget)` to detach one without destroying its declaration;
it can be added again later.

Textboxes and textareas are editable while the program is running. The optional
`placeholder` is shown while the control is empty and unfocused. Read the current
contents with `textboxId.text` or `textareaId.text`, including inside an f-string:

~~~pxl
label result = label {
    text: "Waiting for input...",
    x: 50,
    y: 240,
    width: 400,
    height: 35,
}

button showInput = button {
    text: "Show input",
    x: 50,
    y: 290,
    width: 180,
    height: 45,
    onclick {
        result.text = f"You entered: {nameInput.text}";
    }
}
my_window.add(result);
my_window.add(showInput);
~~~

## Updating properties

A single property does not need update:

~~~pxl
greeting.text = "New text";
greeting.x = 100;
greeting.width = 300;
my_window.title = "New title";
~~~

Change several fields and commit them together with the window's `update()` method:

~~~pxl
greeting.text = "Updated text";
greeting.x = 100;
greeting.y = 120;
greeting.width = 300;
greeting.height = 50;
my_window.update();
~~~
