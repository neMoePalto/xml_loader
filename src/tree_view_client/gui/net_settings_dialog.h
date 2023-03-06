#pragma once

#include <cstdint>
#include <memory>

#include <QDialog>
#include <QString>


class QLineEdit;
class QSettings;


namespace xml_loader {


class widget;

class net_settings_dialog : public QDialog {
  Q_OBJECT

public:
  net_settings_dialog(widget* parent);
  ~net_settings_dialog();
  std::uint16_t port() const noexcept;
  QString ip() const noexcept;

private:
  void load_settings(QLineEdit& le_ip, QLineEdit& le_port);

private:
  QLineEdit* ip_le_;
  QLineEdit* port_le_;

  std::unique_ptr<QSettings> settings_;
};

}
