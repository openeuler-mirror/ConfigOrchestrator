#ifndef UITOOLS_H
#define UITOOLS_H

#include "YTypes.h"
#include <string>

using std::string;

namespace dialog_meta {
const static string INFO = "INFO";
const static string WARNING = "WARNING";
const static string ERROR = "ERROR";
const static string HELP = "HELP";

static constexpr YLayoutSize_t kPopDialogMinWidth = 30;
static constexpr YLayoutSize_t kPopDialogMinHeight = 6;
} // namespace dialog_meta

auto showDialog(const string &title, const string &msg) -> void;

#endif
