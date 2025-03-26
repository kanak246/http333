/*
 * Copyright ©2025 Hal Perkins.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Winter Quarter 2025 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <stdint.h>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <map>
#include <string>
#include <vector>

#include "./HttpRequest.h"
#include "./HttpUtils.h"
#include "./HttpConnection.h"

using std::map;
using std::string;
using std::vector;

namespace hw4 {

static const char *kHeaderEnd = "\r\n\r\n";
static const int kHeaderEndLen = 4;

bool HttpConnection::GetNextRequest(HttpRequest *const request) {
  // Use WrappedRead from HttpUtils.cc to read bytes from the files into
  // private buffer_ variable. Keep reading until:
  // 1. The connection drops
  // 2. You see a "\r\n\r\n" indicating the end of the request header.
  //
  // Hint: Try and read in a large amount of bytes each time you call
  // WrappedRead.
  //
  // After reading complete request header, use ParseRequest() to parse into
  // an HttpRequest and save to the output parameter request.
  //
  // Important note: Clients may send back-to-back requests on the same socket.
  // This means WrappedRead may also end up reading more than one request.
  // Make sure to save anything you read after "\r\n\r\n" in buffer_ for the
  // next time the caller invokes GetNextRequest()!

  // STEP 1:

  unsigned char buf[1024];
  int bytes_read;

  // keep on going until u find that end of request
  // header
  while (buffer_.find(kHeaderEnd) == string::npos) {
    bytes_read = WrappedRead(fd_, buf, sizeof(buf));
    if (bytes_read == -1) {  // Error occured
      return false;
    }
    if (bytes_read == 0) {  // EOF occured
      break;
    }
    buffer_ += string(reinterpret_cast<char*>(buf), bytes_read);
  }
  // I want to just make sure the header end is present
  size_t header_end_pos = buffer_.find(kHeaderEnd);
  // No valid request found so have to return false
  if (header_end_pos == string::npos) {
    return false;
  }

  // Extract the complete request header
  string request_str = buffer_.substr(0, header_end_pos + kHeaderEndLen);

  // Remove the processed request from buffer_
  buffer_ = buffer_.substr(header_end_pos + kHeaderEndLen);

  // Parse the request
  *request = ParseRequest(request_str);
  return true;  // you may need to change this return value
}

bool HttpConnection::WriteResponse(const HttpResponse &response) const {
  // We use a reinterpret_cast<> to cast between unrelated pointer types, and
  // a static_cast<> to perform a conversion from an unsigned type to its
  // corresponding signed one.
  string str = response.GenerateResponseString();
  int res = WrappedWrite(fd_,
                         reinterpret_cast<const unsigned char*>(str.c_str()),
                         str.length());

  if (res != static_cast<int>(str.length()))
    return false;
  return true;
}

HttpRequest HttpConnection::ParseRequest(const string &request) const {
  HttpRequest req("/");  // by default, get "/".

  // Plan for STEP 2:
  // 1. Split the request into different lines (split on "\r\n").
  // 2. Extract the URI from the first line and store it in req.URI.
  // 3. For the rest of the lines in the request, track the header name and
  //    value and store them in req.headers_ (e.g. HttpRequest::AddHeader).
  //
  // Hint: Take a look at HttpRequest.h for details about the HTTP header
  // format that you need to parse.
  //
  // You'll probably want to look up boost functions for:
  // - Splitting a string into lines on a "\r\n" delimiter
  // - Trimming whitespace from the end of a string
  // - Converting a string to lowercase.
  //
  // Note: If a header is malformed, skip that line.

  // STEP 2:

  // First split the request into different lines using boost
  // and store them in vector
  // Check is size is 0 for malformed requests
  vector<string> req_lines;
  boost::split(req_lines, request, boost::is_any_of("\r\n"));
  if (req_lines.size() == 0) {
      return req;
  }

  // Now extract the URI from first line and
  // store it in req.uri
  // Make sure to check if it is malformed
  vector<string> first_line;
  boost::split(first_line, req_lines[0], boost::is_any_of(" "));

  // Based on HttpRequst.h:
  // format fof request is
  // GET [URI] [http_protocol]\r\n
  // So need to check if size > 3 and
  // request contains GET as first word
  if (first_line.size() < 3 || first_line[0] != "GET") {
      return req;
  }

  // At this point, I have a valid request and need to
  // parse headers from req_lines vector
  req.set_uri(first_line[1]);
  for (size_t i = 1; i < req_lines.size(); i++) {
    string header = req_lines[i];
    // split based on : in
    // [headername]: [headerval]\r\n
    // to check for malformed headers
    if (header.find(":") == string::npos) {
        continue;
    }

    // store the header name and value
    string header_name = header.substr(0, header.find(":"));
    string header_val = header.substr(header.find(":") + 1);
    boost::trim(header_name);
    boost::trim(header_val);
    boost::to_lower(header_name);
    req.AddHeader(header_name, header_val);
  }

  return req;
}

}  // namespace hw4
