module;
#include <string>
#include <map>
#include <filesystem>
#include <sqlite_orm/sqlite_orm.h>
export module courier_service;

namespace hexpress {

  export struct WorkItem {
    std::string courier;
    int pkg;
    int price;
  };

  using namespace sqlite_orm;

  static constexpr auto WorkItemDBPath = "workitem.db",
                        WorkItemTableName = "workitem";

  static auto &GetWorkItemStore() {
    static auto table = make_storage(WorkItemDBPath, make_table(WorkItemTableName,
      make_column("courier", &WorkItem::courier),
      make_column("pkg", &WorkItem::pkg, primary_key()),
      make_column("price", &WorkItem::price)
    ));
    return table;
  }

  export class CCourierService {

    CCourierService(CCourierService&&) = delete;
    CCourierService(CCourierService const&) = delete;
    CCourierService(CCourierService&) = delete;

    std::invoke_result_t<decltype(GetWorkItemStore)> workitem_db;

  public:

    CCourierService(): workitem_db(GetWorkItemStore()) {
      // sync database table
      if (!workitem_db.table_exists(WorkItemTableName)) {
        auto&& res = workitem_db.sync_schema();
        if (auto it = res.find(WorkItemTableName); it != res.end()) {

          if (it->second != sync_schema_result::new_table_created) {
            throw std::runtime_error("unexpected table result when it should create table");
          }

        } else {
          throw std::runtime_error("table name not found in the sync result");
        }
      }
    }

    ~CCourierService() { }

    WorkItem GetWorkItem(int package_id) const;

    void InsertWorkItem(WorkItem const &item);

    void AssignWorkItem(int package_id, std::string const& courier);

    std::vector<WorkItem> GetCourierItems(std::string const &courier) const;

    void RemoveWorkItem(int package_id);

    std::string AllocateCourier() const;

  };

  export CCourierService CourierService;

}
