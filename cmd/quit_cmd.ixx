module;
#include <array>
#include <string>
#include <map>
#include <ostream>
#include <format>
export module quit_cmd;

export namespace hexpress {

  struct QuitCommandProvider {

    static constexpr std::array Prototype{
      "quit",
    };

    static bool ExecuteCommand(
      std::ostream& output,
      std::map<std::string, std::string> const& args) {
      
      output << "Quit..." << std::endl;
      exit(0);
      return true;
    }
  };

}
