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
      
      try {
        for (auto&& pkg : PackageService.GetPackages()) {
          OutputPackageInfo(output, pkg);
        }
      } catch (std::exception const& err) {
        output << std::format("failed to list packages [{}]\n", err.what());
        return false;
      }

      return true;
    }
  };

}
