module;
#include <array>
#include <string>
#include <map>
#include <ostream>
#include <format>
export module whoami_cmd;

import user_service;

export namespace hexpress {

  struct WhoamiCommandProvider {

    static constexpr std::array Prototype{
      "whoami",
    };

    static bool ExecuteCommand(
      std::ostream& output,
      std::map<std::string, std::string> const& args) {

      if (UserService.IsLoggedIn()) {
        auto &&user = UserService.GetCurrentUser();
        output << std::format("{} :: {}", user.name, static_cast<int>(user.role)).c_str() << std::endl;
      } else {
        output << "You're not logged in yet!" << std::endl;
      }

      return true;
    }
  };

}
