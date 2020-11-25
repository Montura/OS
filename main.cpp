#include <unordered_map>
#include <cstdint>
#include <fstream>
using namespace std;

// Mapping logical address to the physical address for x86 arch

// Paging: Logical address in x86 (Long Mode)
// 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
//    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |    |
//   60   56   52   48   44   40   36   32   28   24   20   16   12    8    4    0
// |                   |  PLM4   |  DirPtr   | Directory |  Table  |    Offset    |
// |    [63:48] = 47   | 47 - 39 |  38 - 30  |  29 - 21  | 20 - 12 |    12 - 0    |

static constexpr uint64_t mask(size_t from, size_t to) {
  uint64_t res = 0;
  for (size_t i = from; i <= to; ++i) {
    res |= (1ULL << i);
  }
  return res;
}

struct AddressPart {
  uint64_t const mask;
  uint64_t const shift;
  
  constexpr AddressPart(uint64_t const mask, uint64_t const shift) : mask(mask), shift(shift) {} 
};

static constexpr AddressPart const PLM4_MASK { mask(39, 47), 39 };
static constexpr AddressPart const DIRECTORY_PTR_MASK { mask(30, 38), 30 };
static constexpr AddressPart const DIRECTORY_MASK { mask(21, 29), 21 };
static constexpr AddressPart const TABLE_MASK { mask(12, 20), 12 };
static constexpr AddressPart const OFFSET_MASK { mask(0, 11), 0 };
static constexpr AddressPart const PHYSICAL_ADDRESS_MASK { mask(12, 51), 0 };
static constexpr AddressPart const LAST_BIT_MASK { mask(0, 1), 0 };

inline uint64_t part_of_logical_address(uint64_t logical_addr, AddressPart part) {
  return (logical_addr & part.mask) >> part.shift;
}

inline uint64_t value_or_default(std::unordered_map<uint64_t, uint64_t> const& map, uint64_t const key) {
  auto it = map.find(key);
  return it != map.end() ? it->second : 0;
}

uint64_t foo(uint64_t const table_base_address, uint64_t const logical_address, AddressPart const& part,
             std::unordered_map<uint64_t, uint64_t> const& map, std::ofstream & out)
{
  uint64_t table_idx        = part_of_logical_address(logical_address, part);
  uint64_t table_ph_address = part_of_logical_address(table_base_address, PHYSICAL_ADDRESS_MASK);

  auto table_key = table_ph_address + table_idx * 8;
  auto next_table_base_address = value_or_default(map, table_key);
  if (!part_of_logical_address(next_table_base_address, LAST_BIT_MASK)) { // check P bit
    out << "fault" << "\n";
    return 0;
  }
  return next_table_base_address;
}

inline void map_addr(uint64_t const logical_addr, uint64_t table_base_address,
                     std::unordered_map<uint64_t, uint64_t> const& map, std::ofstream & out)
{
  table_base_address = foo(table_base_address, logical_addr, PLM4_MASK, map, out);
  if (!table_base_address) {
    return;
  }

  table_base_address = foo(table_base_address, logical_addr, DIRECTORY_PTR_MASK, map, out);
  if (!table_base_address) {
    return;
  }

  table_base_address = foo(table_base_address, logical_addr, DIRECTORY_MASK, map, out);
  if (!table_base_address) {
    return;
  }

  table_base_address = foo(table_base_address, logical_addr, TABLE_MASK, map, out);
  if (!table_base_address) {
    return;
  }

  table_base_address =
      part_of_logical_address(table_base_address, PHYSICAL_ADDRESS_MASK) +
      part_of_logical_address(logical_addr, OFFSET_MASK);

  out << table_base_address << "\n";
}

int main() {
  auto in = ifstream("dataset_44327_15.txt", ios::in);
  auto out = ofstream("out_44327_15.txt", ios::out | ios::binary);

  if (in.is_open() && out.is_open()) {
    uint64_t table_size = 0, query_count = 0, root_address = 0;
    in >> table_size >> query_count >> root_address;
    unordered_map<uint64_t, uint64_t> m{};

    uint64_t ph_addr = 0, value = 0;
    for (size_t i = 0; i < table_size; ++i) {
      in >> ph_addr >> value;
      m[ph_addr] = value;
    }

    uint64_t logical_addr = 0;
    for (size_t i = 0; i < query_count; ++i) {
      in >> logical_addr;
      map_addr(logical_addr, root_address, m, out);
    }

    in.close();
    out.close();
  }

  return 0;
}