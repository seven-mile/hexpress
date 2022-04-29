module;
#include <array>
#include <string>
#include <map>
#include <ostream>
#include <format>
export module create_courier_cmd;

import user_service;
import role_guard;

export namespace hexpress {

  struct CreateCourierCommandProvider
    : public RoleGuard<CreateCourierCommandProvider, Role::Admin> {

    static constexpr std::array Prototype{
      "create_courier",
      "username",
      "password",
      "realname",
      "phone",
    };

    static constexpr std::array Optional{
      "address",
      "money",
    };

    static bool ExecuteCommandAfterCheck(
      std::ostream& output,
      std::map<std::string, std::string> const& args) {

      User user;
      user.name = args.at("username");
      user.pass = args.at("password");
      user.realname = args.at("realname");
      user.phone = args.at("phone");

      if (auto it = args.find("address"); it != args.end()) {
        user.address = it->second;
      } else {
        user.address = "";
      }

      if (auto it = args.find("money"); it != args.end()) {
        user.money = std::strtoull(it->second.c_str(), nullptr, 10);
      } else {
        user.money = 0;
      }

      user.role = Role::Courier;

      try {
        UserService.InsertUser(user);
      } catch (std::exception const& err) {
        output << std::format("failed to create courier [{}]\n", err.what());
        return false;
      }


      return true;
    }
  };

}
