module;
#include <iostream>
#include <string>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <map>
#include <ranges>
export module user_service;

namespace hexpress {
  export enum class Role : int {
    User,
    Admin,
  };

  export struct User {
    std::string name, realname, phone, pass, address;
    uint64_t money;
    Role role;

    bool operator==(User const& rhs) const {
      return name == rhs.name;
    }
  };

  std::istream &operator>>(std::istream& input, User &user) {
    if (!std::getline(input, user.name)) return input;
    if (!std::getline(input, user.realname)) return input;
    if (!std::getline(input, user.phone)) return input;
    if (!std::getline(input, user.pass)) return input;
    if (!std::getline(input, user.address)) return input;

    input >> user.money;

    int tmp_role = 0;
    input >> tmp_role;
    if (tmp_role != static_cast<int>(Role::User) && tmp_role != static_cast<int>(Role::Admin)) {
      input.setstate(std::ios::failbit);
      return input;
    }
    user.role = static_cast<Role>(tmp_role);

    if (input.peek() == '\n') input.get();

    return input;
  }

  std::ostream &operator<<(std::ostream& output, User const &user) {
    return output << user.name << std::endl
      << user.realname << std::endl
      << user.phone << std::endl
      << user.pass << std::endl
      << user.address << std::endl
      << user.money << std::endl
      << static_cast<int>(user.role) << std::endl;
  }

  using namespace std::filesystem;

  static path file_path = current_path() / "user.db";

  class CUserService {
    
    std::map<std::string, User> users{};
    std::string cur_user{};

    bool Read() {
      users.clear();

      std::ifstream ifs{ file_path };
      if (!ifs.is_open()) return false;
      for (User user; ifs >> user; ) {
        users[user.name] = user;
      }
      if (ifs.eof()) return true;
      if (ifs.fail()) return false;
      return true;
    }

    bool Write() {
      std::ofstream ofs{ file_path, std::ios::trunc };
      if (!ofs.is_open()) return false;
      for (auto& user : users | std::ranges::views::values)
        if (!(ofs << user)) return false;
      return true;
    }

    CUserService(CUserService&&) = delete;
    CUserService(CUserService const&) = delete;
    CUserService(CUserService&) = delete;

  public:

    CUserService() {
      if (!this->Read()) {
        std::cerr << "WARNING: failed to read data from user database." << std::endl;
      }
    }

    ~CUserService() {
      if (!this->Write()) {
        std::cerr << "WARNING: failed to write data to user database." << std::endl;
      }
    }

    void InsertUser(User const& user) {
      if (users.count(user.name)) {
        throw std::runtime_error("user duplication");
      }
      users[user.name] = user;
    }

    User GetByName(std::string const& username) const {
      //if (auto it = users.find(username); it != users.end()) {
      //  return it->second;
      //}
      //throw new std::runtime_error("user does not exist");
      return users.at(username);
    }

    void Login(std::string const& username, std::string const& pass) {
      if (auto &user = users.at(username); user.pass == pass) {
        cur_user = user.name;
      } else {
        throw new std::logic_error("wrong password");
      }
    }

    bool IsLoggedIn() const {
      return cur_user.size();
    }

    User GetCurrentUser() const {
      return GetByName(cur_user);
    }

    void Recharge(std::string const &username, uint64_t money) {
      users.at(username).money += money;
    }


  };

  export CUserService UserService{};
}
