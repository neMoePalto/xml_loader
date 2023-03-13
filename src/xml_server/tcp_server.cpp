#include "tcp_server.h"

#include <QDataStream>
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>


namespace xml_loader {


tcp_server::tcp_server(std::uint16_t connection_limit, QObject* parent)
  : QObject(parent)
  , connection_limit_(connection_limit) {
  server_ = new QTcpServer(this);

  connect(server_, &QTcpServer::newConnection, this, &tcp_server::accept_connection);
}


void tcp_server::restart(std::uint16_t port) {
  close();
  open(port);
}


void tcp_server::close() {
  // Закрываем все входящие соединения:
  for (auto* sock : server_->findChildren<QTcpSocket*>()) {
    sock->disconnectFromHost();
  }
  server_->close();
}


void tcp_server::send_to_client(std::uint16_t port_to, const QByteArray& data) {
  for (auto* sock : server_->findChildren<QTcpSocket*>()) {
    if (sock->peerPort() == port_to) {
      sock->write(data);
      qDebug() << "send to port " << sock->peerPort();
    }
  }
}


void tcp_server::open(std::uint16_t port) {
  if (server_->listen(QHostAddress::Any, port)) {
    emit listen_port(server_->serverPort());
    qDebug() << "Listening port" << port;
  } else {
    if (server_->serverError() == QAbstractSocket::AddressInUseError) {
      qDebug() << "This port is already in use! Can't start the server!";
      close(); // TODO: Проверить необходимость вызова
      emit port_is_busy();
    } else {
      qDebug() << "Strange server behavior";
    }
  }
}


bool tcp_server::is_connection_limit_over() const noexcept {
  return server_->findChildren<QTcpSocket*>().size() > connection_limit_;
}


void tcp_server::accept_connection() {
  QTcpSocket* sock = server_->nextPendingConnection();
  if (is_connection_limit_over()) {
    delete sock; // Работает корректно
    return;
  }

  // Отладка поведения сокетов:
//  connect(sock, &QTcpSocket::stateChanged, [](const auto state) {
//    qDebug() << "===> TCP-client state changed: " << state;
//  });

  connect(sock, &QTcpSocket::readyRead,    this, &tcp_server::read);
  connect(sock, &QTcpSocket::disconnected, this, &tcp_server::delete_socket);

  emit client_connected(sock->peerPort());
}


void tcp_server::read() {
  auto* sock = dynamic_cast<QTcpSocket*>(this->sender());
  Q_ASSERT(sock);

  QByteArray buff = sock->readAll();
  emit have_data(buff, sock->peerPort());
}


void tcp_server::delete_socket() {
  auto* sock = dynamic_cast<QTcpSocket*>(this->sender());
  if (sock == nullptr) {
    return;
  }

  emit client_disconnected(sock->peerPort());
  // Удялять сокет следует через метод deleteLater()
  // delete sock; - здесь так делать нельзя, программа будет падать
  sock->deleteLater();
//  auto sz = _server->findChildren<QTcpSocket* >().size();
//  qDebug() << "Socket's amount after delete: " << sz;
}

}
