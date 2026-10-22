# Getting started with Pixelate

Pixelate is an early-stage UI library and interpreted language for creating graphical applications with `.pxl` files. Windows remains the original
platform; Linux support is experimental and behavior may still change.

## Requirements

- A cloned copy of this repository
- A C++17 compiler and CMake when building from source
- VS Code, if you want syntax highlighting and the run button

### Windows

The Windows backend uses Win32 and GDI. The interpreter for windows is available at [the v1 release](https://github.com/adilevy1011/Pixelate/releases/tag/v0.1.0). You can download the `.exe` and run your .pxl files.

### Linux (experimental)

Install the build and GTK development dependencies:

~~~bash
sudo apt update
sudo apt install build-essential cmake pkg-config libgtk-3-dev libcairo2-dev
~~~

The Linux backend uses GTK 3, Cairo, and Pango. Font appearance depends on
the fonts installed on the system; Windows font names may use Linux fallbacks.

#### Build from source

From the repository root:

~~~bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
~~~

## Run your first program

On Windows, open PowerShell in the repository folder and run:

~~~powershell
.\bin\pixelate.exe programs\demos2\01_example.pxl
~~~
or add pixelate.exe to your path and run it simply with:
~~~powershell
pixelate yourProgram.pxl
~~~
On Linux, run the CMake-built executable:

~~~bash
./build/pixelate programs/demos2/01_example.pxl
~~~

You can run the other examples with the same platform-specific command.

## Create a program

Create a file such as `programs/my_app.pxl`:

~~~pxl
window my_window = window {
    title: "My First Pixelate App",
    width: 800,
    height: 600,
    background: 240 245 250,
}

label greeting = label {
        text: "Hello from Pixelate!",
        x: 50,
        y: 50,
        width: 300,
        height: 40,
}

my_window.add(greeting);
my_window.run();
~~~

Run it with the appropriate command above. For Linux:

~~~bash
./build/pixelate programs/my_app.pxl
~~~

## VS Code extension

A VSCode extension for Pixelate is currently also under development in parallel to Pixelate. You can find it [here](https://github.com/adilevy1011/Pixelate_vscode).

## Documentation

- [Language reference](language.md)
- [Windows, Linux, and widgets](ui.md)
- [Events and mouse support](events.md)
- [VS Code extension](https://github.com/adilevy1011/Pixelate_vscode)
- [Architecture and platform backends](architecture.md)

## Project examples

- `programs/demos2/01_example.pxl` demonstrates declarations, object
  composition with `add`, and explicit window startup with `run`.
- `programs/demos2/02_add_remove.pxl` demonstrates removing and re-adding
  the same widget and trigger objects without destroying their declarations.

Pixelate is under early development. Syntax, APIs, rendering, and platform
behavior may change.
