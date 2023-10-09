/*
Copyright (c) 2017-2018 Adubbz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <cstring>
#include <string>
#include <algorithm>
#include <sys/errno.h>
#include <fcntl.h>
#include <sstream>
#include <curl/curl.h>
#include <switch.h>
#include "ThemeInstall.hpp"
#include "util/error.hpp"
#include "util/config.hpp"
#include "util/util.hpp"
#include "util/curl.hpp"
#include "util/lang.hpp"
#include "ui/MainApplication.hpp"
#include "util/theme.hpp"

bool netConnected2 = false;

namespace inst::ui {
	extern MainApplication* mainApp;
}

namespace inst::ui {
	std::string themei_root = inst::config::appDir + "/theme";
	bool themei_theme = util::themeit(themei_root); //check if we have a previous theme directory first.
}

namespace ThemeInstStuff {

	//Strip the filename from the url we are trying to download - ie list.txt, index.html etc...
	std::string stripfilename(const std::string& s) {

		int pos = 0;
		std::string mystr = s;
		pos = mystr.find_last_of('/');

		//if the url doesn't contain "/" after "http://" don't bother stripping as we won't need to.
		if (pos > 7) {
			mystr = mystr.substr(0, pos);
		}

		return mystr;
	}

	//Find Case Insensitive Sub String in a given substring
	size_t findCaseInsensitive(std::string data, std::string toSearch, size_t pos = 0)
	{
		// Convert complete given String to lower case
		std::transform(data.begin(), data.end(), data.begin(), ::tolower);
		// Convert complete given Sub String to lower case
		std::transform(toSearch.begin(), toSearch.end(), toSearch.begin(), ::tolower);
		// Find sub string in given string
		return data.find(toSearch, pos);
	}

	std::string urlencode(std::string str)
	{
		std::string new_str = "";
		char c;
		int ic;
		const char* chars = str.c_str();
		char bufHex[10];
		int len = strlen(chars);

		for (int i = 0; i < len; i++) {
			c = chars[i];
			ic = c;
			// bodge code needed to prevernt encoded url changing forward slash to %2F
			if (c == '/') new_str += '/';
			else if (c == '.') new_str += '.';
			else if (c == ':') new_str += ':';
			else if (c == '\\') new_str += '/'; //windows paths break urls - change the path mod..
			else if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') new_str += c;
			else {
				sprintf(bufHex, "%X", c);
				if (ic < 16)
					new_str += "%0";
				else
					new_str += "%";
				new_str += bufHex;
			}
		}
		return new_str;
	}

	std::string url_decode(const std::string& encoded)
	{
		int output_length;
		const auto decoded_value = curl_easy_unescape(nullptr, encoded.c_str(), static_cast<int>(encoded.length()), &output_length);
		std::string result(decoded_value, output_length);
		curl_free(decoded_value);
		return result;
	}

	void OnUnwound()
	{
		LOG_DEBUG("unwinding view\n");
		curl_global_cleanup();
	}

	std::vector<std::string> OnSelected()
	{
		padConfigureInput(8, HidNpadStyleSet_NpadStandard);
		PadState pad;
		padInitializeAny(&pad);
		u64 freq = armGetSystemTickFreq();
		u64 startTime = armGetSystemTick();
		OnUnwound();

		std::string info = "romfs:/images/icons/information.png";
		if (inst::ui::themei_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.information"_theme)) {
			info = inst::config::appDir + "icons_others.information"_theme;
		}
		std::string fail = "romfs:/images/icons/fail.png";
		if (inst::ui::themei_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.fail"_theme)) {
			fail = inst::config::appDir + "icons_others.fail"_theme;
		}
		std::string wait = "romfs:/images/icons/wait.png";
		if (inst::ui::themei_theme && inst::config::useTheme && std::filesystem::exists(inst::config::appDir + "/theme/theme.json") && std::filesystem::exists(inst::config::appDir + "icons_others.wait"_theme)) {
			wait = inst::config::appDir + "icons_others.wait"_theme;
		}

		try {
			ASSERT_OK(curl_global_init(CURL_GLOBAL_ALL), "Curl failed to initialized");

			std::string ourIPAddress = inst::util::getIPAddress();
			inst::ui::mainApp->ThemeinstPage->pageInfoText->SetText("inst.net.top_info1"_lang + ourIPAddress);
			inst::ui::mainApp->CallForRender();
			netConnected2 = false;
			LOG_DEBUG("%s %s\n", "Switch IP is ", ourIPAddress.c_str());
			LOG_DEBUG("%s\n", "Waiting for network");
			LOG_DEBUG("%s\n", "B to cancel");
			std::vector<std::string> urls;
			std::vector<std::string> tmp_array;

			while (true) {
				padUpdate(&pad);

				// If we don't update the UI occasionally the Switch basically crashes on this screen if you press the home button
				u64 newTime = armGetSystemTick();

				if (newTime - startTime >= freq * 0.25) {
					startTime = newTime;
					inst::ui::mainApp->CallForRender();
				}

				u64 kDown = padGetButtonsDown(&pad);
				if (kDown & HidNpadButton_B)
				{
					break;
				}

				std::remove("temp.html");
				std::string url;
				unsigned short maxlist = 50;
				unsigned short nowlist = 0;

				if (inst::config::httpkeyboard) {
					url = inst::util::softwareKeyboard("theme.hint_theme"_lang, inst::config::httplastUrl2, 500);
					inst::config::httplastUrl2 = url;
					inst::config::setConfig();
					//refresh options page
					inst::ui::mainApp->optionspage->setMenuText();
				}
				else {
					url = inst::config::httplastUrl2;
				}

				if (url == "") {
					url = ("http://127.0.0.1");
					inst::ui::mainApp->CreateShowDialog("theme.theme_fail"_lang, "inst.net.help.blank"_lang, { "common.ok"_lang }, true, info);
					inst::config::httplastUrl2 = url;
					inst::config::setConfig();
					//refresh options page
					inst::ui::mainApp->optionspage->setMenuText();
					break;
				}

				else {
					std::string response;
					if (inst::util::formatUrlString(url) == "" || url == "https://" || url == "http://" || url == "HTTP://" || url == "HTTPS://") {
						inst::ui::mainApp->CreateShowDialog("inst.net.url.invalid"_lang, "", { "common.ok"_lang }, false, fail);
						break;
					}
					else {
						if (url[url.size() - 1] != '/') //does this line even do anything?

							//First try and see if we have any links for zip files
							response = inst::curl::downloadToBuffer(url);

						//If the above fails we probably have an html page - try to download it instead.
						if (response.empty()) {
							response = inst::curl::html_to_buffer(url);
							if (response.empty()) {
								inst::ui::mainApp->CreateShowDialog("theme.theme_error"_lang, "theme.theme_error_info"_lang, { "common.ok"_lang }, true, fail);
								break;
							}
						}
					}

					if (!response.empty()) {
						if (response[0] == '*') {
							try {
								nlohmann::json j = nlohmann::json::parse(response);
								for (const auto& file : j["files"]) {
									urls.push_back(file["url"]);
								}

								return urls;
								response.clear();
							}
							catch (const nlohmann::detail::exception& ex) {
								LOG_DEBUG("Failed to parse JSON\n");
							}
						}
						else if (!response.empty()) {
							std::size_t index = 0;
							if (!inst::config::listoveride) {
								inst::ui::mainApp->CreateShowDialog("inst.net.url.listwait"_lang + std::to_string(maxlist) + "inst.net.url.listwait2"_lang, "", { "common.ok"_lang }, false, wait);
							}
							while (index < response.size()) {
								std::string link;
								auto found = findCaseInsensitive(response, "href=\"", index);
								if (found == std::string::npos) {
									break;
								}

								index = found + 6;
								while (index < response.size()) {
									if (response[index] == '"') {
										if (findCaseInsensitive(link, ".zip") != std::string::npos) {
											/*
											Try to see if the href links contain http - if not add the own url
											defined in the settings page
											*/
											if (!inst::config::listoveride && nowlist >= maxlist) {
												break;
											}
											else {
												if (link.find("http") == std::string::npos) {
													std::string before_strip = stripfilename(url);
													if (link[0] == '/') {
														tmp_array.push_back(before_strip + link);
														nowlist++;
													}
													else {
														tmp_array.push_back(before_strip + "/" + link);
														nowlist++;

													}
												}
												else {
													tmp_array.push_back(link);
													nowlist++;
												}
											}
										}
										break; //don't remove this or the net install screen will crash
									}
									link += response[index++];
								}
							}
							if (tmp_array.size() > 0) {

								//code to decode the url (if it's encoded), if not then (re)encode all urls.
								for (unsigned long int i = 0; i < tmp_array.size(); i++) {
									std::string debug = tmp_array[i];
									std::string decoded = url_decode(debug);
									debug = urlencode(decoded);
									urls.push_back(debug);
								}

								tmp_array.clear(); //we may as well clear this now as it's done it's job..
								std::sort(urls.begin(), urls.end(), inst::util::ignoreCaseCompare);
								return urls;
								break;
							}

							else {
								inst::ui::mainApp->CreateShowDialog("theme.no_themes"_lang, "", { "common.ok"_lang }, false, fail);
								LOG_DEBUG("Failed to parse themes from HTML\n");
								break;
							}
							response.clear();
						}

						else {
							inst::ui::mainApp->CreateShowDialog("theme.theme_error"_lang, "theme.theme_error_info"_lang, { "common.ok"_lang }, true, fail);
							break;
						}
					}

					else {
						LOG_DEBUG("Failed to fetch theme list\n");
						inst::ui::mainApp->CreateShowDialog("theme.theme_error"_lang, "theme.theme_error_info"_lang, { "common.ok"_lang }, true, fail);
						break;
					}
				}
			}
			return urls;
		}
		catch (std::runtime_error& e) {
			LOG_DEBUG("Failed to perform remote install!\n");
			LOG_DEBUG("%s", e.what());
			fprintf(stdout, "%s", e.what());
			inst::ui::mainApp->CreateShowDialog("inst.net.failed"_lang, (std::string)e.what(), { "common.ok"_lang }, true, fail);
			return {};
		}
	}
}
