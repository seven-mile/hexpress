module;
#include <stdexcept>
#include <map>
#include <ostream>
#include <string>
#include <format>
export module role_guard;

import cli;
import user_service;

namespace hexpress {

  template <Role role>
  bool CheckRole(Role r) {
    return r == role;
  }

  template <Role role1, Role role2, Role... roles>
  bool CheckRole(Role r) {
    bool res = r == role1 || r == role2;
    if constexpr (sizeof...(roles) > 0)
      res |= CheckRole<roles...>(r);
    return res;
  }

  export template <typename T, Role... roles>
  class RoleGuard {

  public:

    static bool ExecuteCommand(
      std::ostream& output,
      std::map<std::string, std::string> const& args) {

      try {
        if (CheckRole<roles...>(UserService.GetCurrentUser().role)) {
          return T::ExecuteCommandAfterCheck(output, args);
        } else {
          output << "permission denied!\n";
          return false;
        }
      } catch (std::exception const& err) {
        output << std::format("permission denied! [{}]\n", err.what());
        return false;
      }

      return true;
    }

  };

}