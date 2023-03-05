#pragma once

#include <cstdint>

#include <QObject>


namespace xml_loader {


class tcp_server;

class data_server : public QObject {
  Q_OBJECT

public:
  data_server();
  ~data_server(); // Temp !

private:
  tcp_server*         tcp_server_;
  const std::uint16_t connection_limit_ = 10;
  const std::uint16_t port_             = 20205;
};

}
