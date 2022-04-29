module;
#include <array>
#include <format>
#include <ostream>
#include <map>
#include <string>
#include <stdexcept>
export module delete_user_cmd;

import user_service;
import role_guard;

export namespace hexpress {

  struct DeleteUserCommandProvider
    : public RoleGuard<DeleteUserCommandProvider, Role::Admin> {

    static constexpr std::array Prototype = {
      "delete_user",
      "username"
    };

    static bool ExecuteCommandAfterCheck(
      std::ostream& output,
      std::map<std::string, std::string> const& args
    ) {

      try {

        UserService.RemoveUser(args.at("username"));

      } catch (std::exception const& err) {
        output << std::format("failed to delete user [{}]", err.what()) << std::endl;

        return false;
      }

      return true;
    }

  };

}
