module;
#include <array>
#include <string>
#include <map>
#include <ostream>
#include <format>
export module assign_courier_cmd;

import user_service;
import courier_service;
import role_guard;

export namespace hexpress {

  struct AssignCourierCommandProvider
    : public RoleGuard<AssignCourierCommandProvider, Role::Admin> {

    static constexpr std::array Prototype{
      "assign_courier",
      "to_pkg_id",
    };

    static constexpr std::array Optional{
      "courier",
    };

    static bool ExecuteCommandAfterCheck(
      std::ostream& output,
      std::map<std::string, std::string> const& args) {

      try {
        
        std::string courier = "";
        if (auto it = args.find("courier"); it != args.end()) {
          courier = it->second;
        } else {
          courier = CourierService.AllocateCourier();
        }

        int pkg_id = std::strtol(args.at("to_pkg_id").c_str(), nullptr, 10);
        
        CourierService.AssignWorkItem(pkg_id, courier);

        output << std::format("this package is assigned to {} successfully", courier.c_str()) << std::endl;

      } catch (std::exception const& err) {
        output << std::format("failed to assign courier [{}]", err.what()) << std::endl;
        return false;
      }


      return true;
    }
  };

}
