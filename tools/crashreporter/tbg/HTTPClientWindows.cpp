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

#include <windows.h>
#include <winhttp.h>

// <wininet.h> conflicts with <winhttp.h>
extern "C" BOOL WINAPI InternetCombineUrlW(LPCWSTR base, LPCWSTR relative, LPWSTR out,
                                           LPDWORD size, DWORD flags);

#include <boost/scope_exit.hpp>

#include "platform/WindowsUtils.h"

namespace http {

class WinHTTPSession : public Session {
	
	HINTERNET m_session;
	INTERNET_SCHEME m_scheme;
	std::basic_string<WCHAR> m_host;
	INTERNET_PORT m_port;
	HINTERNET m_connection;
	
	HINTERNET setup(const Request & request, LPCWSTR method);
	Response * receive(HINTERNET wrequest, const Request & request,
	                   std::basic_string<WCHAR> & url);
	
public:
	
	explicit WinHTTPSession(const std::string & userAgent)
		: m_session(WinHttpOpen(platform::WideString(userAgent), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		                        WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0))
	{ }
	
	~WinHTTPSession() {
		if(m_connection) {
			WinHttpCloseHandle(m_connection);
		}
		if(m_session) {
			WinHttpCloseHandle(m_session);
		}
	}
	
	Response * get(const Request & request);
	
	Response * post(const POSTRequest & request);
	
};

static std::string errorString(DWORD error = GetLastError()) {
	return platform::getErrorString(error, GetModuleHandleW(L"winhttp.dll"));
}

static void APIENTRY statusCallback(HINTERNET handle, DWORD_PTR context, DWORD status,
                                    LPVOID info, DWORD size) {
	if(status == WINHTTP_CALLBACK_STATUS_REDIRECT && context && size >= 1) {
		std::basic_string<WCHAR> * url = reinterpret_cast<std::basic_string<WCHAR> *>(context);
		url->assign(reinterpret_cast<LPWSTR>(info), size - 1);
	}
}

HINTERNET WinHTTPSession::setup(const Request & request, LPCWSTR method) {
	
	platform::WideString wurl(request.url());
	
	URL_COMPONENTS url;
	ZeroMemory(&url, sizeof(url));
	url.dwStructSize = sizeof(url);
	url.dwSchemeLength = url.dwHostNameLength = url.dwUrlPathLength = DWORD(-1);
	if(WinHttpCrackUrl(wurl, 0, 0, &url) != TRUE) {
		throw new Response("Invalid URL: \"" + request.url() + "\"");
	}
	
	DWORD flags = 0;
	if(url.nScheme == INTERNET_SCHEME_HTTPS) {
		flags |= WINHTTP_FLAG_SECURE;
	} else if(url.nScheme != INTERNET_SCHEME_HTTP) {
		throw new Response("Unsupported protocol");
	}
	
	if(m_scheme != url.nScheme || m_port != url.nPort
	   || m_host.compare(0, m_host.length(), url.lpszHostName, url.dwHostNameLength) != 0) {
		if(m_connection) {
			WinHttpCloseHandle(m_connection);
		}
		m_scheme = url.nScheme;
		m_host.assign(url.lpszHostName, url.dwHostNameLength);
		m_port = url.nPort;
		m_connection = WinHttpConnect(m_session, m_host.c_str(), m_port, 0);
	}
	if(!m_connection) {
		throw new Response("Could not create connection object: " + errorString());
	}
	
	LPCWSTR resource = url.dwUrlPathLength ? url.lpszUrlPath : L"/";
	
	LPCWSTR accept[] = { L"*/*", NULL };
	
	HINTERNET wrequest = WinHttpOpenRequest(m_connection, method, resource, NULL,
	                                        WINHTTP_NO_REFERER, accept, flags);
	if(!wrequest) {
		throw new Response("Could not create request object: " + errorString());
	}
	
	if(request.followRedirects()) {
		WinHttpSetStatusCallback(wrequest, statusCallback, WINHTTP_CALLBACK_FLAG_REDIRECT, 0);
	} else {
		ULONG disable = WINHTTP_DISABLE_REDIRECTS;
		WinHttpSetOption(wrequest, WINHTTP_OPTION_DISABLE_FEATURE, &disable, sizeof(disable));
	}
	
	return wrequest;
}

Response * WinHTTPSession::receive(HINTERNET wrequest, const Request & request,
                                   std::basic_string<WCHAR> & redirect) {
	
	
	
	if(!WinHttpReceiveResponse(wrequest, NULL)) {
		return new Response("Error receiving response: " + errorString());
	}
	
	DWORD status = 0;
	DWORD statusSize = sizeof(status);
	
	if(!WinHttpQueryHeaders(wrequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
	                        WINHTTP_HEADER_NAME_BY_INDEX, &status, &statusSize,
	                        WINHTTP_NO_HEADER_INDEX)) {
		return new Response("Error getting status code: " + errorString());
	}
	
	std::string url;
	if(!request.followRedirects()) {
		DWORD urlSize = 0;
		WinHttpQueryHeaders(wrequest, WINHTTP_QUERY_LOCATION, WINHTTP_HEADER_NAME_BY_INDEX,
		                    WINHTTP_NO_OUTPUT_BUFFER, &urlSize, WINHTTP_NO_HEADER_INDEX);
		if(GetLastError() ==  ERROR_INSUFFICIENT_BUFFER && urlSize % sizeof(WCHAR) == 0) {
			platform::WideString redirect;
			redirect.allocate(urlSize / sizeof(WCHAR));
			if(WinHttpQueryHeaders(wrequest, WINHTTP_QUERY_LOCATION, WINHTTP_HEADER_NAME_BY_INDEX,
			                       redirect.data(), &urlSize, WINHTTP_NO_HEADER_INDEX)) {
				redirect.resize(urlSize / sizeof(WCHAR));
				platform::WideString base(request.url());
				platform::WideString wurl;
				wurl.allocate(2 * (base.size() + redirect.size()));
				urlSize = wurl.size();
				if(InternetCombineUrlW(base, redirect, wurl.data(), &urlSize, ICU_BROWSER_MODE)) {
					wurl.resize(urlSize);
					url = wurl.toUTF8();
				}
			}
		}
	} else {
		url = redirect.empty() ? request.url() : platform::WideString::toUTF8(redirect);
	}
	
	std::string data;
	DWORD size;
	for(;;) {
		
		// Check for available data.
		size = 0;
		if(!WinHttpQueryDataAvailable(wrequest, &size)) {
			return new Response("Error reading response: " + errorString());
		}
		if(!size) {
			break;
		}
		
		size_t oldsize = data.size();
		data.resize(oldsize + size);
		
		if(!WinHttpReadData(wrequest, &data[oldsize], size, &size)) {
			return new Response("Error reading response: " + errorString());
		}
		
		data.resize(oldsize + size);
	}
	
	return new Response(status, data, url);
}

Response * WinHTTPSession::get(const Request & request) {
	
	HINTERNET wrequest = 0;
	try {
		wrequest = setup(request, L"GET");
	} catch(Response * error) {
		return error;
	}
	BOOST_SCOPE_EXIT((wrequest)) {
		WinHttpCloseHandle(wrequest);
	} BOOST_SCOPE_EXIT_END
	
	std::basic_string<WCHAR> redirect;
	BOOL result = WinHttpSendRequest(wrequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
	                                 WINHTTP_NO_REQUEST_DATA, 0, 0, reinterpret_cast<DWORD_PTR>(&redirect));
	if(!result) {
		return new Response(errorString());
	}
	
	return receive(wrequest, request, redirect);
}

Response * WinHTTPSession::post(const POSTRequest & request) {
	
	HINTERNET wrequest = 0;
	try {
		wrequest = setup(request, L"POST");
	} catch(Response * error) {
		return error;
	}
	BOOST_SCOPE_EXIT((wrequest)) {
		WinHttpCloseHandle(wrequest);
	} BOOST_SCOPE_EXIT_END
	
	LPCWSTR headers = WINHTTP_NO_ADDITIONAL_HEADERS;
	platform::WideString wheaders;
	if(!request.contentType().empty()) {
		wheaders = "Content-Type: " + request.contentType() + "\r\n";
		headers = wheaders;
	}
	
	LPVOID data = const_cast<LPVOID>(static_cast<LPCVOID>(request.data().data()));
	DWORD size = request.data().size();
	std::basic_string<WCHAR> redirect;
	BOOL result = WinHttpSendRequest(wrequest, headers, -1, data, size, size,
	                                reinterpret_cast<DWORD_PTR>(&redirect));
	if(!result) {
		return new Response("Could not send request: " + errorString());
	}
	
	return receive(wrequest, request, redirect);
}

Session * createSession(const std::string & userAgent) {
	return new WinHTTPSession(userAgent);
}

} // namespace http
