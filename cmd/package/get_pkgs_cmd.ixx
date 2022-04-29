module;
#include <array>
#include <string>
#include <map>
#include <ostream>
#include <format>
#include <vector>
export module get_pkgs_cmd;

import package_service;
import role_guard;

export namespace hexpress {

  struct GetPkgsCommandProvider
    : public RoleGuard<GetPkgsCommandProvider, Role::Admin> {

    static constexpr std::array Prototype{
      "get_pkgs",
    };

    static bool ExecuteCommandAfterCheck(
      std::ostream& output,
      std::map<std::string, std::string> const& args) {

      auto stringify_datetime = [](time_t val) {
        if (val == INVALID_TIME) {
          return std::string{ "invalid" };
        }
        return std::to_string(val);
      };
      
      try {
        for (auto&& pkg : PackageService.GetPackages()) {
          output << std::format("id: {}\tname: {}\t{} -> {} [{}]  {} => {}",
            pkg.id, pkg.name.c_str(),
            pkg.sender.c_str(), pkg.recver.c_str(),
            pkg.recved ? "Received" : "Sending",
            stringify_datetime(pkg.send_time).c_str(),
            stringify_datetime(pkg.recv_time).c_str())
          << std::endl;
        }
      } catch (std::exception const& err) {
        output << std::format("failed to list packages [{}]", err.what()) << std::endl;
        return false;
      }

      return true;
    }
  };

}
