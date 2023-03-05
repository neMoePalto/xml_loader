#include "app_complete.h"

#include <sys/signal.h>

#include <QCoreApplication>
#include <QDebug>


app_complete::app_complete() {
  auto complete = [](const int signal_type) {
    qDebug() << "\n" << strsignal(signal_type) << "\n Завершаю приложение";
    QCoreApplication::exit(0);
  };

  signal(SIGABRT, complete);
  signal(SIGINT,  complete);
  signal(SIGTERM, complete);
}
