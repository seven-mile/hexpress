module;
#include <array>
#include <string>
#include <map>
#include <ostream>
#include <format>
export module register_cmd;

import user_service;

export namespace hexpress {

  struct RegisterCommandProvider {

    static constexpr std::array Prototype{
      "register",
      "username",
      "password",
      "realname",
    };

    static constexpr std::array Optional{
      "phone",
      "address",
      "money",
    };

    static bool ExecuteCommand(
      std::ostream& output,
      std::map<std::string, std::string> const& args) {

      User user;
      user.name = args.at("username");
      user.pass = args.at("password");
      user.realname = args.at("realname");

      if (auto it = args.find("phone"); it != args.end()) {
        user.phone = it->second;
      } else {
        user.phone = "";
      }

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

      user.role = Role::User;

      try {
        UserService.InsertUser(user);
      } catch (std::exception const& err) {
        output << std::format("failed to register [{}]\n", err.what());
        return false;
      }


      return true;
    }
  };

}
