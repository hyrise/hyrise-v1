#include "column_extract.h"

namespace hyrise { namespace storage {

        class ColumnPartsCollector : public StorageVisitor {
        public:
            ColumnPartsCollector(std::uint16_t column) : column(column) {}
            std::list<Part>&& getParts() { return std::move(parts); }

        private:
            Visitation visitEnter(const MutableVerticalTable& t) override {
                offsets.push(verticalOffset);
                return Visitation::next;
            }

            Visitation visitLeave(const MutableVerticalTable& t) override {
                verticalOffset = offsets.top();
                offsets.pop();
                return Visitation::next;
            }

            Visitation visit(const Table& t) override {
                auto tableOffset = t.columnCount();

                if (verticalOffset + tableOffset > column) {
                    auto ts = t.size();
                    parts.emplace_back(Part {column - verticalOffset, horizontalOffset, horizontalOffset + ts, t});
                    horizontalOffset += ts;
                    return Visitation::skip;
                } else {
                    verticalOffset += tableOffset;
                    return Visitation::next;
                }
            }

            std::size_t column;
            std::size_t verticalOffset = 0;
            std::size_t horizontalOffset = 0;
            std::stack<std::size_t> offsets;

            std::list<Part> parts;
        };


        std::list<Part> column_parts_extract(const AbstractTable& tbl, std::size_t column) {
            ColumnPartsCollector cp(column);
            tbl.accept(cp);
            return cp.getParts();
        }
    }}
