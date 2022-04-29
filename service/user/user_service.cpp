#include <iostream>
#include <string>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <map>
#include <ranges>

import user_service;

namespace hexpress {

  inline std::filesystem::path GetUserDBPath() {
    return std::filesystem::current_path() / "user.db";
  }

  std::istream& operator>>(std::istream& input, User& user) {
    if (!std::getline(input, user.name)) return input;
    if (!std::getline(input, user.realname)) return input;
    if (!std::getline(input, user.phone)) return input;
    if (!std::getline(input, user.pass)) return input;
    if (!std::getline(input, user.address)) return input;

    input >> user.money;

    int tmp_role = 0;
    input >> tmp_role;
    if (tmp_role != static_cast<int>(Role::User)
      && tmp_role != static_cast<int>(Role::Admin)
      && tmp_role != static_cast<int>(Role::Courier)) {
      input.setstate(std::ios::failbit);
      return input;
    }
    user.role = static_cast<Role>(tmp_role);

    if (input.peek() == '\n') input.get();

    return input;
  }

  std::ostream& operator<<(std::ostream& output, User const& user) {
    return output << user.name << std::endl
      << user.realname << std::endl
      << user.phone << std::endl
      << user.pass << std::endl
      << user.address << std::endl
      << user.money << std::endl
      << static_cast<int>(user.role) << std::endl;
  }


  void CUserService::CreateDefaultAdminUser() {
    users.emplace(
      std::string{ "admin" },
      User{
        .name = "admin",
        .pass = "admin",
        .realname = "admin",
        .phone = "",
        .address = "",
        .money = 0,
        .role = Role::Admin,
      }
    );
  }
  
  bool CUserService::Read() {
    users.clear();

    std::ifstream ifs{ GetUserDBPath() };
    if (!ifs.is_open()) return false;
    for (User user; ifs >> user; ) {
      users[user.name] = user;
    }
    if (ifs.eof()) return true;
    if (ifs.fail()) return false;
    return true;
  }

  bool CUserService::Write() {
    std::ofstream ofs{ GetUserDBPath(), std::ios::trunc };
    if (!ofs.is_open()) return false;
    for (auto& user : users
      | std::ranges::views::values)
      if (!(ofs << user)) return false;
    return true;
  }

  CUserService::CUserService() {
    if (!this->Read()) {
      std::cerr << "WARNING: failed to read data from user database." << std::endl;
    }
    if (!users.count("admin")) {
      CreateDefaultAdminUser();
    }
  }

  CUserService::~CUserService() {
    if (!this->Write()) {
      std::cerr << "WARNING: failed to write data to user database." << std::endl;
    }
  }

  void CUserService::InsertUser(User const& user) {
    if (users.count(user.name)) {
      throw std::runtime_error("user duplication");
    }
    users[user.name] = user;
  }

  User CUserService::GetByName(std::string const& username) const {
    //if (auto it = users.find(username); it != users.end()) {
    //  return it->second;
    //}
    //throw new std::runtime_error("user does not exist");
    return users.at(username);
  }

  std::vector<User> CUserService::GetUsers() const {
    std::vector<User> res;
    for (auto&& user : users | std::ranges::views::values) {
      res.emplace_back(user);
    }
    return std::move(res);
  }

  std::vector<User> CUserService::GetRoleUsers(Role role) const
  {
    std::vector<User> res;
    for (auto&& user : users
      | std::ranges::views::values
      | std::ranges::views::filter(
        [role](User const& user) {
          return user.role == role;
        }
      )) {
      res.emplace_back(user);
    }
    return std::move(res);
  }

  void CUserService::Login(std::string const& username, std::string const& pass) {
    if (auto& user = users.at(username); user.pass == pass) {
      cur_user = user.name;
    } else {
      throw std::logic_error("wrong password");
    }
  }

  bool CUserService::IsLoggedIn() const {
    return cur_user.size();
  }

  User CUserService::GetCurrentUser() const {
    return GetByName(cur_user);
  }

  User CUserService::GetAdminUser() const {
    return GetByName("admin");
  }

  void CUserService::RemoveUser(std::string const& username) {
    users.erase(username);
  }

  void CUserService::Pay(uint64_t money) {
    auto& cur = users.at(cur_user);
    if (cur.money < money) {
      throw std::logic_error("insufficient balance");
    }
    users.at("admin").money += money;
    cur.money -= money;
  }

  void CUserService::Recharge(uint64_t money) {
    users.at(cur_user).money += money;
  }

  void CUserService::FetchSalary(uint64_t salary) {
    auto& admin = users.at("admin");
    auto& cur = users.at(cur_user);
    if (admin.money < salary) {
      throw std::logic_error("company bankrupted!");
    }
    admin.money -= salary;
    cur.money += salary;
  }

  void CUserService::ChangePassword(std::string const& new_pass)
  {
    users.at(cur_user).pass = new_pass;
  }

}