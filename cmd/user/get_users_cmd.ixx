module;
#include <array>
#include <stdexcept>
#include <format>
#include <ostream>
#include <vector>
export module get_users_cmd;

import user_service;
import role_guard;

export namespace hexpress {

  struct GetUsersCommandProvider
    : public RoleGuard<GetUsersCommandProvider, Role::Admin> {

    static constexpr std::array Prototype{
      "get_users",
    };

    static bool ExecuteCommandAfterCheck(
      std::ostream& output,
      std::map<std::string, std::string> const& args) {

      try {
        
        for (auto&& user : UserService.GetUsers()) {
          output << std::format("name: {}\trealname: {}\t", user.name, user.realname) << std::endl;
          output << std::format("money: {}\trole: {}", user.money, to_string(user.role).c_str()) << std::endl;

          if (user.phone.size()) {
            output << "phone: " << user.phone << std::endl;
          }

          if (user.address.size()) {
            output << "address: " << user.address << std::endl;
          }

          output << std::endl;
        }

      } catch (std::exception const& err) {
        output << std::format("failed to list users [{}]", err.what()) << std::endl;
        return false;
      }

      return true;
    }
  };

}
