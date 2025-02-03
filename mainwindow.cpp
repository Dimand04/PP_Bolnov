#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "global.h"
#include "QDebug"
#include <QDate>
#include <QFileDialog>
#include <QBuffer>
#include "doc.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect (ui->qpb_tologin,&QPushButton::clicked,this,&MainWindow::showAuthMenu);
    connect (ui->qpb_toregister,&QPushButton::clicked,this,&MainWindow::showRegMenu);
    connect (ui->qpb_login,&QPushButton::clicked,this,&MainWindow::tryLogin);
    connect (ui->qpb_register,&QPushButton::clicked,this,&MainWindow::tryReg);
    connect (ui->qpb_logout,&QPushButton::clicked,this,&MainWindow::logout);
    connect (ui->qpb_p_change,&QPushButton::clicked,this,&MainWindow::lereadonly);
    connect (ui->qpb_p_change_ok,&QPushButton::clicked,this,&MainWindow::updateUserInfo);
    connect (ui->cb_client, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applyFiltersAndSort);
    connect (ui->qpb_sort_clear, &QPushButton::clicked, this, &MainWindow::clearFilter);
    connect (ui->cb_sort, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applyFiltersAndSort);
    connect (ui->rb_up, &QRadioButton::toggled, this, &MainWindow::applyFiltersAndSort);
    connect (ui->rb_down, &QRadioButton::toggled, this, &MainWindow::applyFiltersAndSort);
    connect (ui->cb_sort_search, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applyFiltersAndSort);
    connect (ui->le_search, &QLineEdit::textChanged, this, &MainWindow::applyFiltersAndSort);
    connect(ui->tableWidget, &QTableWidget::cellClicked, this, &MainWindow::TableWidgetRowClicked);
    connect (ui->qpb_updateStatus,&QPushButton::clicked,this,&MainWindow::updateOrderStatus);
    connect (ui->label_8,&QPushLabel::clicked,this,&MainWindow::selectDoc);
    connect (ui->qpb_addOrder,&QPushButton::clicked,this,&MainWindow::addNewOrder);

    ui->rb_up->setChecked(true);

    ui->tableWidget->setColumnWidth(0,30);
    ui->tableWidget->setColumnWidth(1,180);
    ui->tableWidget->setColumnWidth(2,100);
    ui->tableWidget->setColumnWidth(3,180);
    ui->tableWidget->setColumnWidth(4,110);
    ui->tableWidget->setColumnWidth(5,86);
    ui->tableWidget->setColumnWidth(6,70);

    loadFaceName();
    loadUser();
    loadSortOptions();
    loadSearchOptions();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showAuthMenu()
{
    ui->login_frame->setGeometry(ui->login_frame->x(), 0, ui->login_frame->width(), ui->login_frame->height());
    ui->register_frame->setGeometry(ui->register_frame->x(), 1000, ui->register_frame->width(), ui->register_frame->height());
    ui->main_frame->setGeometry(ui->main_frame->x(), 1000, ui->main_frame->width(), ui->main_frame->height());
    ui->login_frame->setEnabled(true);
    ui->register_frame->setEnabled(false);
    ui->main_frame->setEnabled(false);
    ui->le_name->clear();
    ui->le_surname->clear();
    ui->le_patronymic->clear();
    ui->le_phone->clear();
    ui->le_email->clear();
    ui->le_inn->clear();
    ui->le_reg_password->clear();
}

void MainWindow::showRegMenu()
{
    ui->login_frame->setGeometry(ui->login_frame->x(), 1000, ui->login_frame->width(), ui->login_frame->height());
    ui->register_frame->setGeometry(ui->register_frame->x(), 0, ui->register_frame->width(), ui->register_frame->height());
    ui->login_frame->setEnabled(false);
    ui->register_frame->setEnabled(true);
    ui->le_login->clear();
    ui->le_password->clear();
}

void MainWindow::showMainMenu()
{
    ui->login_frame->setGeometry(ui->login_frame->x(), 1000, ui->login_frame->width(), ui->login_frame->height());
    ui->register_frame->setGeometry(ui->register_frame->x(), 1000, ui->register_frame->width(), ui->register_frame->height());
    ui->main_frame->setGeometry(ui->main_frame->x(), 0, ui->main_frame->width(), ui->main_frame->height());
    ui->login_frame->setEnabled(false);
    ui->register_frame->setEnabled(false);
    ui->main_frame->setEnabled(true);
    ui->le_p_name->setReadOnly(true);
    ui->le_p_surname->setReadOnly(true);
    ui->le_p_patronymic->setReadOnly(true);
    ui->le_p_phone->setReadOnly(true);
    ui->le_p_email->setReadOnly(true);
    ui->le_p_inn->setReadOnly(true);
    ui->tabWidget->setCurrentIndex(0);
    getUserInfo(userID);
    loadOrderTable();
}

void MainWindow::tryLogin()
{
    QString login = ui->le_login->text().remove(" ");
    QString password = ui->le_password->text().remove(" ");

    if (login.isEmpty() && password.isEmpty())
    {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, заполните поля 'Логин' и 'Пароль'.");
        return;
    }
    else if (login.isEmpty())
    {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, заполните поле 'Логин'.");
        return;
    }
    else if (password.isEmpty())
    {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, заполните поле 'Пароль'.");
        return;
    }

    db = QSqlDatabase::database("bd_bolnov");
    if(getDBConnection(db))
    {
        QSqlQuery query(db);
        query.prepare("SELECT id, user_role FROM user INNER JOIN user_authorization ON user.id = user_authorization.user_id WHERE user.user_phone = ? AND user_authorization.user_password = ?;");
        query.addBindValue(login);
        query.addBindValue(password);
        query.exec();
        if(query.next())
        {
            QList<QMessageBox::Button> buttons = {QMessageBox::Ok};
            showMessage("Успешная авторизация!", "", QMessageBox::Information, buttons);
            userID = query.value(0).toString();
            userRole = query.value(1).toString();
            ui->le_login->clear();
            ui->le_password->clear();
            showMainMenu();
            adminFunction();
        }
        else
        {
            QMessageBox::warning(this, "Ошибка", "Пользователя с такими данными не существует!");
            ui->le_login->clear();
            ui->le_password->clear();
            return;
        }
    }
}

void MainWindow::tryReg()
{
    int selectedFaceId = ui->cb_face->currentData().toInt();
    db = QSqlDatabase::database("bd_bolnov");
    if(getDBConnection(db))
    {
        QSqlQuery query(db);
        query.prepare("INSERT INTO user (user_name, user_surname, user_patronymic, user_phone, user_email, user_inn, user_role, user_face_id) VALUES (?, ?, ?, ?, ?, ?, ?, ?);");
        query.addBindValue(ui->le_name->text());
        query.addBindValue(ui->le_surname->text());
        query.addBindValue(ui->le_patronymic->text());
        query.addBindValue(ui->le_phone->text());
        query.addBindValue(ui->le_email->text());
        query.addBindValue(ui->le_inn->text());
        query.addBindValue("user");
        query.addBindValue(selectedFaceId);
        if(query.exec())
        {
            query.exec("SELECT MAX(id) FROM user;");
            if(query.next())
            {
                userID = query.value(0).toString();
                query.prepare("INSERT INTO user_authorization (user_id, user_password) VALUES (?, ?);");
                query.addBindValue(userID);
                query.addBindValue(ui->le_reg_password->text());
                if(query.exec())
                {
                    QList<QMessageBox::Button> buttons = {QMessageBox::Ok};
                    showMessage("Успешная регистрация!", "", QMessageBox::Information, buttons);
                    ui->le_name->clear();
                    ui->le_surname->clear();
                    ui->le_patronymic->clear();
                    ui->le_phone->clear();
                    ui->le_email->clear();
                    ui->le_inn->clear();
                    ui->le_reg_password->clear();
                    showMainMenu();
                    adminFunction();
                }
            }
        }
    }
}

void MainWindow::loadFaceName()
{
    if(getDBConnection(db))
    {
        QSqlQuery query(db);
        ui->cb_face->clear();
        query.prepare("SELECT id, face_name FROM user_face");
        if(query.exec())
        {
            while (query.next()) {
                int faceId = query.value(0).toInt();
                QString faceName = query.value(1).toString();
                ui->cb_face->addItem(faceName, faceId);
            }
        }
    }
}

void MainWindow::logout()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение выхода", "Вы действительно хотите выйти из аккаунта?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        userID.clear();
        userRole.clear();
        ui->orderDateLabel->clear();
        ui->userNameLabel->clear();
        ui->userPhoneLabel->clear();
        ui->userEmailLabel->clear();
        ui->userInnLabel->clear();
        ui->userFaceLabel->clear();
        ui->pte_orderText->clear();
        ui->cb_orderStatus->clear();
        ui->plainTextEdit->clear();
        ui->label_8->clear();
        showAuthMenu();
    }
}

void MainWindow::getUserInfo(QString ID)
{
    db = QSqlDatabase::database("bd_bolnov");
    if(getDBConnection(db))
    {
        QSqlQuery query(db);
        query.prepare("select user_name, user_surname, user_patronymic, user_phone, user_email, user_inn from user where id = ?;");
        query.addBindValue(ID);
        query.exec();
        if(query.next())
        {
            ui->le_p_name->setText(query.value(0).toString());
            ui->le_p_surname->setText(query.value(1).toString());
            ui->le_p_patronymic->setText(query.value(2).toString());
            ui->le_p_phone->setText(query.value(3).toString());
            ui->le_p_email->setText(query.value(4).toString());
            ui->le_p_inn->setText(query.value(5).toString());
        }
    }
}

void MainWindow::lereadonly()
{
    bool isReadOnly = ui->le_p_name->isReadOnly();
    if (!isReadOnly) {
        getUserInfo(userID);
    }
    ui->le_p_name->setReadOnly(!isReadOnly);
    ui->le_p_surname->setReadOnly(!isReadOnly);
    ui->le_p_patronymic->setReadOnly(!isReadOnly);
    ui->le_p_phone->setReadOnly(!isReadOnly);
    ui->le_p_email->setReadOnly(!isReadOnly);
    ui->le_p_inn->setReadOnly(!isReadOnly);
    if (isReadOnly) {
        ui->qpb_p_change->setText("Отмена");
    } else {
        ui->qpb_p_change->setText("Изменить");
    }
}

void MainWindow::updateUserInfo()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение изменений", "Вы действительно хотите изменить данные?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        if (ui->le_p_name->isReadOnly()) return;
        db = QSqlDatabase::database("bd_bolnov");
        if(getDBConnection(db))
        {
            QSqlQuery query(db);
            query.prepare("UPDATE user SET user_name = ?, user_surname = ?, user_patronymic = ?, user_phone = ?, user_email = ?, user_inn = ? WHERE id = ?");
            query.addBindValue(ui->le_p_name->text());
            query.addBindValue(ui->le_p_surname->text());
            query.addBindValue(ui->le_p_patronymic->text());
            query.addBindValue(ui->le_p_phone->text());
            query.addBindValue(ui->le_p_email->text());
            query.addBindValue(ui->le_p_inn->text());
            query.addBindValue(userID);
            if(query.exec())
            {
                QList<QMessageBox::Button> buttons = {QMessageBox::Ok};
                showMessage("Данные обновлены!", "", QMessageBox::Information, buttons);
                lereadonly();
                getUserInfo(userID);
            }
        }
    }
}

void MainWindow::loadOrderTable()
{
    db = QSqlDatabase::database("bd_bolnov");
    if(getDBConnection(db))
    {
        int row = 0;
        QSqlQuery query(db);
        if(hasAccess("user"))
        {
            query.prepare("SELECT orders.id, user.user_surname, user.user_name, user.user_patronymic, user.user_phone, user.user_email, user.user_inn, orders.order_date, status.name "
                          "FROM orders "
                          "JOIN user ON orders.user_id = user.id "
                          "JOIN status ON orders.status_id = status.id "
                          "WHERE orders.user_id = :userID");
            query.bindValue(":userID", userID);
        }
        else
        {
            query.prepare("SELECT orders.id, user.user_surname, user.user_name, user.user_patronymic, user.user_phone, user.user_email, user.user_inn, orders.order_date, status.name "
                          "FROM orders "
                          "JOIN user ON orders.user_id = user.id "
                          "JOIN status ON orders.status_id = status.id");
        }

        if(query.exec())
        {
            ui->tableWidget->setRowCount(0);
            while(query.next())
            {
                ui->tableWidget->insertRow(row);
                int orderId = query.value(0).toInt();
                QString surname = query.value(1).toString();
                QString name = query.value(2).toString();
                QString patronymic = query.value(3).toString();
                QString phone = query.value(4).toString();
                QString email = query.value(5).toString();
                QString inn = query.value(6).toString();
                QString orderDate = query.value(7).toString();
                QString orderStatus = query.value(8).toString();
                QString fullname = surname + " " + name + " " + patronymic;

                QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(orderId));
                QTableWidgetItem *nameItem = new QTableWidgetItem(fullname);
                QTableWidgetItem *phoneItem = new QTableWidgetItem(phone);
                QTableWidgetItem *emailItem = new QTableWidgetItem(email);
                QTableWidgetItem *innItem = new QTableWidgetItem(inn);
                QTableWidgetItem *dateItem = new QTableWidgetItem(orderDate);
                QTableWidgetItem *statusItem = new QTableWidgetItem(orderStatus);

                ui->tableWidget->setItem(row, 0, idItem);
                ui->tableWidget->setItem(row, 1, nameItem);
                ui->tableWidget->setItem(row, 2, phoneItem);
                ui->tableWidget->setItem(row, 3, emailItem);
                ui->tableWidget->setItem(row, 4, innItem);
                ui->tableWidget->setItem(row, 5, dateItem);
                ui->tableWidget->setItem(row, 6, statusItem);
                row++;
            }
        }
    }
}

void MainWindow::loadUser()
{
    if (getDBConnection(db))
    {
        QSqlQuery query(db);
        ui->cb_client->clear();
        ui->cb_client->setPlaceholderText("Пользователь");
        ui->cb_client->addItem("");
        query.prepare("SELECT user_name, user_surname, user_patronymic FROM user WHERE user_role != 'admin'");
        if (query.exec())
        {
            while (query.next()) {
                QString fullName = query.value(0).toString() + " " + query.value(1).toString() + " " + query.value(2).toString();
                ui->cb_client->addItem(fullName);
            }
        }
        connect(ui->cb_client, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updatePlaceholder);
    }
}

void MainWindow::updatePlaceholder()
{
    if (ui->cb_client->currentIndex() == 0)
    {
        loadUser();
    }
    else if (ui->cb_sort->currentIndex() == 0)
    {
        loadSortOptions();
    }
    else if (ui->cb_sort_search->currentIndex() == 0)
    {
        loadSearchOptions();
    }
}

void MainWindow::clearFilter()
{
    loadUser();
    loadSortOptions();
    loadSearchOptions();
    ui->le_search->clear();
    ui->rb_up->setChecked(true);
}

void MainWindow::loadSortOptions()
{
    ui->cb_sort->clear();
    ui->cb_sort->setPlaceholderText("Сортировка");
    ui->cb_sort->addItem("");
    int columnCount = ui->tableWidget->columnCount();
    for (int i = 0; i < columnCount; ++i)
    {
        QString columnName = ui->tableWidget->horizontalHeaderItem(i)->text();
        ui->cb_sort->addItem(columnName);
    }
    connect(ui->cb_sort, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updatePlaceholder);
}

void MainWindow::loadSearchOptions()
{
    ui->cb_sort_search->clear();
    ui->cb_sort_search->setPlaceholderText("Сортировка");
    ui->cb_sort_search->addItem("");
    ui->cb_sort_search->addItem("Номер заявки");
    ui->cb_sort_search->addItem("ФИО");
    ui->cb_sort_search->addItem("Телефон");
    ui->cb_sort_search->addItem("Электронная почта");
    ui->cb_sort_search->addItem("ИНН");
    ui->cb_sort_search->addItem("Дата заявки");
    ui->cb_sort_search->addItem("Статус заказа");
    connect(ui->cb_sort_search, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::updatePlaceholder);
}

void MainWindow::applyFiltersAndSort()
{
    ui->tableWidget->setRowCount(0);

    QString selectedClient = ui->cb_client->currentText();
    QString searchOption = ui->cb_sort_search->currentText();
    QString searchText = ui->le_search->text();
    QString selectedColumn = ui->cb_sort->currentText();
    bool isAscending = ui->rb_up->isChecked();

    QString queryStr = "SELECT orders.id, user.user_name, user.user_surname, user.user_patronymic, user.user_phone, user.user_email, user.user_inn, orders.order_date, status.name "
                       "FROM orders "
                       "INNER JOIN user ON orders.user_id = user.id "
                       "INNER JOIN status ON orders.status_id = status.id WHERE 1=1 ";

    if (!selectedClient.isEmpty()) {
        queryStr += "AND CONCAT(user.user_name, ' ', user.user_surname, ' ', user.user_patronymic) = :selectedClient ";
    }

    if (!searchOption.isEmpty() && !searchText.isEmpty()) {
        if (searchOption == "Номер заявки") {
            queryStr += "AND orders.id = :searchText ";
        } else if (searchOption == "ФИО") {
            queryStr += "AND CONCAT(user.user_name, ' ', user.user_surname, ' ', user.user_patronymic) LIKE :searchText ";
        } else if (searchOption == "Телефон") {
            queryStr += "AND user.user_phone LIKE :searchText ";
        } else if (searchOption == "Электронная почта") {
            queryStr += "AND user.user_email LIKE :searchText ";
        } else if (searchOption == "ИНН") {
            queryStr += "AND user.user_inn LIKE :searchText ";
        } else if (searchOption == "Дата заявки") {
            queryStr += "AND orders.order_date LIKE :searchText ";
        } else if (searchOption == "Статус заказа") {
            queryStr += "AND status.name LIKE :searchText ";
        }
    }

    if (getDBConnection(db)) {
        QSqlQuery query(db);
        query.prepare(queryStr);

        if (!selectedClient.isEmpty()) {
            query.bindValue(":selectedClient", selectedClient);
        }

        if (!searchOption.isEmpty() && !searchText.isEmpty()) {
            if (searchOption == "Номер заявки") {
                query.bindValue(":searchText", searchText.toInt());
            } else {
                query.bindValue(":searchText", "%" + searchText + "%");
            }
        }

        query.exec();

        int row = 0;
        while (query.next())
        {
            QString orderId = query.value(0).toString();
            QString firstName = query.value(1).toString();
            QString lastName = query.value(2).toString();
            QString patronymic = query.value(3).toString();
            QString phone = query.value(4).toString();
            QString email = query.value(5).toString();
            QString inn = query.value(6).toString();
            QString orderDate = query.value(7).toString();
            QString orderStatus = query.value(8).toString();

            ui->tableWidget->insertRow(row);
            ui->tableWidget->setItem(row, 0, new QTableWidgetItem(orderId));
            ui->tableWidget->setItem(row, 1, new QTableWidgetItem(firstName + " " + lastName + " " + patronymic));
            ui->tableWidget->setItem(row, 2, new QTableWidgetItem(phone));
            ui->tableWidget->setItem(row, 3, new QTableWidgetItem(email));
            ui->tableWidget->setItem(row, 4, new QTableWidgetItem(inn));
            ui->tableWidget->setItem(row, 5, new QTableWidgetItem(orderDate));
            ui->tableWidget->setItem(row, 6, new QTableWidgetItem(orderStatus));

            row++;
        }

        if (!selectedColumn.isEmpty()) {
            int columnIndex = -1;
            int columnCount = ui->tableWidget->columnCount();
            for (int i = 0; i < columnCount; ++i) {
                QString columnName = ui->tableWidget->horizontalHeaderItem(i)->text();
                if (columnName == selectedColumn) {
                    columnIndex = i;
                    break;
                }
            }

            if (columnIndex != -1) {
                Qt::SortOrder order = isAscending ? Qt::AscendingOrder : Qt::DescendingOrder;
                ui->tableWidget->sortItems(columnIndex, order);
            }
        }
    }
}

void MainWindow::TableWidgetRowClicked(int row, int column)
{
    QTableWidgetItem *item = ui->tableWidget->item(row, 0);
    if (item)
    {
        int orderId = item->text().toInt();
        loadOrderDetails(orderId);

    }
}

void MainWindow::loadOrderDetails(int orderId)
{
    ui->orderDateLabel->clear();
    ui->userNameLabel->clear();
    ui->userPhoneLabel->clear();
    ui->userEmailLabel->clear();
    ui->userInnLabel->clear();
    ui->userFaceLabel->clear();
    ui->pte_orderText->clear();
    ui->cb_orderStatus->clear();
    currentOrderId = orderId;
    disconnect(ui->qpb_doc, &QPushButton::clicked, nullptr, nullptr);
    db = QSqlDatabase::database("bd_bolnov");
    if(getDBConnection(db))
    {
        QSqlQuery query(db);
        query.prepare("SELECT orders.order_date, user.user_name, user.user_surname, user.user_patronymic, user.user_phone, user.user_email, user.user_inn, user_face.face_name, orders.order_text "
                      "FROM orders "
                      "INNER JOIN user ON orders.user_id = user.id "
                      "INNER JOIN user_face ON user.user_face_id = user_face.id "
                      "WHERE orders.id = :orderID");
        query.bindValue(":orderID", orderId);
        if(query.exec())
        {
            while(query.next())
            {
                QString orderDate = query.value(0).toString();
                QString userName = query.value(1).toString();
                QString userSurname = query.value(2).toString();
                QString userPatronymic = query.value(3).toString();
                QString userPhone = query.value(4).toString();
                QString userEmail = query.value(5).toString();
                QString userInn = query.value(6).toString();
                QString faceName = query.value(7).toString();
                QString orderText = query.value(8).toString();
                QString fullname = userSurname + " " + userName + " " + userPatronymic;

                ui->orderDateLabel->setText(orderDate);
                ui->userNameLabel->setText(fullname);
                ui->userPhoneLabel->setText(userPhone);
                ui->userEmailLabel->setText(userEmail);
                ui->userInnLabel->setText(userInn);
                ui->userFaceLabel->setText(faceName);
                ui->pte_orderText->setPlainText(orderText);
            }
        }
        QSqlQuery statusQuery(db);
        statusQuery.prepare("SELECT name FROM status");

        if(statusQuery.exec())
        {
            while(statusQuery.next())
            {
                QString statusName = statusQuery.value(0).toString();
                ui->cb_orderStatus->addItem(statusName);
            }
        }
        QSqlQuery currentStatusQuery(db);
        currentStatusQuery.prepare("SELECT status.name FROM orders "
                                   "INNER JOIN status ON orders.status_id = status.id "
                                   "WHERE orders.id = :orderID");
        currentStatusQuery.bindValue(":orderID", orderId);

        if(currentStatusQuery.exec())
        {
            if(currentStatusQuery.next())
            {
                QString currentStatusName = currentStatusQuery.value(0).toString();
                int index = ui->cb_orderStatus->findText(currentStatusName);
                if (index != -1)
                {
                    ui->cb_orderStatus->setCurrentIndex(index);
                }
            }
        }
        connect(ui->qpb_doc, &QPushButton::clicked, this, [this, orderId]() { loadDoc(orderId); });
    }
}

void MainWindow::updateOrderStatus()
{
    if(getDBConnection(db))
    {
        QString selectedStatus = ui->cb_orderStatus->currentText();

        QSqlQuery statusIdQuery(db);
        statusIdQuery.prepare("SELECT id FROM status WHERE name = :statusName");
        statusIdQuery.bindValue(":statusName", selectedStatus);
        int statusId = -1;
        if (statusIdQuery.exec() && statusIdQuery.next())
        {
            statusId = statusIdQuery.value(0).toInt();
        }

        QSqlQuery updateQuery(db);
        updateQuery.prepare("UPDATE orders SET status_id = :statusId WHERE id = :orderId");
        updateQuery.bindValue(":statusId", statusId);
        updateQuery.bindValue(":orderId", currentOrderId);

        if (updateQuery.exec())
        {
            QList<QMessageBox::Button> buttons = {QMessageBox::Ok};
            showMessage("Статус обновлён!", "", QMessageBox::Information, buttons);
            loadOrderTable();
        }
    }
}

void MainWindow::addNewOrder()
{
    db = QSqlDatabase::database("bd_bolnov");
    if(getDBConnection(db))
    {
        QPixmap image = ui->label_8->pixmap();
        QByteArray arr;
        QBuffer buffer(&arr);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        QSqlQuery query(db);
        query.prepare("INSERT INTO orders (user_id, status_id, order_date, order_text, order_doc) "
                      "VALUES (:user_id, :status_id, :order_date, :order_text, :order_doc)");
        query.bindValue(":user_id", userID);
        query.bindValue(":status_id", 1);
        query.bindValue(":order_date", QDate::currentDate().toString("yyyy-MM-dd"));
        query.bindValue(":order_text", ui->plainTextEdit->toPlainText());
        query.bindValue(":order_doc", arr);
        if (query.exec())
        {
            QList<QMessageBox::Button> buttons = {QMessageBox::Ok};
            showMessage("Заявка добавлена!", "", QMessageBox::Information, buttons);
            ui->plainTextEdit->clear();
            ui->label_8->clear();
            ui->tabWidget->setCurrentIndex(0);
            loadOrderTable();
        }
    }
}

QPixmap MainWindow::getImageFromDB(QByteArray arr)
{
    QPixmap image;
    image.loadFromData(arr);
    return image;
}

void MainWindow::selectDoc()
{
    QStringList imageName;
    QFileDialog fd;
    fd.setAcceptMode(QFileDialog::AcceptOpen);
    fd.setViewMode(QFileDialog::Detail);
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setWindowTitle("Выберите документ");
    fd.setNameFilter("Image files (*.png *.jpeg *.jpg)");
    if(fd.exec())
    {
        imageName = fd.selectedFiles();
    }
    if(!imageName.isEmpty())
    {
        QPixmap image;
        image.load(imageName.at(0));
        ui->label_8->setPixmap(image);
    }
}

void MainWindow::loadDoc(int orderID)
{
    doc *chp = new doc(orderID);
    chp->show();
}

bool MainWindow::hasAccess(const QString &requiredRole)
{
    return userRole == requiredRole;
    qDebug() << userRole;
}

void MainWindow::adminFunction()
{
    if(hasAccess("user"))
    {
        ui->cb_client->setVisible(false);
        ui->cb_sort->setVisible(false);
        ui->rb_up->setVisible(false);
        ui->rb_down->setVisible(false);
        ui->cb_sort_search->setVisible(false);
        ui->le_search->setVisible(false);
        ui->qpb_sort_clear->setVisible(false);
        ui->label_16->setVisible(false);
        ui->cb_orderStatus->setVisible(false);
        ui->qpb_updateStatus->setVisible(false);
        ui->le_p_name->setVisible(true);
        ui->le_p_surname->setVisible(true);
        ui->le_p_patronymic->setVisible(true);
        ui->le_p_phone->setVisible(true);
        ui->le_p_email->setVisible(true);
        ui->le_p_inn->setVisible(true);
        ui->qpb_p_change->setVisible(true);
        ui->qpb_p_change_ok->setVisible(true);
        ui->label_5->setVisible(true);
        ui->label_6->setVisible(false);
        ui->label_7->setVisible(false);
        ui->tabWidget->setTabVisible(1, true);
        ui->qpb_logout->setGeometry(ui->qpb_logout->x(), 216, ui->qpb_logout->width(), ui->qpb_logout->height());
        ui->tableWidget->setGeometry(ui->tableWidget->x(), 6, ui->tableWidget->width(), ui->tableWidget->height());
        ui->tabWidget->setGeometry(ui->tabWidget->x(), 6, ui->tabWidget->width(), ui->tabWidget->height());
        ui->tableWidget->setFixedHeight(708);
        ui->tabWidget->setFixedHeight(708);
    }
    else
    {
        ui->cb_client->setVisible(true);
        ui->cb_sort->setVisible(true);
        ui->rb_up->setVisible(true);
        ui->rb_down->setVisible(true);
        ui->cb_sort_search->setVisible(true);
        ui->le_search->setVisible(true);
        ui->qpb_sort_clear->setVisible(true);
        ui->label_16->setVisible(true);
        ui->cb_orderStatus->setVisible(true);
        ui->qpb_updateStatus->setVisible(true);
        ui->le_p_name->setVisible(false);
        ui->le_p_surname->setVisible(false);
        ui->le_p_patronymic->setVisible(false);
        ui->le_p_phone->setVisible(false);
        ui->le_p_email->setVisible(false);
        ui->le_p_inn->setVisible(false);
        ui->qpb_p_change->setVisible(false);
        ui->qpb_p_change_ok->setVisible(false);
        ui->label_5->setVisible(false);
        ui->label_6->setVisible(true);
        ui->label_7->setVisible(true);
        ui->tabWidget->setTabVisible(1, false);
        ui->qpb_logout->setGeometry(ui->qpb_logout->x(), 6, ui->qpb_logout->width(), ui->qpb_logout->height());
        ui->tableWidget->setGeometry(ui->tableWidget->x(), 40, ui->tableWidget->width(), ui->tableWidget->height());
        ui->tabWidget->setGeometry(ui->tabWidget->x(), 40, ui->tabWidget->width(), ui->tabWidget->height());
        ui->tableWidget->setFixedHeight(675);
        ui->tabWidget->setFixedHeight(675);
    }
}
