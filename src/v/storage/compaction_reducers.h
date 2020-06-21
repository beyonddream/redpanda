#pragma once

#include "storage/compacted_index.h"
#include "storage/compacted_index_writer.h"
#include "storage/logger.h"
#include "units.h"

#include <absl/container/btree_map.h>
#include <absl/container/flat_hash_map.h>
#include <fmt/core.h>
#include <roaring/roaring.hh>

namespace storage::internal {

struct compaction_reducer {};

class truncation_offset_reducer : public compaction_reducer {
public:
    // offset to natural index mapping
    using underlying_t = absl::btree_map<model::offset, uint32_t>;

    ss::future<ss::stop_iteration> operator()(compacted_index::entry&&);
    Roaring end_of_stream();

private:
    uint32_t _natural_index = 0;
    underlying_t _indices;
};

class compaction_key_reducer : public compaction_reducer {
public:
    static constexpr const size_t max_memory_usage = 5_MiB;
    struct value_type {
        value_type(model::offset o, uint32_t i)
          : offset(o)
          , natural_index(i) {}
        model::offset offset;
        uint32_t natural_index;
    };
    using underlying_t
      = absl::flat_hash_map<bytes, value_type, bytes_type_hash, bytes_type_eq>;

public:
    // truncation_reduced are the entries *to keep* if nullopt - keep all
    // and do key-reduction
    explicit compaction_key_reducer(std::optional<Roaring> truncation_reduced)
      : _to_keep(std::move(truncation_reduced)) {}

    ss::future<ss::stop_iteration> operator()(compacted_index::entry&&);
    Roaring end_of_stream();

private:
    std::optional<Roaring> _to_keep;
    Roaring _inverted;
    underlying_t _indices;
    size_t _mem_usage{0};
    uint32_t _natural_index{0};
};

} // namespace storage::internal