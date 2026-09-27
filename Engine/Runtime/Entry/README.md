# Entry

`DEFINE_APPLICATION` keeps the application factory unchanged. The engine entry point forwards command line arguments to `Application::OnCommandLineArguments(std::span<const std::string_view>)` before `GetInitParams` and before initializing threads, audio, windows, or rendering.

The argument span excludes the executable name and preserves shell-provided argument boundaries, including paths containing spaces. The argument views and span remain valid through application shutdown. Return `true` to continue startup or `false` to exit with status 1 without initializing engine resources. Existing applications inherit a callback that returns `true`.

```cpp
bool OnCommandLineArguments(std::span<const std::string_view> arguments) override
{
    // Parse application-specific options here and retain them for OnInit.
    return true;
}
```

Rebuild and install the Entry package, then rebuild consumers after changing `Application`, because its virtual interface has changed.
