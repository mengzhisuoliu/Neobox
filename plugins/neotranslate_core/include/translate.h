#ifndef TRANSLATE_H
#define TRANSLATE_H

#include <stdint.h>
#include <map>
#include <string>
#include <vector>

class LanPair {
public:
  double& from;
  double& to;
public:
  explicit LanPair(class YJson& array);
  auto f() const { return static_cast<size_t>(from); }
  auto t() const { return static_cast<size_t>(to); } 
};

class Translate {
public:
  // enum class Lan { AUTO, ZH_CN, ZH_TW, EN_US, JA_JP, FR_LU, RU_RU, MAX };
  enum Source { Baidu = 0, Youdao = 1, None = 2 } m_Source;
  typedef std::vector<std::vector<std::pair<std::u8string, std::vector<std::u8string>>>> LanguageMap;

public:
  explicit Translate(class YJson& setting);
  ~Translate();

public:
  const std::u8string& GetResult(const std::u8string& text);
  void SetSource(Source dict);
  inline Source GetSource() const { return m_Source; }

private:
  LanPair m_LanPairBaidu, m_LanPairYoudao;
public:
  LanPair* m_LanPair;
  static std::map<std::u8string, std::u8string> m_LangNameMap;
  static const LanguageMap m_LanguageCanFromTo;

private:
  std::u8string GetResultBaidu(const std::u8string& text);
  std::u8string GetResultYoudao(const std::u8string& text);
  static void FormatYoudaoResult(std::u8string& result,
                                 const class YJson& data);
};

#endif  // TRANSLATE_H
