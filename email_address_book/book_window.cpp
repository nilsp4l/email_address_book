#include "book_window.h"

#include "./ui_book_window.h"

void book_window::init_address_list() {
  init_model();
  ui->address_list->horizontalHeader()->setSectionResizeMode(
      QHeaderView::Stretch);
  ui->address_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->address_list->setModel(this->model);
}

void book_window::init_model() {
  this->model = new QStandardItemModel(0, ADDRESS_LIST::SIZE);
  model->setHeaderData(ADDRESS_LIST::NAME, Qt::Horizontal, QObject::tr("Name"));
  model->setHeaderData(ADDRESS_LIST::EMAIL, Qt::Horizontal,
                       QObject::tr("Email Address"));
}

void book_window::append_query_to_table(
    const std::vector<std::array<QString, database::QUERY_COLUMN::SIZE>>&
        search_results) {
  for (const auto& db_row : search_results) {
    QStandardItem* name_item =
        new QStandardItem(db_row[database::QUERY_COLUMN::FIRSTNAME] + " " +
                          db_row[database::QUERY_COLUMN::SURNAME]);
    QStandardItem* email_item =
        new QStandardItem(db_row[database::QUERY_COLUMN::EMAIL]);
    QList<QStandardItem*> cur_row;
    cur_row.append(name_item);
    cur_row.append(email_item);
    this->model->appendRow(cur_row);
  }
}

void book_window::clear_error_label() { ui->error_label->setText(""); }

void book_window::output_error(QString error_msg) {
  ui->error_label->setText(error_msg);
}

void book_window::clear_model() {
  for (int i{this->model->rowCount()}; i >= 0; --i) {
    model->removeRow(i);
  }
}

void book_window::search() {
  clear_model();
  clear_error_label();
  std::vector<std::array<QString, database::QUERY_COLUMN::SIZE>> search_results;
  QString firstname{ui->first_name->text().toLower()};
  QString surname{ui->second_name->text().toLower()};
  QString mail{ui->email->text()};

  if (mail.isEmpty() && firstname.isEmpty() && surname.isEmpty()) {
    output_error("No arguments provided");
  }

  search_results = query.execute_search_query(firstname, surname, mail);
  append_query_to_table(search_results);
}

void book_window::insert() {
  clear_error_label();
  if (ui->first_name->text().isEmpty() || ui->second_name->text().isEmpty() ||
      ui->email->text().isEmpty()) {
    output_error("Not enough arguments for insertion provided");
    return;
  }
  QString firstname{ui->first_name->text().toLower()};
  QString surname{ui->second_name->text().toLower()};
  QString email{ui->email->text().toLower()};
  if (!query.execute_insertion_query(firstname, surname, email)) {
    output_error("Insertion failed");
  } else {
    clear_model();
    append_query_to_table(query.refresh_query());
  }
}

void book_window::remove() {
  clear_error_label();

  auto selected_indexes{ui->address_list->selectionModel()->selectedIndexes()};

  if (selected_indexes.size() == 0) {
    output_error("No row chosen, hence nothing to remove");
    return;
  }

  for (const auto index :
       ui->address_list->selectionModel()->selectedIndexes()) {
    QString email{
        this->model->index(index.row(), ADDRESS_LIST::EMAIL).data().toString()};
    if (!query.execute_remove_query(email)) {
      output_error("Remove of " + email + " has failed");
    }
  }

  clear_model();
  // a query must have been executed to even get to this point, hence even
  // though last_query of query was not updated here, it is
  this->append_query_to_table(query.refresh_query());
}

book_window::book_window(QWidget* parent, QString db_path)
    : QMainWindow(parent), ui(new Ui::book_window), query(db_path) {
  ui->setupUi(this);

  init_address_list();
  connect(ui->search_button, SIGNAL(clicked()), this, SLOT(search()));
  connect(ui->insert_button, SIGNAL(clicked()), this, SLOT(insert()));
  connect(ui->remove_button, SIGNAL(clicked()), this, SLOT(remove()));
}

book_window::~book_window() {
  delete ui;
  delete model;
}
