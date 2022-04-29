module;
#include <array>
#include <stdexcept>
#include <format>
#include <ostream>
#include <vector>
export module recharge_cmd;

import user_service;
import role_guard;

export namespace hexpress {

  struct RechargeCommandProvider
    : public RoleGuard<RechargeCommandProvider, Role::User, Role::Admin> {

    static constexpr std::array Prototype{
      "recharge",
      "money"
    };

    static bool ExecuteCommandAfterCheck(
      std::ostream& output,
      std::map<std::string, std::string> const& args) {

      try {

        int money = std::strtol(args.at("money").c_str(), nullptr, 10);
        UserService.Recharge(money);
        
      } catch (std::exception const& err) {
        output << std::format("failed to recharge [{}]", err.what()) << std::endl;
        return false;
      }

      return true;
    }
  };

}
