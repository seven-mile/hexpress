#include <string>
#include <vector>
#include <sqlite_orm/sqlite_orm.h>
import courier_service;
import user_service;

namespace hexpress {

  using namespace sqlite_orm;

  WorkItem CCourierService::GetWorkItem(int package_id) const {
    return workitem_db.get<WorkItem>(package_id);
  }

  void CCourierService::InsertWorkItem(WorkItem const &item) {
    // full insert
    workitem_db.insert<WorkItem>(item, columns(
      &WorkItem::courier, &WorkItem::pkg, &WorkItem::price));
  }

  void CCourierService::AssignWorkItem(
    int package_id,
    std::string const &courier) {
    
    if (UserService.GetByName(courier).role != Role::Courier) {
      throw std::invalid_argument("invalid courier to assign");
    }

    auto &&item = workitem_db.get<WorkItem>(package_id);
    item.courier = courier;
    workitem_db.update<WorkItem>(item);
  }

  std::vector<WorkItem> CCourierService::GetCourierItems(
    std::string const &courier) const {
    return workitem_db.get_all<WorkItem>(
      where(c(&WorkItem::courier) == courier));
  }

  void CCourierService::RemoveWorkItem(int package_id) {
    workitem_db.remove<WorkItem>(package_id);
  }
  std::string CCourierService::AllocateCourier() const {
    int min_count = 0x3f3f3f3f;
    std::string res{};
    for (auto&& user : UserService.GetRoleUsers(Role::Courier)) {
      if (auto cnt = workitem_db.count<WorkItem>(where(
        c(&WorkItem::courier) == user.name
        )); cnt < min_count) {
        min_count = cnt;
        res = user.name;
      }
    }
    if (res.empty()) {
      throw std::logic_error("no courier available");
    }
    return res;
  }
}
