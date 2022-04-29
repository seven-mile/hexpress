#include <sqlite_orm/sqlite_orm.h>

import package_service;
import user_service;

namespace hexpress {

  using namespace sqlite_orm;

  std::vector<Package> CPackageService::GetPackages() const {
    return pkg_db.get_all<Package>();
  }

  std::vector<Package> CPackageService::GetUserSendPackages() const {
    auto&& cur_name = UserService.GetCurrentUser().name;
    return pkg_db.get_all<Package>(where(c(&Package::sender) == cur_name));
  }

  std::vector<Package> CPackageService::GetUserRecvPackages() const {
    auto&& cur_name = UserService.GetCurrentUser().name;
    return pkg_db.get_all<Package>(where(c(&Package::recver) == cur_name));
  }

  std::vector<Package> CPackageService::GetUserFuturePackages() const {
    auto&& cur_name = UserService.GetCurrentUser().name;
    return pkg_db.get_all<Package>(where(
      c(&Package::recver) == cur_name
      && c(&Package::recved) != true));
  }

  int CPackageService::InsertPackage(Package const& pkg) {
    return pkg_db.insert<Package>(pkg);
  }

  void CPackageService::SignPackage(int id) {
    auto p = pkg_db.get<Package>(id);
    if (p.recved) {
      throw std::logic_error("package has been signed already!");
    }
    p.recved = true;
    p.recv_time = ::time(nullptr);
    pkg_db.update<Package>(p);
  }

  Package CPackageService::GetPackage(int id) {
    return pkg_db.get<Package>(id);
  }

  

}
