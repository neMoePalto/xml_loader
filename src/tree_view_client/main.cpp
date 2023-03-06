#include <QApplication>

#include "gui/widget.h"


int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  xml_loader::widget w;
  w.show();

  return a.exec();
}
