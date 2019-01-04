/*
 * Copyright 2015-2018 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "crashreporter/tbg/HTTPClient.h"

#include <boost/scope_exit.hpp>

#include <curl/curl.h>

namespace http {

static struct InitCURL {
	InitCURL() {
		curl_global_init(CURL_GLOBAL_DEFAULT);
	}
} initCURL;

static size_t writeToString(void * ptr, size_t size, size_t nmemb, void * string) {
	std::string & data = *static_cast<std::string *>(string);
	data.append(reinterpret_cast<const char *>(ptr), size * nmemb);
	return nmemb;
}

class CURLSession : public Session {
	
	std::string m_userAgent;
	CURL * m_curl;
	
	void setup(const Request & request);
	Response * perform(const Request & request);
	
public:
	
	explicit CURLSession(const std::string & userAgent)
		: m_userAgent(userAgent)
		, m_curl(curl_easy_init())
	{
		if(m_curl) {
			// Enable cookies
			curl_easy_setopt(m_curl, CURLOPT_COOKIEFILE, "");
		}
	}
	
	~CURLSession() {
		if(m_curl) {
			curl_easy_cleanup(m_curl);
		}
	}
	
	Response * get(const Request & request);
	
	Response * post(const POSTRequest & request);
	
};

void CURLSession::setup(const Request & request) {
	
	if(!m_curl) {
		return;
	}
	
	curl_easy_reset(m_curl);
	
	curl_easy_setopt(m_curl, CURLOPT_URL, request.url().c_str());
	curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, request.followRedirects() ? 1l : 0l);
	
	curl_easy_setopt(m_curl, CURLOPT_USERAGENT, m_userAgent.c_str());
	
	curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 1l);
	
}

Response * CURLSession::perform(const Request & request) {
	
	if(!m_curl) {
		return new Response("CURL init failed");
	}
	
	std::string data;
	
	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, writeToString);
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &data);
	
	CURLcode res = curl_easy_perform(m_curl);
	if(res != CURLE_OK) {
		return new Response(curl_easy_strerror(res));
	}
	
	long status = 0;
	curl_easy_getinfo(m_curl, CURLINFO_RESPONSE_CODE, &status);
	
	CURLINFO urlInfo;
	if(request.followRedirects()) {
		urlInfo = CURLINFO_EFFECTIVE_URL;
	} else {
		urlInfo = CURLINFO_REDIRECT_URL;
	}
	const char * url = NULL;
	if(curl_easy_getinfo(m_curl, urlInfo, &url) != CURLE_OK || !url) {
		url = "";
	}
	
	return new Response(status, data, url);
}

Response * CURLSession::get(const Request & request) {
	
	setup(request);
	
	return perform(request);
}

Response * CURLSession::post(const POSTRequest & request) {
	
	setup(request);
	
	struct curl_slist * headers = NULL;
	BOOST_SCOPE_EXIT((headers)) {
		if(headers) {
			curl_slist_free_all(headers);
		}
	} BOOST_SCOPE_EXIT_END
	if(m_curl) {
		curl_easy_setopt(m_curl, CURLOPT_POST, 1l);
		curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, request.data().c_str());
		curl_easy_setopt(m_curl, CURLOPT_INFILESIZE_LARGE, curl_off_t(request.data().size()));
		if(!request.contentType().empty()) {
			std::string header = "Content-Type: " + request.contentType();
			headers = curl_slist_append(headers, header.c_str());
			curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, headers);
		}
	}
	
	return perform(request);
}

Session * createSession(const std::string & userAgent) {
	return new CURLSession(userAgent);
}

} // namespace http
