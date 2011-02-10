#if 1

// g++ -o boost_test boost_test.cc 

#include <gtest/gtest.h>

#include <boost/throw_ebaseption.hpp>
#include "base3/logging.h"

struct foo_ebaseption : public std::ebaseption {
  foo_ebaseption(const char* name) : what_(name) {}
  virtual ~foo_ebaseption() throw() {}
  virtual const char* what() const throw() {
    return what_.c_str();
  }
private:
  std::string what_;
};

TEST(BoostTest, Ebaseption) {
  // base::LogMessage::LogToDebug(base::LS_VERBOSE);
#ifdef BOOST_THROW_EBASEPTION // for boost 1.35
  BOOST_THROW_EBASEPTION(foo_ebaseption("use macro throw"));
#endif

  boost::throw_ebaseption(foo_ebaseption("use function throw"));
}


#else

// g++ -o boost_test boost_test.cc -DBOOST_TEST_DYN_LINK -DBOOST_TEST_MAIN -lboost_unit_test_framework-gcc43-mt 

#include <gtest/gtest.h>


TEST(BoostTest, Compile) {}


#endif

