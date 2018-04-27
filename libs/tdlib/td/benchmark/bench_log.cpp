//
// Copyright Aliaksei Levin (levlam@telegram.org), Arseny Smirnov (arseny30@gmail.com) 2014-2018
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "td/utils/benchmark.h"
#include "td/utils/logging.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <mutex>
#include <ostream>
#include <streambuf>
#include <string>

#include <unistd.h>

std::string create_tmp_file() {
#if TD_ANDROID
  std::string name = "/data/local/tmp/large_file.txt";
  unlink(name.c_str());
  return name;
#else
  char file_name[] = "largefileXXXXXX";
  int fd = mkstemp(file_name);
  if (fd == -1) {
    perror("Can't cretate temporary file");
  }
  CHECK(fd != -1);

  close(fd);
  return file_name;
#endif
}

class IostreamWriteBench : public td::Benchmark {
 protected:
  std::string file_name_;
  std::ofstream stream;
  enum { buffer_size = 1 << 20 };
  char buffer[buffer_size];

 public:
  std::string get_description() const override {
    return "ostream (to file, no buf, no flush)";
  }

  void start_up() override {
    file_name_ = create_tmp_file();
    stream.open(file_name_.c_str());
    CHECK(stream.is_open());
    //    stream.rdbuf()->pubsetbuf(buffer, buffer_size);
  }

  void run(int n) override {
    for (int i = 0; i < n; i++) {
      stream << "This is just for test" << 987654321 << '\n';
    }
  }

  void tear_down() override {
    stream.close();
    unlink(file_name_.c_str());
  }
};

class FILEWriteBench : public td::Benchmark {
 protected:
  std::string file_name_;
  FILE *file;
  enum { buffer_size = 1 << 20 };
  char buffer[buffer_size];

 public:
  std::string get_description() const override {
    return "std::fprintf (to file, no buf, no flush)";
  }

  void start_up() override {
    file_name_ = create_tmp_file();
    file = fopen(file_name_.c_str(), "w");
    //    setvbuf(file, buffer, _IOFBF, buffer_size);
  }

  void run(int n) override {
    for (int i = 0; i < n; i++) {
      std::fprintf(file, "This is just for test%d\n", 987654321);
      //      std::fflush(file);
    }
  }

  void tear_down() override {
    std::fclose(file);
    unlink(file_name_.c_str());
  }
};

#if TD_ANDROID
#include <android/log.h>
#define ALOG(...) __android_log_print(ANDROID_LOG_VERBOSE, "XXX", __VA_ARGS__)
class ALogWriteBench : public td::Benchmark {
 public:
  std::string get_description() const override {
    return "android_log";
  }
  void start_up() override {
  }
  void run(int n) override {
    for (int i = 0; i < n; i++) {
      ALOG("This is just for test%d\n", 987654321);
    }
  }
  void tear_down() override {
  }
};
#endif

class LogWriteBench : public td::Benchmark {
 protected:
  std::string file_name_;
  std::ofstream stream;
  std::streambuf *old_buf;
  enum { buffer_size = 1 << 20 };
  char buffer[buffer_size];

 public:
  std::string get_description() const override {
    return "td_log (slow in debug mode)";
  }

  void start_up() override {
    file_name_ = create_tmp_file();
    stream.open(file_name_.c_str());
    CHECK(stream.is_open());
    old_buf = std::cerr.rdbuf(stream.rdbuf());
  }

  void run(int n) override {
    for (int i = 0; i < n; i++) {
      LOG(DEBUG) << "This is just for test" << 987654321;
    }
  }

  void tear_down() override {
    stream.close();
    unlink(file_name_.c_str());
    std::cerr.rdbuf(old_buf);
  }
};

std::mutex mutex;

int main() {
  td::bench(LogWriteBench());
#if TD_ANDROID
  td::bench(ALogWriteBench());
#endif
  td::bench(IostreamWriteBench());
  td::bench(FILEWriteBench());
  return 0;
}
