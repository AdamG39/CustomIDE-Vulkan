# CustomIDE

Note: This project is still early in development therefore mentioned features may not be implemented yet check Features for a list of features and their implementation status.

CustomIDE is a lightweight IDE with heavy emphasis on customisation from colour schemes and layouts to keybinds and more. I wanted this project to have as little dependencies as possible so
the only required libraries are Vulkan, glfw and zlib in future dependencies may have to be added or could be removed (i.e. zlib is only used to parse PNG files. As mentioned the rendering is
handled using Vulkan, window management is handled using glfw to allow for cross-platform support, all other features are written from scratch.

## Features

Below is a feature list as well as if they have been implemented or not:

| Feature                     | Status          |
|-----------------------------|-----------------|
| Basic text editing          | In progress     |
| Layout customisation        | In progress     |
| Colour schemes              | Not implemented |
| Keybind customisation       | Not implemented |
| Text highlighting           | Not implemented |
| Code autofill               | Not implemented |
| Debugging tools             | Not implemented |
| Error checking              | Not implemented |

## Building
Create a build folder in the root directory and navigate inside of it then,

Setup build folder using `cmake` and specifying a generator with `-g` or leaving it to use default:
```
cmake .. -G <GENERATOR_NAME>
```

Then build files with:
```
cmake --build .
```
The outputted executable should now be available to run under the following name:
```
.\CustomIDE.exe
```

## License

Copyright (C) 2026 Adam Graham

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
[GNU General Public License](https://www.gnu.org/licenses/gpl-3.0.html) for more details.

