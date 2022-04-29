module;
#include <memory>
#include <string>
#include <stdexcept>
export module object_proto;

namespace hexpress {

  export enum class ObjectType {
    Common,
    Fragile,
    Book,
  };

  export std::string to_string(ObjectType type) {
    switch (type)
    {
    case ObjectType::Common:
      return "Common";
    case ObjectType::Fragile:
      return "Fragile";
    case ObjectType::Book:
      return "Book";
    default:
      throw std::invalid_argument("invalid object type");
    }
  }

  export ObjectType parse_object_type(std::string const& text) {
    if (text == "Common") return ObjectType::Common;
    else if (text == "Fragile") return ObjectType::Fragile;
    else if (text == "Book") return ObjectType::Book;
    else throw std::invalid_argument("invalid object type");
  }

  export class IObject {
  protected:
    int amount;
  public:
    virtual int GetPrice() const = 0;
  };

  struct CommonObject : public IObject {
    CommonObject(int _amount) { amount = _amount; }
    virtual int GetPrice() const override {
      return amount * 5;
    }
  };

  struct BookObject : public IObject {
    BookObject(int _amount) { amount = _amount; }
    virtual int GetPrice() const override {
      return amount * 2;
    }
  };

  struct FragileObject : public IObject {
    FragileObject(int _amount) { amount = _amount; }
    virtual int GetPrice() const override {
      return amount * 8;
    }
  };

  export struct ObjectFactory {
    static std::unique_ptr<IObject> CreateObject(ObjectType type, int amount) {
      switch (type)
      {
      case ObjectType::Common:
        return std::make_unique<CommonObject>(amount);
      case ObjectType::Fragile:
        return std::make_unique<FragileObject>(amount);
      case ObjectType::Book:
        return std::make_unique<BookObject>(amount);
      default:
        throw std::invalid_argument("invalid object type");
      }
    }
  };

}

