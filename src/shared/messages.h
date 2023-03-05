#pragma once

#include <cstdint>


namespace xml_loader {


enum class messages : std::uint8_t {
  get_all_devices_description = 1,
  all_devices_description
};

}
