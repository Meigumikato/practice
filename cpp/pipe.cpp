#include <functional>
#include <iostream>
#include <string>
#include <type_traits>


enum class ConfigFormat {
  XML = 0,
  JSON,
  YAML,
  TOML
};



template<typename T, typename Callable>
auto operator | (T&& t, Callable&& callable) -> std::invoke_result_t<Callable, T> {
  return std::move(callable)(std::forward<T>(t));
}


std::string ReadFile(const std::string& filename) {

  std::cout << "FileName = "<< filename << "\n";
  return "ReadFile";
}

template <ConfigFormat format>
std::string ParseFileContent(std::string&& a) {

  if (format == ConfigFormat::JSON) {

  }

  std::cout << a << "Complete \n";
  return "ParseFileContent";
}

void CreateSome(std::string&& a) {

  std::cout << a << "Complete \n";
}


int main() {


  std::string st;

  st | [] (auto& s) -> std::string { return "nihao"; };

  // std::invoke_result_t<decltype(Stage2), std::string> s;

  ReadFile("./config.json") | ParseFileContent<ConfigFormat::JSON> | CreateSome;

  return 0;
}
