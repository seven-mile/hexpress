module;
#include <string>
#include <map>
export module user_service;

namespace hexpress {
  export enum class Role : int {
    User,
    Admin,
  };

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

    void Recharge(std::string const& username, uint64_t money);

  };

  export CUserService UserService;
}
