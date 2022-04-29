module;
#include <array>
#include <string>
#include <map>
#include <ostream>
#include <format>
#include <vector>
export module collect_pkg_cmd;

import user_service;
import package_service;
import courier_service;
import role_guard;

export namespace hexpress {

  struct CollectPkgCommandProvider
    : public RoleGuard<CollectPkgCommandProvider, Role::Courier> {

    static constexpr std::array Prototype{
      "collect_pkg",
      "id",
    };

    static bool ExecuteCommandAfterCheck(
      std::ostream& output,
      std::map<std::string, std::string> const& args) {

      try {
        int id = std::strtol(args.at("id").c_str(), nullptr, 10);
        
        int got_salary = PackageService.CollectPackage(id);

        output << std::format("great! successfully collected, got {} as salary\n", got_salary);

      } catch (std::exception const& err) {
        output << std::format("failed to collect package [{}]\n", err.what());
        return false;
      }

      return true;
    }
  };

}
