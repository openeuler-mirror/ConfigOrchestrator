#include "backend/package_manager/package_manager_backend.h"

#include <iostream>

/* avoid using C++20 keywords */
#define LIBSOLV_SOLVABLE_PREPEND_DEP 1
#include "libdnf/libdnf.h"

PackageManagerBackend::PackageManagerBackend() {
  DnfRepo repo;
  std::cout << "PackageManagerBackend::PackageManagerBackend()" << std::endl;
}
