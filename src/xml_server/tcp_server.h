#pragma once

#include <cstdint>
#include <vector>

#include <QByteArray>
#include <QObject>


class QTcpServer;


namespace xml_loader {


class tcp_server : public QObject {
  Q_OBJECT

public:
  explicit tcp_server(std::uint16_t connection_limit, QObject* parent = nullptr);

  void restart(std::uint16_t port);
  void close();
  void send_to_client(std::uint16_t port_to, const QByteArray& data);

signals:
  void client_connected(std::uint16_t port_from);
  void client_disconnected(std::uint16_t port_from);
  void have_data(QByteArray data, std::uint16_t port_from);
  void port_is_busy();
  void listen_port(std::uint16_t port);

private:
  void open(std::uint16_t port);
  bool is_connection_limit_over() const noexcept;
  void accept_connection();
  void read();
  void delete_socket();

private:
  QTcpServer*         server_;
  std::vector<char>   buff_;
  const std::uint16_t connection_limit_; // Искусственное ограничение "на всякий случай"
};

}
