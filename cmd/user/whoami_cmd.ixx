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
        
        output << std::format("name: {}\trealname: {}\n", user.name, user.realname);
        output << std::format("money: {}\trole: {}\n", user.money, to_string(user.role).c_str());

        if (user.phone.size()) {
          output << "phone: " << user.phone << '\n';
        }

        if (user.address.size()) {
          output << "address: " << user.address << '\n';
        }

        output << '\n';
      } else {
        output << "You're not logged in yet!" << '\n';
      }

      return true;
    }
  };

}
