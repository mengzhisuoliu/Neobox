#include <iostream>
#include <filesystem>

#include <neobox/unicode.h>

int main(int argc, char* argv[]) try {
  auto loc = std::locale { UTF8_DEFAULT_LOCALE };
  std::cout << "======================" << std::endl;
  auto prev = std::locale::global(loc);
  auto cur = std::locale {};
  if (cur.name() != loc.name()) {
    std::cerr << "Error: setlocale failed." << std::endl;
    return 1;
  }

  std::cout << "当前区域设置：" << std::locale().name() << std::endl
            << "上一个区域设置：" << prev.name() << std::endl
            << "当前区域设置：" << cur.name() << std::endl;

  std::cout << "你好，世界！" << std::endl;

  std::filesystem::path p = u8"测试中文.txt";
  std::cout << p.string() << std::endl;

#ifdef _WIN32 
#ifdef _DEBUG
  std::locale::global(prev);
  std::cout << Utf82Ansi(u8"当前区域设置：") << std::locale().name() << std::endl;
#endif
#else
  std::locale::global(prev);
  std::cout << "当前区域设置：" << std::locale().name() << std::endl;
#endif
  return 0;
} catch (const std::exception& e) {
  std::cerr << "Error: " << e.what() << std::endl;
  return 1;
}
