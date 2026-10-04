file(READ "${INPUT}" _bytes HEX)
string(REGEX REPLACE "(..)" "0x\\1," _bytes "${_bytes}")
file(WRITE "${OUTPUT}" "#pragma once\nnamespace Aether::Script::Detail { inline constexpr unsigned char BridgeAssembly[] = {${_bytes}}; }\n")
