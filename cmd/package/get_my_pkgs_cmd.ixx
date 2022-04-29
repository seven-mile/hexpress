module;
#include <array>
#include <string>
#include <map>
#include <ostream>
#include <format>
#include <vector>
export module get_my_pkgs_cmd;

import package_service;
import role_guard;

export namespace hexpress {

  struct GetMyPkgsCommandProvider
    : public RoleGuard<GetMyPkgsCommandProvider, Role::User, Role::Admin> {

    static constexpr std::array Prototype{
      "get_my_pkgs",
      "type"
    };

    static constexpr auto Description
      = "Get my packages with <type = sent | recv | inbox>";

    static bool ExecuteCommandAfterCheck(
      std::ostream& output,
      std::map<std::string, std::string> const& args) {

      try {
        auto&& type = args.at("type");

        if (type == "sent") {
          for (auto&& pkg
            : PackageService.GetUserSendPackages()) {
            
            OutputPackageInfo(output, pkg);
          }
        } else if (type == "recv") {
          for (auto&& pkg
            : PackageService.GetUserRecvPackages()) {

            OutputPackageInfo(output, pkg);
          }
        } else if (type == "inbox") {
          for (auto&& pkg
            : PackageService.GetUserFuturePackages()) {

            OutputPackageInfo(output, pkg);
          }
        } else {
          throw std::invalid_argument("invalid type parameter");
        }

      } catch (std::exception const& err) {
        output << std::format("failed to list packages [{}]", err.what()) << std::endl;
        return false;
      }

      return true;
    }
  };

}
