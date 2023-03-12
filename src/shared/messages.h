#pragma once

#include <cstdint>


namespace xml_loader {


enum class messages : std::uint8_t {
  get_all_devices_description = 0x10,
  all_devices_description
};


enum class delimiters : std::uint8_t {
  end_of_word = 0x02,
  end_of_row
};

}
