module;
#include <array>
#include <string>
#include <map>
#include <ostream>
#include <format>
export module login_cmd;

export namespace hexpress {

  struct LoginCommandProvider {

    static constexpr std::array Prototype{
      "login",
      "username",
      "password",
    };

    static bool ExecuteCommand(
      std::ostream& output,
      std::map<std::string, std::string> const& args) {

      output << std::format("hello from login command,\n{}\n{}\ncopying your password ;)",
        args.at("username"),
        args.at("password")) 
        << std::endl;

      return true;
    }
  };

}
