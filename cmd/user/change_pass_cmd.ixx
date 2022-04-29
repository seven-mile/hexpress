module;
#include <array>
#include <format>
#include <ostream>
#include <map>
#include <string>
#include <stdexcept>
export module change_pass_cmd;

import user_service;
import role_guard;

export namespace hexpress {

  struct ChangePassCommandProvider
    : public RoleGuard<ChangePassCommandProvider, Role::User, Role::Courier, Role::Admin> {

    static constexpr std::array Prototype{
      "change_pass",
      "old_pass",
      "new_pass"
    };

    static bool ExecuteCommandAfterCheck(
      std::ostream& output,
      std::map<std::string, std::string> const& args) {

      try {

        if (UserService.GetCurrentUser().pass
          == args.at("old_pass")) {

          UserService.ChangePassword(args.at("new_pass"));
        } else {
          throw std::invalid_argument("wrong old password");
        }

      } catch (std::exception const& err) {
        output << std::format("failed to delete user [{}]\n", err.what());

        return false;
      }

      return true;
    }

  };

}
