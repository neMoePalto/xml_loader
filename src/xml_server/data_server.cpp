#include "data_server.h"

#include <array>

#include <netinet/in.h>
#include <string.h>

#include <QByteArray>
#include <QDebug>
#include <QDir>
#include <QDomDocument>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>

#include "../shared/messages.h"
#include "tcp_server.h"


namespace xml_loader {


data_server::data_server(QObject* parent)
  : QObject(parent)
  , tables_{{"blocks", "id, Name, IP, BoardCount, MtR, MtC, Description, Label"},
            {"boards", "id, Num, Name, PortCount, IntLinks, Algoritms, block_id"},
            {"ports",  "id, Num, Media, Signal, board_id"}} {
  // Подключаемся к БД:
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
//  db.setHostName("some_host");
//  db.setUserName("some_user");
//  db.setPassword("1234");
  db.setDatabaseName("devices.sqlite3");
  if (!db.open()) {
    qFatal("%s", db.lastError().text().toStdString().c_str());
  }

  // Создаем (или пересоздаем) таблицы БД:
  for (const auto& t : tables_) {
    QSqlQuery query("DROP TABLE IF EXISTS " + t.name);
    if (query.lastError().isValid()) {
      qDebug() << query.lastError();
    }
  }

  QSqlQuery q1("CREATE TABLE " + tables_.at(0).name + " ("
               "id          INT  NOT NULL PRIMARY KEY, "
               "Name        TEXT NOT NULL, "
               "IP          TEXT NOT NULL, "
               "BoardCount  INT  NOT NULL, "
               "MtR         INT  NOT NULL, "
               "MtC         INT  NOT NULL, "
               "Description TEXT NOT NULL, "
               "Label       TEXT NOT NULL)");
  if (q1.lastError().isValid()) {
    qFatal("%s", db.lastError().text().toStdString().c_str());
  }

  // TODO: По-хорошему, стоит сделать отдельную таблицу для Algoritms [board_id, Algoritm]:
  QSqlQuery q2("CREATE TABLE " + tables_.at(1).name + " ("
               "id        INT  NOT NULL PRIMARY KEY, "
               "Num       INT  NOT NULL, "
               "Name      TEXT NOT NULL, "
               "PortCount INT  NOT NULL, "
               "IntLinks  TEXT, "
               "Algoritms TEXT, "
               "block_id  INT  NOT NULL)");
  if (q1.lastError().isValid()) {
    qFatal("%s", db.lastError().text().toStdString().c_str());
  }

  QSqlQuery q3("CREATE TABLE " + tables_.at(2).name + " ("
               "id       INT NOT NULL PRIMARY KEY, "
               "Num      INT NOT NULL, "
               "Media    INT NOT NULL, "
               "Signal   INT NOT NULL, "
               "board_id INT NOT NULL)");
  if (q1.lastError().isValid()) {
    qFatal("%s", db.lastError().text().toStdString().c_str());
  }

  // Читаем имена файлов:
  QDir dir(xml_path_);
  QStringList filter;
  filter << "*.xml";
  dir.setNameFilters(filter);
  QStringList file_names = dir.entryList(QDir::Files, QDir::Name);
  if (file_names.isEmpty()) {
    QString info{"Error: directory \"" + dir.path() + "\" - no xml-files inside or bad path"};
    qFatal("%s", info.toStdString().c_str());
  }

  auto insert_into_blocks_table = [table = tables_.at(0), this] (const QDomNode& block_node) -> std::int32_t {
    if (block_node.nodeName() != "block") {
      qDebug() << "Error: unexpected xml-node" << block_node.nodeName() << "in file" << xml_path_;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO " + table.name +
                  " (" + table.fields + ") VALUES (?, ?, ?, ?, ?, ?, ?, ?)");

    auto el = block_node.toElement();
    std::array<bool, 4> converted{false, false, false, false};
    auto id = el.attribute("id").toInt(&converted.at(0));
    query.addBindValue(id);
    query.addBindValue(el.attribute("Name"));
    query.addBindValue(el.attribute("IP"));
    query.addBindValue(el.attribute("BoardCount").toInt(&converted.at(1)));
    query.addBindValue(el.attribute("MtR").toInt(&converted.at(2)));
    query.addBindValue(el.attribute("MtC").toInt(&converted.at(3)));
    query.addBindValue(el.attribute("Description"));
    query.addBindValue(el.attribute("Label"));

    const auto it = std::find(converted.cbegin(), converted.cend(), false);
    if (it != converted.cend()) {
      qDebug() << "Error: xml attribute has type unconvertable to int. "
                  "Skip insertion into DB table" << table.name;
      return -1;
    }

    query.exec();
    if (query.lastError().isValid()) {
      qDebug() << query.lastError();
      return -1;
    }

    return id; // TODO: В будущем - заменить на std::optional<>, которого пока нет в С++14
  };

  auto insert_into_ports_table = [table = tables_.at(2), this](const QDomNode& board_node, std::int32_t board_id) {
    for (auto node = board_node.firstChild(); !node.isNull(); node = node.nextSibling()) {
      if (node.nodeName() != "port") {
        qDebug() << "Error: unexpected xml-node" << node.nodeName() << "in file" << xml_path_;
      }

      QSqlQuery query;
      query.prepare("INSERT INTO " + table.name +
                    " (" + table.fields + ") VALUES (?, ?, ?, ?, ?)");

      auto el = node.toElement();
      std::array<bool, 4> converted{false, false, false, false};
      query.addBindValue(el.attribute("id").toInt(&converted.at(0)));
      query.addBindValue(el.attribute("Num").toInt(&converted.at(1)));
      query.addBindValue(el.attribute("Media").toInt(&converted.at(2)));
      query.addBindValue(el.attribute("Signal").toInt(&converted.at(3)));
      query.addBindValue(board_id);

      const auto it = std::find(converted.cbegin(), converted.cend(), false);
      if (it != converted.cend()) {
        qDebug() << "Error: xml attribute has type unconvertable to int. "
                    "Skip insertion into DB table" << table.name;
        continue;
      }

      query.exec();
      if (query.lastError().isValid()) {
        qDebug() << query.lastError();
      }
    }
  };

  auto insert_into_boards_table = [table = tables_.at(1), this, insert_into_ports_table](const QDomNode& block_node,
                                                                                         std::int32_t block_id) {
    for (auto node = block_node.firstChild(); !node.isNull(); node = node.nextSibling()) {
      if (node.nodeName() != "board") {
        qDebug() << "Error: unexpected xml-node" << node.nodeName() << "in file" << xml_path_;
      }

      QSqlQuery query;
      query.prepare("INSERT INTO " + table.name +
                 " (id, Num, Name, PortCount, IntLinks, Algoritms, block_id)"
                 " VALUES (?, ?, ?, ?, ?, ?, ?)");

      auto el = node.toElement();
      std::array<bool, 3> converted{false, false, false};
      auto id = el.attribute("id").toInt(&converted.at(0));
      query.addBindValue(id);
      query.addBindValue(el.attribute("Num").toInt(&converted.at(1)));
      query.addBindValue(el.attribute("Name"));
      query.addBindValue(el.attribute("PortCount").toInt(&converted.at(2)));
      query.addBindValue(el.attribute("IntLinks"));
      query.addBindValue(el.attribute("Algoritms"));
      query.addBindValue(block_id);

      const auto it = std::find(converted.cbegin(), converted.cend(), false);
      if (it != converted.cend()) {
        qDebug() << "Error: xml attribute has type unconvertable to int. "
                    "Skip insertion into DB table" << table.name;
        continue;
      }

      query.exec();
      if (query.lastError().isValid()) {
        qDebug() << query.lastError();
        continue;
      }

      insert_into_ports_table(node, id);
    }
  };

  // Переписываем данные из файлов в таблицы БД:
  for (const auto& file_name : file_names) {
    QFile file(xml_path_ + file_name);
    qDebug() << "Reading file" << file.fileName();

    if (!file.open(QIODevice::ReadOnly)) {
      qDebug() << "Error: can't open file" << file.fileName();
      continue;
    }

    QDomDocument doc("device_description");
    QString error_msg;
    if (!doc.setContent(&file, &error_msg)) {
      qDebug() << QString{"Error in file " + file.fileName() + ": " + error_msg};
      file.close();
      continue;
    }

    file.close();

    QDomElement docElem = doc.documentElement();
    if (docElem.hasChildNodes()) {
      QDomNode block = docElem.firstChild();
      auto block_id = insert_into_blocks_table(block);
      if (block_id > 0) {
        insert_into_boards_table(block, block_id);
      }
    } else {
      qDebug() << "Error: unsupported xml syntax in file" << file.fileName();
    }
  }

  // Запускаем tcp-сервер:
  tcp_server_ = new tcp_server(connection_limit_, this);
  tcp_server_->restart(port_);

  connect(tcp_server_, &tcp_server::client_connected, [](const std::uint16_t port) {
    qDebug() << "New connection from port" << port;
  });

  connect(tcp_server_, &tcp_server::client_disconnected, [](const std::uint16_t port) {
    qDebug() << "Connection from port" << port << "closed";
  });

  auto prepare_all_devices_description = [this]() {
    QByteArray ba;
    ba.append(static_cast<char>(messages::all_devices_description));
    ba.append(4, 0x00); // Резервируем место для поля "длина сообщения"

    auto copy_table_contents_to_byte_array = [](QByteArray& ba, const table_description& table) {
      QSqlQuery query("SELECT " + table.fields + " FROM " + table.name);
      while (query.next()) {
        QString name = table.name;
        name.resize(name.size() - 1);
        ba += name.toUtf8();

        for (int i = 0; query.value(i).isValid(); ++i) {
          ba += static_cast<char>(delimiters::end_of_word);
          ba += query.value(i).toString().toUtf8();
        }
        ba += static_cast<char>(delimiters::end_of_row);
      }
    };

    for (const auto& table : tables_) {
      copy_table_contents_to_byte_array(ba, table);
    }

    std::uint32_t len = htonl(ba.length());
    Q_ASSERT(ba.length() >= 5);
    memcpy(ba.data() + 1, &len, sizeof(len));

    return ba;
  };

  connect(tcp_server_, &tcp_server::have_data,
          [this, prepare_all_devices_description](QByteArray data, std::uint16_t port) {
            if (!data.isEmpty()) {
              if (data.at(0) == static_cast<char>(messages::get_all_devices_description)) {
                if (data.size() != 1) {
                  qDebug() << "Warning: Client" << port << ". Uncorrect request len" << data.size() << ", expected 1";
                }
                QByteArray ba = prepare_all_devices_description();
                tcp_server_->send_to_client(port, ba);
              } else {
                qDebug() << "Warning: Client" << port << ". Unknown request type, skip it";
              }
            }
          });
}

}
