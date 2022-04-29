module;
#include <array>
#include <string>
#include <map>
#include <ostream>
#include <format>
#include <vector>
export module get_my_pkgs_cmd;

import user_service;
import package_service;
import courier_service;
import role_guard;

export namespace hexpress {

  struct GetMyPkgsCommandProvider
    : public RoleGuard<GetMyPkgsCommandProvider, Role::User, Role::Courier, Role::Admin> {

    static constexpr std::array Prototype{
      "get_my_pkgs",
      "type"
    };

    static constexpr auto Description
      = "Get my packages with <type = sent | recv | inbox | assign>";

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
        } else if (type == "assign") {

          auto&& cur_user = UserService.GetCurrentUser();

          if (cur_user.role != Role::Courier) {
            throw std::logic_error("you're not courier, no assigned packages!");
          }

          for (auto&& item : CourierService.GetCourierItems(cur_user.name)) {
            OutputPackageInfo(output, PackageService.GetPackage(item.pkg));
          }

        } else {
          throw std::invalid_argument("invalid type parameter");
        }

      } catch (std::exception const& err) {
        output << std::format("failed to list packages [{}]\n", err.what());
        return false;
      }

      return true;
    }
  };

}
