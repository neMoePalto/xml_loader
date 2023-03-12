#pragma once

#include <memory>

#include <QByteArray>
#include <QList>

#include <QWidget>


class QPushButton;
class QTextEdit;
class QTreeWidget;


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
  void update_tree_widget(const QList<QByteArray>& rows);
  void change_connection_status(Qt::GlobalColor status, std::uint16_t server_port);

private:
  std::unique_ptr<tcp_smart_client> tcp_client_;
  net_settings_dialog*              settings_widget_;
  QTreeWidget*                      tree_widget_;
  QPushButton*                      connection_status_pb_;
  QTextEdit*                        output_te_;
};

}
