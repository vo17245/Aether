## Usage

## implement stb

in ONLY ONE .c/.cpp file
```cpp
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"
```

## enable assert

define macro AETHER_ENABLE_ASSERT to enable assert

AETHER_ASSERT usage example
```cpp
AETHER_ASSERT(false&&"some wrongs happen here");
```