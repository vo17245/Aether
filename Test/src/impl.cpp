#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
#include "Aether/Core/Config.h"
namespace Aether
{
    Config CreateConfig()
    {
        Config config;
        config.resource_path="/path/to/resource_dir";
        config.log_name="log_name";
        config.app_name="app_name";
        return config;
    }
    Config& GetConfig()
    {
        static Config config=CreateConfig();
        return config;
    }
}