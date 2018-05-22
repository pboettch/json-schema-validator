/*
 * Modern C++ JSON schema validator
 *
 * Licensed under the MIT License <http://opensource.org/licenses/MIT>.
 *
 * Copyright (c) 2016 Patrick Boettcher <patrick.boettcher@posteo.de>.
 *
 * Permission is hereby  granted, free of charge, to any  person obtaining a
 * copy of this software and associated  documentation files (the "Software"),
 * to deal in the Software  without restriction, including without  limitation
 * the rights to  use, copy,  modify, merge,  publish, distribute,  sublicense,
 * and/or  sell copies  of  the Software,  and  to  permit persons  to  whom
 * the Software  is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS
 * OR IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN
 * NO EVENT  SHALL THE AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY
 * CLAIM,  DAMAGES OR  OTHER LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT
 * OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR
 * THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include "json-schema.hpp"

namespace nlohmann
{

void local_json_pointer::from_string(const std::string &r)
{
	str_ = "#";

	if (r.size() == 0)
		return;

	if (r[0] != '#')
		throw std::invalid_argument("not a valid JSON pointer - missing # at the beginning");

	if (r.size() == 1)
		return;

	std::size_t pos = 1;

	do {
		std::size_t next = r.find('/', pos + 1);
		str_.append(r.substr(pos, next - pos));
		pos = next;
	} while (pos != std::string::npos);
}

void json_uri::update(const std::string &uri)
{
	std::string pointer = "#"; // default pointer is document-root

	// first split the URI into location and pointer
	auto pointer_separator = uri.find('#');
	if (pointer_separator != std::string::npos) // and extract the pointer-string if found
		pointer = uri.substr(pointer_separator);

	auto location = uri.substr(0, pointer_separator);

	if (location.size()) {               // a location part has been found
		pointer_ = local_json_pointer(""); // if a location is given, the pointer is emptied

		// if it is an URN take it as it is
		if (location.find("urn:") == 0) {
			urn_ = location;

			// and clear URL members
			proto_ = "";
			hostname_ = "";
			path_ = "";

		} else { // it is an URL

			// split URL in protocol, hostname and path
			std::size_t pos = 0;
			auto proto = location.find("://", pos);
			if (proto != std::string::npos) { // extract the protocol

				urn_ = ""; // clear URN-member if URL is parsed

				proto_ = location.substr(pos, proto - pos);
				pos = 3 + proto; // 3 == "://"

				auto hostname = location.find("/", pos);
				if (hostname != std::string::npos) { // and the hostname (no proto without hostname)
					hostname_ = location.substr(pos, hostname - pos);
					pos = hostname;
				}
			}

			auto path = location.substr(pos);

			// URNs cannot of have paths
			if (urn_.size() && path.size())
				throw std::invalid_argument("Cannot add a path (" + path + ") to an URN URI (" + urn_ + ")");

			if (path[0] == '/') // if it starts with a / it is root-path
				path_ = path;
			else if (pos == 0 && path_.back() != '/') // the URL contained only a path and the current path has no / at the end, it is an absolute path
				path_ = '/' + path;
			else // otherwise it is a subfolder
				path_.append(path);
		}
	}

	// if there was a pointer part store it internally
	if (pointer.size() > 0)
		pointer_ = pointer;
}

const std::string json_uri::location() const
{
	if (urn_.size())
		return urn_;

	std::stringstream s;

	if (proto_.size() > 0)
		s << proto_ << "://";

	s << hostname_
	  << path_;

	return s.str();
}

std::string json_uri::to_string() const
{
	std::stringstream s;

	s << location()
	  << pointer_.to_string();

	return s.str();
}

std::ostream &operator<<(std::ostream &os, const json_uri &u)
{
	return os << u.to_string();
}

std::string json_uri::unescape(const std::string &src)
{
	std::string l = src;
	std::size_t pos = src.size() - 1;

	do {
		pos = l.rfind('~', pos);

		if (pos == std::string::npos)
			break;

		if (pos < l.size() - 1) {
			switch (l[pos + 1]) {
			case '0':
				l.replace(pos, 2, "~");
				break;

			case '1':
				l.replace(pos, 2, "/");
				break;

			default:
				break;
			}
		}

		if (pos == 0)
			break;
		pos--;
	} while (pos != std::string::npos);

	// TODO - percent handling

	return l;
}

std::string json_uri::escape(const std::string &src)
{
	std::vector<std::pair<std::string, std::string>> chars = {
	    {"~", "~0"},
	    {"/", "~1"},
	    {"%", "%25"}};

	std::string l = src;

	for (const auto &c : chars) {
		std::size_t pos = 0;
		do {
			pos = l.find(c.first, pos);
			if (pos == std::string::npos)
				break;
			l.replace(pos, 1, c.second);
			pos += c.second.size();
		} while (1);
	}

	return l;
}

} // namespace nlohmann
