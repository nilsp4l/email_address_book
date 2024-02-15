#ifndef BOOK_WINDOW_H
#define BOOK_WINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>

#include "query.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class book_window;
}
QT_END_NAMESPACE

class book_window : public QMainWindow {
  Q_OBJECT

 public slots:
  void search();
  void insert();
  void remove();

 public:
  book_window(QWidget *parent = nullptr, QString db_path = "database/maildb");
  ~book_window();

 private:
  enum ADDRESS_LIST { NAME = 0, EMAIL, SIZE };

  void append_query_to_table(
      const std::vector<std::array<QString, database::QUERY_COLUMN::SIZE>>
          &search_results);
  void clear_model();
  void clear_error_label();
  void output_error(QString error_msg);
  void init_address_list();
  void init_model();
  Ui::book_window *ui;
  QStandardItemModel *model;
  query query;
};
#endif  // BOOK_WINDOW_H
