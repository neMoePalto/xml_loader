#include "net_settings_dialog.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>

#include "widget.h"


namespace xml_loader {


net_settings_dialog::net_settings_dialog(widget* parent)
  : QDialog(parent) { // Для центровки данного виджета и его автоматического
                      // удаления через иерархию QObject
  setModal(true);
  setMaximumSize(275, 160);
  setFont(parent->font());

  auto* common_label = new QLabel(tr("Параметры сервера:"));
  auto* ip_label     = new QLabel(tr("IP-адрес:"));
  auto* port_label   = new QLabel(tr("Tcp-порт:"));

  ip_le_ = new QLineEdit();
  ip_le_->setFixedWidth(180);
  port_le_ = new QLineEdit();
  port_le_->setFixedWidth(180);
  load_settings(*ip_le_, *port_le_);

  auto* apply_settings_pb = new QPushButton(tr("Применить настройки"));

  auto* grid = new QGridLayout(this);
  grid->addWidget(common_label,      0, 0,   1, 2);
  grid->addWidget(ip_label,          1, 0,   1, 1);
  grid->addWidget(ip_le_,            1, 1,   1, 1);
  grid->addWidget(port_label,        2, 0,   1, 1);
  grid->addWidget(port_le_,          2, 1,   1, 1);
  grid->addWidget(apply_settings_pb, 3, 1,   1, 1);
  setLayout(grid);

  Q_ASSERT(parent);
  connect(apply_settings_pb, &QPushButton::clicked, parent, &widget::slot_client_restart);
}


net_settings_dialog::~net_settings_dialog() {
  settings_->setValue("IP",   ip_le_->text());
  settings_->setValue("port", port_le_->text());
}


std::uint16_t net_settings_dialog::port() const noexcept {
  return port_le_->text().toUShort();
}


QString net_settings_dialog::ip() const noexcept {
  return ip_le_->text();
}


void net_settings_dialog::load_settings(QLineEdit &le_ip, QLineEdit &le_port) {
  settings_ = std::make_unique<QSettings>("../cli_settings.ini", QSettings::IniFormat);

  le_ip.setText(settings_->value("IP").toString());
  le_port.setText(settings_->value("port").toString());
}

}
