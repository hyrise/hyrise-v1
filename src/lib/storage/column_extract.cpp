#include "column_extract.h"

namespace hyrise {
namespace storage {



class ColumnPartsCollector : public StorageVisitor {
 public:
  ColumnPartsCollector(std::size_t column, std::size_t start, std::size_t stop)
      : _column(column), _start(start), _stop(stop) {}
  std::list<Part>&& getParts() { return std::move(_parts); }

 private:
  Visitation visitEnter(const MutableVerticalTable& t) override {
    _offsets.push(_verticalOffset);
    return Visitation::next;
  }

  Visitation visitLeave(const MutableVerticalTable& t) override {
    _verticalOffset = _offsets.top();
    _offsets.pop();
    return Visitation::next;
  }

  Visitation visit(const Table& t) override {
    auto tableOffset = t.columnCount();

    if (_verticalOffset + tableOffset > _column) {
      auto ts = t.size();
      if (!((_stop < _horizontalOffset)or(_start > _horizontalOffset + ts))) {
        // auto start = std::max(_start, _horizontalOffset);
        // auto stop =  std::min(_stop, _horizontalOffset + ts);
        _parts.emplace_back(Part{_column - _verticalOffset, _horizontalOffset, _horizontalOffset + ts, t});
      }
      _horizontalOffset += ts;
      return Visitation::skip;
    } else {
      _verticalOffset += tableOffset;
      return Visitation::next;
    }
  }

  std::size_t _column;
  std::size_t _start;
  std::size_t _stop;

  std::size_t _verticalOffset = 0;
  std::size_t _horizontalOffset = 0;
  std::stack<std::size_t> _offsets;

  std::list<Part> _parts;
};


std::list<Part> column_parts_extract(const AbstractTable& tbl,
                                     std::size_t column,
                                     std::size_t start,
                                     std::size_t stop) {
  ColumnPartsCollector cp(column, start, stop);
  tbl.accept(cp);
  return cp.getParts();
}
}
}
