#include <QApplication>
#include <iostream>

#include "book_window.h"

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  auto args{QApplication::arguments()};
  if (QApplication::arguments().size() <= 1) {
    std::cerr << "Usage: " << args.first().toStdString()
              << " <path to database>" << std::endl;
    return 1;
  }

  QString path{*(args.begin() + 1)};

  book_window w(nullptr, path);
  w.setWindowTitle("Email Address Book");
  w.show();
  return a.exec();
}
