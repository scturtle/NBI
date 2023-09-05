#pragma once
#include <string>

namespace inst::curl {
	bool downloadFile(const std::string ourUrl, const char* pagefilename, long timeout = 5000, bool writeProgress = false);
	std::string downloadToBuffer(const std::string ourUrl, int firstRange = -1, int secondRange = -1, long timeout = 5000);
	std::string html_to_buffer(const std::string ourUrl);
}