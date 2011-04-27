

#include "client/linux/handler/exception_handler.h"

bool MyCallback(const char *dump_path,
   const char *minidump_id,
   void *context,
   bool succeeded) {
  printf("got crash %s %s\n", dump_path, minidump_id);
  return true;
}

void bar() {
  char* p = (char*)0;
  *p = 'a';
}

int foo() {
  bar();
}

void foo1() {
  foo();
}

void foo2() {
  foo1();
}

void foo3() {
  foo2();
}

void foo4() {
  foo3();
}

void foo5() {
  foo4();
}

int main(int argc, char * argv[]) {
  google_breakpad::ExceptionHandler eh(".", 0, MyCallback
				       , 0, true);


  // crash
  // 
  foo5();
}
