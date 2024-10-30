#include "tools/sys.h"

auto isSuperUser() -> bool { return getuid() == 0; }
