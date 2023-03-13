#include "tcp_smart_client.h"

#include <QDebug>
#include <QTcpSocket>
#include <QTimer>


namespace xml_loader {


tcp_smart_client::tcp_smart_client(QHostAddress server_ip, std::uint16_t server_port)
  : server_ip_(server_ip)
  , server_port_(server_port) {
  socket_ = new QTcpSocket(this);

  connect(socket_, &QTcpSocket::errorOccurred, this, &tcp_smart_client::error);
  connect(socket_, &QTcpSocket::connected,     this, &tcp_smart_client::connected);
  connect(socket_, &QTcpSocket::disconnected,  this, &tcp_smart_client::disconnected);
  connect(socket_, &QTcpSocket::readyRead,     this, &tcp_smart_client::read);

  connect_to_host();
}


tcp_smart_client::~tcp_smart_client() {
  emit disconnected();
}


void tcp_smart_client::connect_to_host() {
  socket_->connectToHost(server_ip_, server_port_);
}


std::int64_t tcp_smart_client::send_to_server(const QByteArray& ba) {
  std::int64_t res = socket_->write(ba);

  if (res != ba.size()) {
    qDebug() << "tcp_smart_client::send_to_server(): sended" << res << "bytes instead" << ba.size();
  }

  return res;
}


void tcp_smart_client::read() {
  QByteArray buff = socket_->readAll();
  emit have_data(buff);
}


void tcp_smart_client::error(const QAbstractSocket::SocketError error) {
  QString error_info;
  switch (error) {
    case QAbstractSocket::RemoteHostClosedError:
      error_info = "RemoteHostClosedError";
      break;
    case QAbstractSocket::HostNotFoundError:
      error_info = "HostNotFoundError";
      break;
    case QAbstractSocket::ConnectionRefusedError:
      error_info = "ConnectionRefusedError";
      break;
    default:
      error_info = socket_->errorString();
      break;
  }

  emit connection_error(error_info);

  socket_->disconnectFromHost(); // TODO: Скорее всего, не нужен. Проверить при случае
  QTimer::singleShot(reconnect_interval_, this, &tcp_smart_client::connect_to_host);
}

}
