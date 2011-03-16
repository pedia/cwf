#ifndef HTMLSAX_CONTENT_HANDLER_H__
#define HTMLSAX_CONTENT_HANDLER_H__

#include <boost/algorithm/string.hpp>
#include <iostream>

// a { href / name, title }

class TidyHandler {
public:
  TidyHandler(std::ostream & os = std::cout) 
    : os_(os), care_data_(false)
  {}  

  bool start_document() {
    return true;
  }
  bool start_element(const crange& name, const attr_array& arr) {
    if (!name.empty() && boost::iequals(name, "A")) {
      care_data_ = true;

      os_ << "<a {";

      // read href
      attr_array::const_iterator i = FindAttribute(arr, "HREF");

      if (i != arr.end()) {
        os_ << i->second << ", ";
      }
      else {
        i = FindAttribute(arr, "NAME");

        //                assert((arr.size() && i != arr.end()) || !arr.size());
        //                for (attr_array::const_iterator i=arr.begin(); i!=arr.end(); ++i)
        //                {
        //                    std::cerr << i->first << "=" << i->second << "\n";
        //                }
      }
    }

//     if (!name.empty() && boost::iequals(name, "TITLE")) {
//       is_title_ = true;
//     }
    return true;
  }

  bool characters(const crange& text) {
    if (care_data_) {
      os_ << text << "}\n";
      care_data_ = false;
    }

    return true;
  }
  bool end_element(const crange& name) {
    return true;
  }
  bool entities(const crange& text) {
    return true;
  }
  bool script(const crange& text, const attr_array&) {
    return true;
  }
  bool end_document() {
    return true;
  }

  bool care_data_;

private:
  std::ostream & os_;
};

#endif // HTMLSAX_CONTENT_HANDLER_H__
