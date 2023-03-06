#include "widget.h"

#include <QDebug>
#include <QGridLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QTreeView>

#include "../shared/messages.h"
#include "gui/net_settings_dialog.h"
#include "tcp_smart_client.h"


namespace xml_loader {


widget::widget(QWidget *parent)
  : QWidget(parent) {
  QFont noto_sans_11("Noto Sans", 11, QFont::Normal);
  setFont(noto_sans_11);

  auto* tree_view = new QTreeView();
  tree_view->setMinimumSize(300, 400);

  connection_status_pb_ = new QPushButton();
  connection_status_pb_->setPalette(Qt::GlobalColor::red);
  connection_status_pb_->setText(tr("Подключение к серверу..."));

  auto* settings_pb = new QPushButton(tr("Изменить\nсетевые настройки"));
  settings_pb->setMinimumSize(160, 70);
  settings_widget_ = new net_settings_dialog(this);

  auto* send_request_pb = new QPushButton(tr("Запрос конфигурации\nвсех устройств"));
  output_te_ = new QTextEdit(this);

  auto* grid = new QGridLayout(this);
  grid->addWidget(tree_view,             0, 0,   8, 2);
  grid->addWidget(connection_status_pb_, 0, 2,   1, 1, Qt::AlignTop);
  grid->addWidget(settings_pb,           0, 3,   1, 1, Qt::AlignTop);
  grid->addWidget(output_te_,            1, 2,   6, 2);
  grid->addWidget(send_request_pb,       7, 2,   1, 2);
  setLayout(grid);

  slot_client_restart();

  connect(settings_pb, &QPushButton::clicked, [this]() {
    settings_widget_->show();
  });

  connect(send_request_pb, &QPushButton::clicked, [this]() {
    assert(tcp_client_);

    QByteArray req;
    // TODO: Позже - сократить namespace (т.к. класс войдет в него):
    req.append(static_cast<char>(xml_loader::messages::get_all_devices_description));
    if (tcp_client_->send_to_server(req) == -1) {
      output_te_->append("tcp_smart_client: Data sending failed");
    }
  });
}


widget::~widget() {
}


void widget::slot_client_restart() {
  tcp_client_ = std::make_unique<tcp_smart_client>(QHostAddress{settings_widget_->ip()},
                                                   settings_widget_->port());

  connect(tcp_client_.get(), &tcp_smart_client::connected, [this]() {
    connection_status_pb_->setPalette(Qt::GlobalColor::green);
    connection_status_pb_->setText(tr("Соединение установлено"));
//    qDebug() << "connected with port" << QString::number(settings_widget_->port());
  });

  connect(tcp_client_.get(), &tcp_smart_client::disconnected, [this]() {
    connection_status_pb_->setPalette(Qt::GlobalColor::red);
    connection_status_pb_->setText(tr("Подключение к серверу..."));
  });

  connect(tcp_client_.get(), &tcp_smart_client::connection_error, [this](QString info) {
    output_te_->append("tcp_smart_client: " + info);
  });

  settings_widget_->hide();
}

}
