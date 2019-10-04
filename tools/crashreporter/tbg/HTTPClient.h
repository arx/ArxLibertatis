/*
 * Copyright 2015-2019 Arx Libertatis Team (see the AUTHORS file)
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

/*!
 * \file
 *
 * Simple HTTP client supporting GET and POST requests
 */
#ifndef ARX_TOOLS_CRASHREPORTER_TBG_HTTPCLIENT_H
#define ARX_TOOLS_CRASHREPORTER_TBG_HTTPCLIENT_H

#include <string>

namespace http {

//! HTTP request information
class Request {
	
	std::string m_url;
	bool m_followRedirects;
	
public:
	
	/* implicit */ Request(const std::string & url)
		: m_url(url)
		, m_followRedirects(true)
	{ }
	
	/* implicit */ Request(const char * url)
		: m_url(url)
		, m_followRedirects(true)
	{ }
	
	/*!
	 * Set weather redirects returned by the server should be followed
	 *
	 * Redirects will be followed by default and the response for the final request
	 * returned.
	 *
	 * Set to \c false to retrieve the initial respnse.
	 */
	void setFollowRedirects(bool follow) {
		m_followRedirects = follow;
	}
	
	const std::string & url() const {
		return m_url;
	}
	
	bool followRedirects() const {
		return m_followRedirects;
	}
	
};

//! HTTP POST request information
class POSTRequest : public Request {
	
	std::string m_data;
	std::string m_contentType;
	
	std::string defaultContentType() {
		return "application/x-www-form-urlencoded";
	}
	
public:
	
	/* implicit */ POSTRequest(const std::string & url)
		: Request(url)
		, m_contentType(defaultContentType())
	{ }
	
	/* implicit */ POSTRequest(const char * url)
		: Request(url)
		, m_contentType(defaultContentType())
	{ }
	
	/* implicit */ POSTRequest(const Request & request)
		: Request(request)
		, m_contentType(defaultContentType())
	{ }
	
	//! Set the POST data
	void setData(const std::string & data) {
		m_data = data;
	}
	
	//! Set the mime type of the post data
	void setContentType(const std::string & type) {
		m_contentType = type;
	}
	
	const std::string & data() const {
		return m_data;
	}
	
	const std::string & contentType() const {
		return m_contentType;
	}
	
};

//! HTTP response data
class Response {
	
	int m_status;
	std::string m_error;
	std::string m_data;
	std::string m_url;
	
public:
	
	explicit Response(const std::string & error) : m_status(0), m_error(error) { }
	explicit Response(const char * error) : m_status(0), m_error(error) { }
	Response(int status, const std::string & data, const std::string & url)
		: m_status(status)
		, m_data(data)
		, m_url(url)
	{ }
	
	//! HTTP status code returned by the server or \c 0 if there was an error
	int status() { return m_status; }
	
	//! Check if the request completed successfully
	bool ok() { return m_error.empty() && m_status >= 200 && m_status < 400; }
	
	//! Get a string describing the error that prevented the request
	const std::string & error() const { return m_error; }
	
	//! Get the content returned by the server
	const std::string & data() const { return m_data; }
	
	/*!
	 * Get the redirect or effective URL
	 *
	 * If following redirects was enabled for the request, this returns the
	 * the final URL in the redirect chain - the one that this response comes from.
	 *
	 * Otherwise the redirect URL from the response is returned, or an empty string
	 * if there was no redirect URL.
	 */
	const std::string & url() const { return m_url; }
	
};

/*!
 * HTTP client session object
 *
 * If multiple requests are made using the same session object, connections will
 * be reused if possible and cookies returned by the server will be added to
 * subsequent requests.
 */
class Session {
	
public:
	
	virtual ~Session() { }
	
	/*!
	* Send a HTTP GET request
	* 
	* \return A \ref Response object that should be freed using delete.
	*/
	virtual Response * get(const Request & request) = 0;
	
	/*!
	* Send a HTTP POSY request
	* 
	* \return A \ref Response object that should be freed using delete.
	*/
	virtual Response * post(const POSTRequest & request) = 0;
	
};

/*!
 * Create a new session object
 *
 * \param userAgent HTTP User-Agent header to send with requests.
 * 
 * \return A \ref Session object that should be freed using delete.
 */
Session * createSession(const std::string & userAgent);

} // namespace http

#endif // ARX_TOOLS_CRASHREPORTER_TBG_HTTPCLIENT_H
