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
    : public RoleGuard<WhoamiCommandProvider, Role::User, Role::Courier, Role::Admin> {

    static constexpr std::array Prototype{
      "whoami",
    };

    static bool ExecuteCommandAfterCheck(
      std::ostream& output,
      std::map<std::string, std::string> const& args) {

      if (UserService.IsLoggedIn()) {
        auto &&user = UserService.GetCurrentUser();
        
        output << std::format("name: {}\trealname: {}\t", user.name, user.realname) << std::endl;
        output << std::format("money: {}\trole: {}", user.money, to_string(user.role).c_str()) << std::endl;

        if (user.phone.size()) {
          output << "phone: " << user.phone << std::endl;
        }

        if (user.address.size()) {
          output << "address: " << user.address << std::endl;
        }

        output << std::endl;
      } else {
        output << "You're not logged in yet!" << std::endl;
      }

      return true;
    }
  };

}
