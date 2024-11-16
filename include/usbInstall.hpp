#pragma once
#include <string>
#include <vector>

namespace usbInstStuff {
std::vector<std::string> OnSelected();
void installTitleUsb(std::vector<std::string> ourNspList, int ourStorage);
} // namespace usbInstStuff