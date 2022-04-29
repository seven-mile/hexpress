module;
#include <string>
#include <map>
#include <stdexcept>
export module user_service;

namespace hexpress {
  export enum class Role : int {
    User,
    Admin,
  };

  export std::string to_string(Role const& role) {
    switch (role)
    {
    case hexpress::Role::Admin:
      return "Admin";
    case hexpress::Role::User:
      return "User";
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

    void Login(std::string const& username, std::string const& pass);

    bool IsLoggedIn() const;

    User GetCurrentUser() const;

    void Recharge(uint64_t money);

    void ChangePassword(std::string const& new_pass);

  };

  export CUserService UserService;
}
