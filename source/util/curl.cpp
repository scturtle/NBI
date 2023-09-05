#include <curl/curl.h>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "util/curl.hpp"
#include "util/config.hpp"
#include "util/error.hpp"
#include "ui/instPage.hpp"

static size_t writeDataFile(void* ptr, size_t size, size_t nmemb, void* stream) {
	size_t written = fwrite(ptr, size, nmemb, (FILE*)stream);
	return written;
}

size_t writeDataBuffer(char* ptr, size_t size, size_t nmemb, void* userdata) {
	std::ostringstream* stream = (std::ostringstream*)userdata;
	size_t count = size * nmemb;
	stream->write(ptr, count);
	return count;
}

int progress_callback(void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
	if (ultotal) {
		int uploadProgress = (int)(((double)ulnow / (double)ultotal) * 100.0);
		inst::ui::instPage::setInstBarPerc(uploadProgress);
	}
	else if (dltotal) {
		int downloadProgress = (int)(((double)dlnow / (double)dltotal) * 100.0);
		inst::ui::instPage::setInstBarPerc(downloadProgress);
	}
	return 0;
}

char* unconstchar(const char* s) {
	if (!s)
		return NULL;
	int i;
	char* res = NULL;
	res = (char*)malloc(strlen(s) + 1);
	if (!res) {
		fprintf(stderr, "Memory Allocation Failed! Exiting...\n");
		exit(EXIT_FAILURE);
	}
	else {
		for (i = 0; s[i] != '\0'; i++) {
			res[i] = s[i];
		}
		res[i] = '\0';
		return res;
	}
}

namespace inst::curl {
	bool downloadFile(const std::string ourUrl, const char* pagefilename, long timeout, bool writeProgress) {
		CURL* curl_handle;
		CURLcode result;
		FILE* pagefile;

		curl_global_init(CURL_GLOBAL_ALL);
		curl_handle = curl_easy_init();

		curl_easy_setopt(curl_handle, CURLOPT_URL, ourUrl.c_str());
		curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Linux; Android 13; Pixel 7 Pro) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/112.0.0.0 Mobile Safari/537.36");
		curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, timeout);
		curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT_MS, timeout);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeDataFile);
		if (writeProgress) curl_easy_setopt(curl_handle, CURLOPT_XFERINFOFUNCTION, progress_callback);

		pagefile = fopen(pagefilename, "wb");
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);
		result = curl_easy_perform(curl_handle);

		curl_easy_cleanup(curl_handle);
		curl_global_cleanup();
		fclose(pagefile);

		if (result == CURLE_OK) return true;
		else {
			LOG_DEBUG(curl_easy_strerror(result));
			return false;
		}
	}

	std::string downloadToBuffer(const std::string ourUrl, int firstRange, int secondRange, long timeout) {
		CURL* curl_handle;
		CURLcode result;
		std::ostringstream stream;
		char* url = unconstchar(ourUrl.c_str());

		curl_global_init(CURL_GLOBAL_ALL);
		curl_handle = curl_easy_init();

		curl_easy_setopt(curl_handle, CURLOPT_URL, url);
		curl_easy_setopt(curl_handle, CURLOPT_REFERER, url);
		curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl_handle, CURLOPT_HEADER, 0L);
		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Linux; Android 13; Pixel 7 Pro) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/112.0.0.0 Mobile Safari/537.36");
		curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, timeout);
		curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT_MS, timeout);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeDataBuffer);
		if (firstRange && secondRange) {
			const char* ourRange = (std::to_string(firstRange) + "-" + std::to_string(secondRange)).c_str();
			curl_easy_setopt(curl_handle, CURLOPT_RANGE, ourRange);
		}

		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &stream);
		result = curl_easy_perform(curl_handle);

		curl_easy_cleanup(curl_handle);
		curl_global_cleanup();

		if (result == CURLE_OK) return stream.str();
		else {
			LOG_DEBUG(curl_easy_strerror(result));
			return "";
		}
	}

	std::string html_to_buffer(const std::string ourUrl) {
		CURL* curl;
		FILE* fp;
		CURLcode res{};
		char* url = unconstchar(ourUrl.c_str());
		char outfilename[FILENAME_MAX] = "temp.html";
		long time_delay = 5000;
		curl = curl_easy_init();
		curl_global_init(CURL_GLOBAL_ALL);

		if (curl)
		{
			fp = fopen(outfilename, "wb");
			curl_easy_setopt(curl, CURLOPT_URL, url);
			//
			curl_easy_setopt(curl, CURLOPT_REFERER, url);
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(curl, CURLOPT_HEADER, 0L);
			curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Linux; Android 13; Pixel 7 Pro) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/112.0.0.0 Mobile Safari/537.36");
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
			curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, time_delay);
			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, time_delay);
			//
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
			res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);
			fclose(fp);
		}
		// put file into a buffer:
		std::uintmax_t filesize = std::filesystem::file_size("temp.html");
		// Allocate buffer to hold file
		char* buf = new char[filesize];
		// Read file
		std::ifstream fin("temp.html", std::ios::binary);
		fin.read(buf, filesize);
		// Close file
		fin.close();
		std::string x = buf;

		if (res == CURLE_OK) {
			std::remove("temp.html"); // delete file
			curl_global_cleanup();
			return x;
		}
		else {
			LOG_DEBUG(curl_easy_strerror(result));
			curl_global_cleanup();
			return "";
		}
	}
}