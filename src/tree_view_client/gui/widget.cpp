#include "widget.h"

#include <map>

#include <netinet/in.h>
#include <string.h>

#include <QDebug>
#include <QGridLayout>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QTextEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include "../shared/messages.h"
#include "gui/net_settings_dialog.h"
#include "tcp_smart_client.h"


namespace xml_loader {


widget::widget(QWidget *parent)
  : QWidget(parent) {
  QFont noto_sans_11("Noto Sans", 11, QFont::Normal);
  setFont(noto_sans_11);

  tree_widget_ = new QTreeWidget();
  tree_widget_->setMinimumSize(900, 400);
  tree_widget_->setColumnCount(9);

  connection_status_pb_ = new QPushButton();
  connection_status_pb_->setMinimumSize(200, 55);

  auto* settings_pb = new QPushButton(tr("Изменить\nсетевые настройки"));
  settings_pb->setMinimumSize(160, 70);
  settings_widget_ = new net_settings_dialog(this);

  change_connection_status(Qt::GlobalColor::red, settings_widget_->port());

  auto* send_request_pb = new QPushButton(tr("Запрос конфигурации\nвсех устройств"));
  output_te_ = new QTextEdit(this);

  auto* grid = new QGridLayout(this);
  grid->addWidget(tree_widget_,          0, 0,   8, 6);
  grid->addWidget(connection_status_pb_, 0, 6,   1, 1, Qt::AlignTop);
  grid->addWidget(settings_pb,           0, 7,   1, 1, Qt::AlignTop);
  grid->addWidget(output_te_,            1, 6,   6, 2);
  grid->addWidget(send_request_pb,       7, 6,   1, 2);
  setLayout(grid);

  slot_client_restart();

  connect(settings_pb, &QPushButton::clicked, [this]() {
    settings_widget_->show();
  });

  connect(send_request_pb, &QPushButton::clicked, [this]() {
    Q_ASSERT(tcp_client_);

    QByteArray req;
    req.append(static_cast<char>(messages::get_all_devices_description));
    if (tcp_client_->send_to_server(req) == -1) {
      output_te_->append("tcp_smart_client: Data sending failed");
    }
  });

  connect(tcp_client_.get(), &tcp_smart_client::have_data, [this](QByteArray ba) {
    Q_ASSERT(!ba.isEmpty());

    if (ba.at(0) == static_cast<char>(messages::all_devices_description)) {
      std::uint32_t len = 0;
      if (ba.size() >= 5) {
        memcpy(&len, ba.data() + 1, sizeof(len));
        len = htonl(len);
      } else {
        qDebug() << "Warning: Uncorrect reply len" << ba.size() << ", skip it";
      }

      if (ba.size() != len) {
        qDebug() << "Warning: Uncorrect reply len (header value =" << len << "instead" << ba.size() << "), skip it";
      } else {
        ba.replace(0, 5, "");

        QList<QByteArray> rows = ba.split(static_cast<char>(delimiters::end_of_row));
        if (rows.last().isEmpty()) {
          rows.removeLast();
        }
        update_tree_widget(rows);
      }
    } else {
      qDebug() << "Warning: Unknown reply type, skip it";
    }
  });
}


widget::~widget() {
}


void widget::slot_client_restart() {
  tcp_client_ = std::make_unique<tcp_smart_client>(QHostAddress{settings_widget_->ip()},
                                                   settings_widget_->port());

  connect(tcp_client_.get(), &tcp_smart_client::connected, [this]() {
    change_connection_status(Qt::GlobalColor::green, settings_widget_->port());
  });

  connect(tcp_client_.get(), &tcp_smart_client::disconnected, [this]() {
    change_connection_status(Qt::GlobalColor::red, settings_widget_->port());
  });

  connect(tcp_client_.get(), &tcp_smart_client::connection_error, [this](QString info) {
    output_te_->append("tcp_smart_client: " + info);
  });

  settings_widget_->hide();
}


void widget::update_tree_widget(const QList<QByteArray>& rows) {
  tree_widget_->clear();

  struct item_with_id {
    QString id;
    QTreeWidgetItem* item = nullptr;
  };

  std::map<QString, QTreeWidgetItem*>  blocks;
  std::multimap<QString, item_with_id> boards;
  std::multimap<QString, item_with_id> ports;

  for (const auto& r : rows) {
    QList<QByteArray> words = r.split(static_cast<char>(delimiters::end_of_word));
    QStringList row_data;
    for (const auto& w : words) {
      row_data << w;
    }

    if (row_data.size() > 1) {
      QString row_type = row_data.value(0);
      if (row_type == "block") {
        QTreeWidgetItem* item = new QTreeWidgetItem(row_data);
        blocks.insert({row_data.value(1), item});
      } else if (row_type == "board") {
        QString parent_id = row_data.last();
        row_data.removeLast();
        QTreeWidgetItem* item = new QTreeWidgetItem(row_data);
        boards.insert({parent_id, {row_data.value(1), item}});
      } else if (row_type == "port") {
        QString parent_id = row_data.last();
        row_data.removeLast();
        QTreeWidgetItem* item = new QTreeWidgetItem(row_data);
        ports.insert({parent_id, {row_data.value(1), item}});
      } else {
        qDebug() << "Error: Unknown row type while reply parsing, skip it";
      }
    }
  }

  QList<QTreeWidgetItem*> items;
  for (const auto& bl : blocks) {
    const auto b_range = boards.equal_range(bl.first);
    for (auto it = b_range.first; it != b_range.second; ++it) {
      bl.second->addChild(it->second.item);
    }
    items.append(bl.second);
  }

  for (const auto& board : boards) {
    const auto ports_range = ports.equal_range(board.second.id);
    for (auto it = ports_range.first; it != ports_range.second; ++it) {
      board.second.item->addChild(it->second.item);
    }
  }

  tree_widget_->insertTopLevelItems(0, items);
}


void widget::change_connection_status(Qt::GlobalColor status, std::uint16_t server_port) {
  connection_status_pb_->setPalette(status);

  QString s = status == Qt::GlobalColor::red ? QString{QString{"Подключение к серверу\nчерез порт "} +
                                                       QString::number(server_port) + "..."}
                                             : QString{QString{"Соединение с портом\n"} +
                                                       QString::number(server_port) + " установлено"};
  connection_status_pb_->setText(s);
}

}
