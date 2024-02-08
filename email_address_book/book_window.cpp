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
    QStandardItem* item1 =
        new QStandardItem(db_row[database::QUERY_COLUMN::FIRSTNAME] + " " +
                          db_row[database::QUERY_COLUMN::SURNAME]);
    QStandardItem* item2 =
        new QStandardItem(db_row[database::QUERY_COLUMN::EMAIL]);
    QList<QStandardItem*> cur_row;
    cur_row.append(item1);
    cur_row.append(item2);
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
  } else if (!mail.isEmpty() && firstname.isEmpty() && surname.isEmpty()) {
    search_results = db.find_by_email(mail);
  } else if (!firstname.isEmpty() && !surname.isEmpty()) {
    search_results = db.find_by_full_name(firstname, surname);
  } else if (surname.isEmpty()) {
    search_results = db.find_by_one_name(firstname, true);
  } else if (firstname.isEmpty()) {
    search_results = db.find_by_one_name(surname, false);
  } else {
    return;
  }

  append_query_to_table(search_results);
}

void book_window::insert() {
  if (ui->first_name->text().isEmpty() || ui->second_name->text().isEmpty() ||
      ui->email->text().isEmpty()) {
    output_error("Not enough arguments for insertion provided");
    return;
  }
  QString firstname{ui->first_name->text().toLower()};
  QString surname{ui->second_name->text().toLower()};
  QString email{ui->email->text().toLower()};
  if (!db.insert_into_db(firstname, surname, email)) {
    output_error("Insertion failed");
  }
}

void book_window::remove() {
  auto selected_indexes{ui->address_list->selectionModel()->selectedIndexes()};

  if (selected_indexes.size() == 0) {
    output_error("No row chosen, hence nothing to remove");
    return;
  }

  // if all rows are selected then the whole person entry can be deleted as well
  bool person_removed{
      static_cast<int>(
          ui->address_list->selectionModel()->selectedIndexes().size()) ==
      this->model->rowCount()};

  /*
   *  Because the id in person is autoincrement there will be a hole between the
   * position before the removed one and the the following persons. Yet, I find
   * it better to still remove the person instead of keeping a zombie there. As
   * there can be a max of MAX LONG aka 2^64 - 1 ids it is very unlikely that
   * this point will ever be reached, even if there are multiple persons removed
   * and added again.
   */
  if (person_removed) {
    QString email{
        this->model->index(selected_indexes.first().row(), ADDRESS_LIST::EMAIL)
            .data()
            .toString()};
    db.remove_person_by_email(email);
    clear_model();
    return;
  }

  QString email_for_id_finding{
      this->model->index(selected_indexes.first().row(), ADDRESS_LIST::EMAIL)
          .data()
          .toString()};

  std::optional<QString> option_id{db.get_id_by_email(email_for_id_finding)};
  if (!option_id.has_value()) {
    output_error("Remove failed");
    return;
  }

  QString id{option_id.value()};

  for (const auto index :
       ui->address_list->selectionModel()->selectedIndexes()) {
    QString email{
        this->model->index(index.row(), ADDRESS_LIST::EMAIL).data().toString()};
    if (!db.remove_by_email(email)) {
      output_error("Remove of " + email + " has failed");
      return;
    }
  }

  auto search_results{db.find_by_id(id)};
  clear_model();
  this->append_query_to_table(search_results);
}

book_window::book_window(QWidget* parent, QString db_path)
    : QMainWindow(parent), ui(new Ui::book_window), db(database(db_path)) {
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
