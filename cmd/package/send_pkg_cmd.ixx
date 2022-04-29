module;
#include <array>
#include <string>
#include <map>
#include <ostream>
#include <format>
#include <vector>
export module send_pkg_cmd;

import user_service;
import package_service;
import role_guard;

export namespace hexpress {

  struct SendPkgCommandProvider
    : public RoleGuard<SendPkgCommandProvider, Role::User, Role::Admin> {

    static constexpr int SEND_PKG_FEE = 15;

    static constexpr std::array Prototype{
      "send_pkg",
      "name",
      "to_user",
    };

    static bool ExecuteCommandAfterCheck(
      std::ostream& output,
      std::map<std::string, std::string> const& args) {

      try {

        UserService.Pay(SEND_PKG_FEE);

        PackageService.InsertPackage(Package {
          .id = -1, // auto increamental
          .name = args.at("name"),
          .sender = UserService.GetCurrentUser().name,
          .recver = args.at("to_user"),
          .send_time = ::time(nullptr),
          .recv_time = INVALID_TIME,
          .recved = false,
        });

      } catch (std::exception const& err) {
        output << std::format("failed to send package [{}]", err.what()) << std::endl;
        return false;
      }

      return true;
    }
  };

}
