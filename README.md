# CustomIDE

## Debug/Testing

### Building
Create a build folder in the root directory and navigate inside of it then,

Setup build folder using `Ninja` as the generator:
```
cmake .. -G Ninja
```
Optional parameters can be passed to this step:
| Command                             | Effect                                                                                 |
|-------------------------------------|----------------------------------------------------------------------------------------|
| `-DCMAKE_EXPORT_COMPILE_COMMANDS=1` | Creates a `compile_commands.json` file which clangd requires to work                   |
| `-DCMAKE_BUILD_TYPE=Type`           | `Type` can be `Release` or `Debug`, which specifies what build mode `Ninja` should use |

Then build files with:
```
cmake --build .
```
The outputted executable should now be fine to run either by double clicking the exectuable or via terminal with:
```
.\CustomIDE.exe
```
