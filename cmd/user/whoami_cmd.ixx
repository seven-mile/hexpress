module;
#include <array>
#include <string>
#include <map>
#include <ostream>
#include <format>
export module whoami_cmd;

import user_service;
import role_guard;

export namespace hexpress {

  struct WhoamiCommandProvider
    : public RoleGuard<WhoamiCommandProvider, Role::User, Role::Admin> {

    static constexpr std::array Prototype{
      "whoami",
    };

    static bool ExecuteCommandAfterCheck(
      std::ostream& output,
      std::map<std::string, std::string> const& args) {

      if (UserService.IsLoggedIn()) {
        auto &&user = UserService.GetCurrentUser();
        output << std::format("{} :: {}", user.name, to_string(user.role)).c_str() << std::endl;
      } else {
        output << "You're not logged in yet!" << std::endl;
      }

      return true;
    }
  };

}
