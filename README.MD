# STM32H745I-DISCO PROJECT TEMPLATE

*(c)2023 by CodeDog Sp. z o.o.*

This project is a pre-configured STM32H745I-DISCO board firmware
with some custom drivers, tools and code scaffolding added.

Intended use: testing STM32H745I-DISCO boards, starting new
STM32H745I TouchGFX based projects.

## Credits / Legal

Majority of the code files belong to the projects by STMicroelectronics.

Middlewares and drivers belong to third-party organizations.

Appropriate licenses apply.

Portions marked in sources as copyrighted by CodeDog are written
by Adam Łyskawa of CodeDog.

This project is a (working) code sample. It is not an official release.

## Disclaimer

The current version lacks the separate documentation for the added features.

Added features are property of CodeDog company and can be used with
or without modifications in your code under the MIT license.

CodeDog Sp. z o.o. doesn't guarantee this code would work in your project,
the code is made for internal use.

## Custom add-ons

### Directories

- `Drivers/Custom/`
- `Process/`
- `Tools/`

## Configuration

The project has configured internal `eMMC` memory, `USB_OTG_FS`,
`USART3` as debug port, `USART1` as serial printer port.

See `User Constants` in `.ioc` file.

It also configures TIM7 as microtime timer (not used in this template)
and TIM17 as delay time (used by `delay_async` and `repeat_async` functions
of `tim_ex.h`).

The original template is modified by setting LCD backlight power
enable pin to LOW on pre-os initialization, and setting it back high
on main app process start.

`main_user.c` also contains `clearLTDCRam` function that clears
the frame buffer used by the LCD screen to remove random artifacts
at startup.

Both `eMMC` and `USB` file systems are initialized on startup.
When a USB disk is inserted, the file system present on its first
partition is mounted and `App::fsAvailableChanged` function is called.

Check the `Tools/FS.hpp` source for inline documentation of how
to read and write files using this tool.

(Using `FS` tool is optional, `FATFS` functions can be used directly.)

The file systems can be accessed in RAII-friendly way using the
`FS` utility present in `Tools` directory.

The `FATFS` middleware is configured to support ExFAT file system
and long file names up to 255 characters.

## Startup

When the application run, the `App::init` function is call from
`App_Task` function declared in code generated in the device
configuration (`.ioc`) file.

The demo initializes the additional timers, audio chip, tries
to mount the file systems, turns the LCD backlight power on
and plays a beep sound through the headphone output.

## GUI

The application has a basic testing GUI consisting of 2 screens
and a button on each screen to navigate between screens.
It allows testing if the TouchGFX module works properly.

## Toolchain / environment

This projects requires `TouchGFX Designer` in version `4.22.0`
or later, and `STM32CubeIDE` in version `1.13.0` or later.

Optionally, `Visual Studio Code` can be used for working with the code files.

Microsoft `C++` extension is required, also a ARM toolchain that can
be downloaded at:

https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads

It needs to be extracted to a local directory and the path should be
specified in line `3` of `.vscode/c_cpp_properties.json` file.

## Project name customization

To change the project name in any TouchGFX project, the project's name
must be placed in several places, be sure to cover all of them:

- root directory name
- `.ioc` file name
- `CM7/TouchGFX/*.touchgfx.part` content:
  - `Application/Name` property value
  - `Application/ProjectFile` property value (with `.ioc` extension)
- `*.touchgfx` file name
- `*.touchgfx` file content (same properties as in `*.touchgfx.part` file)
- `STM32CubeIDE/.project` content:
  - `projectDescription/name` property value
  - `linkedResources` section, `link` item, name of `.ioc` file

The changes should be applied wheb the project is not open in neither
`TouchGFX Designer` nor `STM32CubeIDE`.

The project should be imported to a clean `STM32CubeIDE` workspace again after its name was changed
to avoid errors.

### CAUTION

DO NOT USE *RENAME* OPTION IN EITHER TOOLS TO RENAME THE PROJECT!

DOING SO WOULD BREAK THE PROJECT BEYOND REPAIR.

The only way to rename the project is following the instructions above.

Keep all changes commited before renaming the project.
