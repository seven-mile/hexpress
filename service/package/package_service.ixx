module;
#include <type_traits>
#include <format>
#include <sqlite_orm/sqlite_orm.h>
export module package_service;

namespace hexpress {

  export struct PackageState {
    constexpr static int Collecting = 0;
    constexpr static int Sending = 1;
    constexpr static int Received = 2;
  };

  export struct Package {
    int id;
    std::string name;
    std::string sender, recver;
    time_t send_time, recv_time;
    int state;
  };

  export extern constexpr time_t INVALID_TIME = static_cast<time_t>(-1);

  std::string stringify_datetime(time_t val) {
    if (val == INVALID_TIME) {
      return std::string{ "invalid" };
    }
    return std::to_string(val);
  };

  export std::string to_string(int const& state) {
    switch (state)
    {
    case PackageState::Collecting:
      return "Collecting";
    case PackageState::Sending:
      return "Sending";
    case PackageState::Received:
      return "Received";
    default:
      throw std::invalid_argument("invalid package state");
    }
  }

  export void OutputPackageInfo(
    std::ostream &output,
    Package const& pkg) {

    output << std::format("id: {}\tname: {}\t{} -> {} [{}]  {} => {}\n",
      pkg.id, pkg.name.c_str(),
      pkg.sender.c_str(), pkg.recver.c_str(),
      to_string(pkg.state),
      stringify_datetime(pkg.send_time).c_str(),
      stringify_datetime(pkg.recv_time).c_str());
  }

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
      make_column("state", &Package::state)
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

    int CollectPackage(int id);

    Package GetPackage(int id);
  };

  export CPackageService PackageService;
}
