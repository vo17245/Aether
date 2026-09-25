# EditorHost sample game

This is a small game project that consumes installed Aether packages. Configure it with `CMAKE_PREFIX_PATH` pointing to an Aether install and its dependency prefix. `GAME_BUILD_EDITOR=OFF` builds only the runtime executable and Runtime feature libraries.

```sh
cmake -S Templates/EditorHost -B /tmp/editor-host-build \
  -DCMAKE_PREFIX_PATH="/path/to/aether-install;/path/to/dependencies" \
  -DGAME_BUILD_EDITOR=ON
cmake --build /tmp/editor-host-build --parallel
```

`SampleSceneFeature` depends on `PaletteFeature`. The Runtime registration collects both compiled Features, validates their dependency through the project manifest, and registers the Runtime component and asset type contracts. The Editor registration demonstrates separate component schemas and default editors for `example.palette` and `example.scene-settings`.

`TestProject` uses project relative paths and an empty asset catalog. Run the editor from its build directory to use the copied fixture at `TestProject/`. The sample host handles project setup, World creation/open/save, asset import/open/save/rename, edit history, and Play controls. It does not register a viewport provider yet, so the viewport reports that preview is unavailable. Games provide their own render systems and viewport providers through their Feature registrations.
