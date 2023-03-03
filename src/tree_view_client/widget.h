#pragma once

#include <QWidget>


class widget : public QWidget {
  Q_OBJECT

public:
  widget(QWidget *parent = nullptr);
  ~widget();
};
