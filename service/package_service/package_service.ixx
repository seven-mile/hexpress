module;
#include <type_traits>
#include <sqlite_orm/sqlite_orm.h>
export module package_service;

namespace hexpress {

  export struct Package {
    int id;
    std::string name;
    std::string sender, recver;
    time_t send_time, recv_time;
    bool recved;
  };

  export extern constexpr time_t INVALID_TIME = static_cast<time_t>(-1);

  using namespace sqlite_orm;

  constexpr auto PackageDBPath = "package.db",
                 PackageTableName = "package";

  static auto& GetPackageStore() {
    static auto pkg_store = make_storage(PackageDBPath, make_table(PackageTableName,
      make_column("id", &Package::id, autoincrement(), primary_key()),
      make_column("name", &Package::name),
      make_column("sender", &Package::sender),
      make_column("recver", &Package::recver),
      make_column("send_time", &Package::send_time),
      make_column("recv_time", &Package::recv_time),
      make_column("recved", &Package::recved)
    ));
    return pkg_store;
  }

  export class CPackageService {

    CPackageService(CPackageService&&) = delete;
    CPackageService(CPackageService const&) = delete;
    CPackageService(CPackageService&) = delete;

    std::invoke_result_t<decltype(GetPackageStore)> pkg_db;

  public:

    CPackageService():
      pkg_db(GetPackageStore()) {
      
      if (!pkg_db.table_exists(PackageTableName)) {
        auto &&res = pkg_db.sync_schema();
        if (auto it = res.find(PackageTableName); it != res.end()) {

          if (it->second != sync_schema_result::new_table_created) {
            throw std::runtime_error("unexpected table result when it should create table");
          }

        } else {
          throw std::runtime_error("table name not found in the sync result");
        }
      }
    }

    ~CPackageService() {

    }

    std::vector<Package> GetPackages() const;

    std::vector<Package> GetUserSendPackages() const;

    std::vector<Package> GetUserRecvPackages() const;

    std::vector<Package> GetUserFuturePackages() const;

    int InsertPackage(Package const& pkg);

    void SignPackage(int id);

    Package GetPackage(int id);
  };

  export CPackageService PackageService;
}
