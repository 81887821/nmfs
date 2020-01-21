#ifndef NMFS__CONFIGURATION_HPP
#define NMFS__CONFIGURATION_HPP

#include "structures/indexing_types/full_path.hpp"
#include "local_caches/caching_policy/evict_on_last_close.hpp"
#include "logger/log_levels.hpp"
#include "logger/log_locations.hpp"

namespace nmfs::configuration {

using indexing_type = nmfs::indexing_types::full_path;
using caching_policy = nmfs::caching_policies::evict_on_last_close;

}

#endif //NMFS__CONFIGURATION_HPP
