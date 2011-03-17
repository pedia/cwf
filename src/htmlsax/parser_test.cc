#include <gtest/gtest.h>

#include <sstream>

#include "htmlsax/parser.h"
#include "htmlsax/handler.h"
#include "htmlsax/tidy_handler.h"

const char * foo = "<html>hello<html>";
const char * foo_end = foo + strlen(foo);

TEST(ParserTest, Compile) {
  typedef Parser<DummyHandler> DummyParser;
  DummyHandler dummy;

  DummyParser dp(&dummy);
  dp.Feed(foo, foo_end);
}

TEST(ParserTest, Script) {
  const char * html = "<html><title>hello</title>"
    "<script>"
    "alert(xss);"
    "</script>"
    "</html>"
    ;

  typedef Parser<DumpHandler> DumpParser;
  DumpHandler dump;

  DumpParser dp(&dump);
  dp.Feed(html, html+strlen(html));
}

TEST(ParserTest, ExtractContent) {
  const char * html = "<html><title>hello</title>"
    "<script>"
    "alert(xss);"
    "</script>"
    "</html>"
    ;

  std::ostringstream oss;

  typedef Parser<TidyHandler> ContentExtract;
  TidyHandler extract(oss);

  ContentExtract dp(&extract);
  dp.Feed(html, html+strlen(html));

  EXPECT_EQ("<html><title>hello</title></html>", oss.str());
}
