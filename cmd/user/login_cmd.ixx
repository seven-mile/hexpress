module;
#include <array>
#include <string>
#include <map>
#include <ostream>
#include <format>
export module login_cmd;

import user_service;

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

      std::string username = args.at("username"), pass = args.at("password");
      
      try {
        UserService.Login(username, pass);
      } catch (std::exception const& err) {
        output << std::format("failed to login [{}]", err.what()) << std::endl;
        return false;
      }

      return true;
    }
  };

}
