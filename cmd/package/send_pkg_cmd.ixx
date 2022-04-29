module;
#include <array>
#include <string>
#include <map>
#include <ostream>
#include <format>
#include <vector>
export module send_pkg_cmd;

import object_proto;
import user_service;
import package_service;
import courier_service;
import role_guard;

export namespace hexpress {

  struct SendPkgCommandProvider
    : public RoleGuard<SendPkgCommandProvider, Role::User, Role::Admin> {

    static constexpr std::array Prototype{
      "send_pkg",
      "name",
      "type",
      "amount",
      "to_user",
    };

    static constexpr auto Description = "Send a package named <name> weighed <amount> with <type = common | book | fragile> <to_user>";

    static bool ExecuteCommandAfterCheck(
      std::ostream& output,
      std::map<std::string, std::string> const& args) {

      try {

        ObjectType type = parse_object_type(args.at("type"));

        int amount = std::strtol(args.at("amount").c_str(), nullptr, 10);

        auto obj = ObjectFactory::CreateObject(type, amount);

        UserService.Pay(obj->GetPrice());

        auto&& to_user = args.at("to_user");
        // ensure existance
        UserService.GetByName(to_user);

        // record it in the database
        int id = PackageService.InsertPackage(Package {
          .id = -1, // auto increamental
          .name = args.at("name"),
          .sender = UserService.GetCurrentUser().name,
          .recver = to_user,
          .send_time = ::time(nullptr),
          .recv_time = INVALID_TIME,
          .state = PackageState::Collecting,
        });

        output << std::format("you have paid {} units of money for express.\n"
          "your package has been sent successfully! tracking number: {}.\n\n",
          obj->GetPrice(), id);

        // record an empty work item to wait for courier allocation
        CourierService.InsertWorkItem(WorkItem{
          .courier = "",
          .pkg = id,
          .price = obj->GetPrice(),
        });

      } catch (std::exception const& err) {
        output << std::format("failed to send package [{}]\n", err.what());
        return false;
      }

      return true;
    }
  };

}
