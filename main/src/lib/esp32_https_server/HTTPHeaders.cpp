#include "HTTPHeaders.hpp"

namespace httpsserver {

HTTPHeaders::HTTPHeaders() {
  _headers = new std::vector<HTTPHeader *>();
  _headers->reserve(HTTPS_REQUEST_MAX_HEADERS);
}

HTTPHeaders::~HTTPHeaders() {
  clearAll();
  delete _headers;
}

HTTPHeader * HTTPHeaders::get(std::string const &name) {
  std::string normalizedName = normalizeHeaderName(name);
  for(std::vector<HTTPHeader*>::iterator header = _headers->begin(); header != _headers->end(); ++header) {
    if ((*header)->_name.compare(normalizedName)==0) {
      return (*header);
    }
  }
  return NULL;
}

std::string HTTPHeaders::getValue(std::string const &name) {
  std::string normalizedName = normalizeHeaderName(name);
  for(std::vector<HTTPHeader*>::iterator header = _headers->begin(); header != _headers->end(); ++header) {
    if ((*header)->_name.compare(normalizedName)==0) {
      return ((*header)->_value);
    }
  }
  return "";
}


void HTTPHeaders::set(HTTPHeader * header) {
  for(int i = 0; i < _headers->size(); i++) {
    if ((*_headers)[i]->_name.compare(header->_name)==0) {
      delete (*_headers)[i];
      (*_headers)[i] = header;
      return;
    }
  }
  if (_headers->size() < HTTPS_REQUEST_MAX_HEADERS)
    _headers->push_back(header);
  else
    delete header;
}

std::vector<HTTPHeader *> * HTTPHeaders::getAll() {
  return _headers;
}

/**
 * Deletes all headers
 */
void HTTPHeaders::clearAll() {
  for(std::vector<HTTPHeader*>::iterator header = _headers->begin(); header != _headers->end(); ++header) {
    delete (*header);
  }
  _headers->clear();
  _headers->reserve(HTTPS_REQUEST_MAX_HEADERS);
}

} /* namespace httpsserver */
