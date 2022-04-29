module;
#include <array>
#include <string>
#include <map>
#include <ostream>
#include <format>
#include <vector>
export module sign_pkg_cmd;

import user_service;
import package_service;
import role_guard;

export namespace hexpress {

  struct SignPkgCommandProvider
    : public RoleGuard<SignPkgCommandProvider, Role::User, Role::Admin> {

    static constexpr std::array Prototype{
      "sign_pkg",
      "id",
    };

    static bool ExecuteCommandAfterCheck(
      std::ostream& output,
      std::map<std::string, std::string> const& args) {

      try {
        int id = std::strtol(args.at("id").c_str(), nullptr, 10);
        
        PackageService.SignPackage(id);

      } catch (std::exception const& err) {
        output << std::format("failed to sign package [{}]\n", err.what());
        return false;
      }

      return true;
    }
  };

}
