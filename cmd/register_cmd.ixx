module;
#include <array>
#include <string>
#include <map>
#include <ostream>
#include <format>
export module register_cmd;

export namespace hexpress {

  struct RegisterCommandProvider {

    static constexpr std::array Prototype{
      "register",
      "username",
      "password",
    };

    static bool ExecuteCommand(
      std::ostream& output,
      std::map<std::string, std::string> const& args) {

      output << std::format("register successfully ;)",
        args.at("username"),
        args.at("password"))
        << std::endl;

      return true;
    }
  };

}
