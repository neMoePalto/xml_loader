#include <QCoreApplication>

#include "app_complete.h"
#include "data_server.h"


int main(int argc, char *argv[]) {
  QCoreApplication a(argc, argv);

  xml_loader::data_server server;
  app_complete completing_by_signals;

  return a.exec();
}
