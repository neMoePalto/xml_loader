#pragma once

#include <memory>

#include <QWidget>


class QPushButton;
class QTextEdit;


namespace xml_loader {


class net_settings_dialog;
class tcp_smart_client;

class widget : public QWidget {
  Q_OBJECT

public:
  widget(QWidget* parent = nullptr);
  ~widget();
  void slot_client_restart();

private:
  std::unique_ptr<tcp_smart_client> tcp_client_;
  net_settings_dialog*              settings_widget_;
  QPushButton*                      connection_status_pb_;
  QTextEdit*                        output_te_;
};

}
