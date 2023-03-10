#pragma once

#include <cstdint>
#include <vector>

#include <QList>
#include <QObject>
#include <QString>


namespace xml_loader {


class tcp_server;

class data_server : public QObject {
  Q_OBJECT

public:
  explicit data_server(QObject* parent = nullptr);

private:
  tcp_server*         tcp_server_;
  const std::uint16_t connection_limit_ = 10;
  const std::uint16_t port_             = 20205;
  const QString       xml_path_         = "../xml_files/";

  struct table_description {
    QString name;
    QString fields;
  };

  const QList<table_description> tables_;
};

}
