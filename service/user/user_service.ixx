module;
#include <string>
#include <map>
#include <vector>
#include <stdexcept>
export module user_service;

namespace hexpress {
  export enum class Role : int {
    User,
    Courier,
    Admin,
  };

  export std::string to_string(Role const& role) {
    switch (role)
    {
    case Role::Admin:
      return "Admin";
    case Role::User:
      return "User";
    case Role::Courier:
      return "Courier";
    default:
      throw std::runtime_error("invalid role");
    }
  }

  export struct User {
    std::string name, pass, realname, phone, address;
    uint64_t money;
    Role role;

    bool operator==(User const& rhs) const {
      return name == rhs.name;
    }
  };

  export class CUserService {

    std::map<std::string, User> users{};
    std::string cur_user{};

    void CreateDefaultAdminUser();
    bool Read();
    bool Write();
    
    CUserService(CUserService&&) = delete;
    CUserService(CUserService const&) = delete;
    CUserService(CUserService&) = delete;

  public:

    CUserService();
    ~CUserService();

    void InsertUser(User const& user);

    User GetByName(std::string const& username) const;

    std::vector<User> GetUsers() const;

    std::vector<User> GetRoleUsers(Role role) const;

    void Login(std::string const& username, std::string const& pass);

    bool IsLoggedIn() const;

    User GetCurrentUser() const;

    User GetAdminUser() const;

    void RemoveUser(std::string const &username);

    void Pay(uint64_t money);

    void Recharge(uint64_t money);

    void FetchSalary(uint64_t salary);

    void ChangePassword(std::string const& new_pass);

  };

  export CUserService UserService;
}
