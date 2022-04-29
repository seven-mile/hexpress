#include <sqlite_orm/sqlite_orm.h>

import package_service;
import user_service;
import courier_service;

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
      && c(&Package::state) == PackageState::Sending));
  }

  int CPackageService::InsertPackage(Package const& pkg) {
    return pkg_db.insert<Package>(pkg);
  }

  void CPackageService::SignPackage(int id) {
    auto&& cur_user = UserService.GetCurrentUser().name;
    if (auto&& pkg = pkg_db.get<Package>(id);
      pkg.recver == cur_user) {
      if (pkg.state == PackageState::Collecting) {
        throw std::logic_error("the package is still being collected!");
      }
      if (pkg.state == PackageState::Received) {
        throw std::logic_error("package has been signed already!");
      }
      pkg.state = PackageState::Received;
      pkg.recv_time = ::time(nullptr);
      pkg_db.update<Package>(pkg);
    } else {
      throw std::logic_error("hey, that's not your package!");
    }
  }

  int CPackageService::CollectPackage(int id)
  {
    if (auto item = CourierService.GetWorkItem(id);
      item.courier == UserService.GetCurrentUser().name) {
      auto&& pkg = pkg_db.get<Package>(id);
      if (pkg.state != PackageState::Collecting) {
        CourierService.RemoveWorkItem(id);
        throw std::logic_error("the package has been collected!");
      }
      pkg.state = PackageState::Sending;
      pkg_db.update<Package>(pkg);
      CourierService.RemoveWorkItem(id);
      UserService.FetchSalary(item.price / 2);
      
      return item.price / 2;
    } else {
      throw std::logic_error("hey, that's not your package!");
    }

  }

  Package CPackageService::GetPackage(int id) {
    return pkg_db.get<Package>(id);
  }

}
