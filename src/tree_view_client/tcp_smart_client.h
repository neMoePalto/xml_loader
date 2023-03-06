#pragma once

#include <cstdint>

#include <QByteArray>
#include <QHostAddress>
#include <QString>


class QTcpSocket;


namespace xml_loader {

// Класс реализует tcp-клиент с функцией автоматического периодического
// подключения к серверу:
class tcp_smart_client : public QObject {
  Q_OBJECT

public:
  tcp_smart_client(QHostAddress server_ip, std::uint16_t server_port);
  ~tcp_smart_client();
  void connect_to_host();
  std::int64_t send_to_server(const QByteArray& ba);

signals:
  void connected();
  void disconnected();
  void have_data(QByteArray);
  void connection_error(QString);

private:
  void read();
  void error(const QAbstractSocket::SocketError);

private:
  QTcpSocket*         socket_;
  const QHostAddress  server_ip_;
  const std::uint16_t server_port_;
  // TODO: Сделать параметром конструктора:
  const std::uint32_t reconnect_interval_ = 1000; // Msec
};

}
