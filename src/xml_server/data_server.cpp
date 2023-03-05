#include "data_server.h"

#include <vector>

#include <QByteArray> // is need ?
#include <QDebug>

#include "../shared/messages.h"
#include "tcp_server.h"


namespace xml_loader {


data_server::data_server() {
  tcp_server_ = new tcp_server(connection_limit_); // TODO: Пробросить this в ctor
  tcp_server_->restart(port_);

  connect(tcp_server_, &tcp_server::client_connected, [](const std::uint16_t port) {
    qDebug() << "New connection from port" << port;
  });

  connect(tcp_server_, &tcp_server::client_disconnected, [](const std::uint16_t port) {
    qDebug() << "Connection from port" << port << "closed";
  });

  connect(tcp_server_, &tcp_server::have_data, [this](std::vector<char>& data, std::uint16_t port) {
    if (!data.empty()) {
      if (data.at(0) == static_cast<char>(messages::get_all_devices_description)) {
        if (data.size() != 1) {
          qDebug() << "Warning: Client" << port << ". Uncorrect request len" << data.size() << ", expected 1";
        }
        // TODO: Готовим и отправляем ответ (QByteArray или std::vector<char>... ?
        QByteArray ba;
        ba.append(static_cast<char>(messages::all_devices_description));
        tcp_server_->send_to_client(port, ba);
      } else {
        qDebug() << "Warning: Client" << port << ". Unknown request type, skip it";
      }
    }
  });
}


data_server::~data_server() {
  delete tcp_server_;
}

}
