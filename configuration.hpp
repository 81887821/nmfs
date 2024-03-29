#ifndef NMFS__CONFIGURATION_HPP
#define NMFS__CONFIGURATION_HPP

#include "structures/indexing_types/all.fwd.hpp"
#include "local_caches/caching_policy/all.fwd.hpp"
#include "logger/log_levels.hpp"
#include "logger/log_locations.hpp"

namespace nmfs::configuration {

using indexing = nmfs::structures::indexing_types::custom::indexing;
template<typename indexing>
using caching_policy = nmfs::caching_policies::hold_closed_cache_for<indexing, 30/*seconds*/>;

}

#endif //NMFS__CONFIGURATION_HPP
