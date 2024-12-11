#include "mainwindow.h"
#include "manager.h"
#include "aboutdialog.h"
#include <QApplication>
#include <QPalette>
#include <QDebug>
#include <QInputDialog>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QHeaderView>
#include <QTableWidget>
#include <QSqlError>
#include <QLabel>
#include <QComboBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Подключение базы данных
    if (!DatabaseManager::instance().openConnection()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к базе данных!");
    }

    // Центральный виджет и макет
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    resize(800, 600);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Поле поиска
    searchField = new QLineEdit(this);
    searchField->setPlaceholderText("Введите название препарата...");
    mainLayout->addWidget(searchField);

    // Кнопки управления
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *prevWeekButton = new QPushButton("Очистить все", this);
    QPushButton *addButton = new QPushButton("+", this);
    QPushButton *exitButton  = new QPushButton("x", this);
    QPushButton *searchButton = new QPushButton("Поиск", this); // Добавляя кнопку поиска
    QPushButton *resetButton = new QPushButton("Сброс", this); // Кнопка сброса
    buttonLayout->addWidget(resetButton); // Добавляем кнопку сброса
    buttonLayout->addWidget(prevWeekButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(exitButton);
    buttonLayout->addWidget(searchButton); // Добавляем кнопку поиска
    mainLayout->addLayout(buttonLayout);

    // Таблица
    table = new QTableWidget(13, 7, this); // 13 строк, 7 столбцов
    table->setStyleSheet(
        "QTableWidget::item:selected {"
        "    background-color: #29B6F6;"
        "    color: white;"            /* Цвет текста выделенной ячейки */
        "}"
    );
    QStringList headers = {"ПН", "ВТ", "СР", "ЧТ", "ПТ", "СБ", "ВС"};
    table->setHorizontalHeaderLabels(headers);

    QStringList timeSlots = {
        "8:00", "9:00", "10:00", "11:00", "12:00",
        "13:00", "14:00", "15:00", "16:00", "17:00",
        "18:00", "19:00", "20:00"
    };
    table->setVerticalHeaderLabels(timeSlots);

    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    mainLayout->addWidget(table);
    weekSelector = new QComboBox(this);
    weekSelector->addItems({"Неделя 1", "Неделя 2", "Неделя 3", "Неделя 4"});
    mainLayout->addWidget(weekSelector);
    loadTableData();
    // Связываем кнопку "+" с функцией добавления записи
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addMedication);
    connect(table, &QTableWidget::cellDoubleClicked, this, &MainWindow::showMedicationDetails);
    connect(searchButton, &QPushButton::clicked, this, &MainWindow::searchMedication);
    connect(table->horizontalHeader(), &QHeaderView::sectionClicked, this, &MainWindow::sortColumn);

    connect(resetButton, &QPushButton::clicked, this, &MainWindow::resetTable);
    connect(weekSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        qDebug() << "Текущая неделя изменена на: " << weekSelector->currentIndex() + 1;
        loadTableData(); // Загружаем данные для выбранной недели
    });
    connect(prevWeekButton, &QPushButton::clicked, this, &MainWindow::clearAllData);
    connect(exitButton, &QPushButton::clicked, this, [&]() {
        AboutDialog *dialog = new AboutDialog(this);
        connect(dialog, &AboutDialog::exitRequested, this, &QMainWindow::close); // Закрываем главное окно
        dialog->exec(); // Запускаем диалог
    });
}

MainWindow::~MainWindow()
{
}

void MainWindow::addMedication() {
    // Проверяем, выбрана ли ячейка
    QModelIndex currentIndex = table->currentIndex();
    if (!currentIndex.isValid()) {
        QMessageBox::warning(this, "Ошибка", "Выберите ячейку для добавления препарата!");
        return;
    }
    int weekNumber = weekSelector->currentIndex() + 1;
    // Создаем диалоговое окно
    QDialog dialog(this);
    dialog.setWindowTitle("Добавление препарата");

    QFormLayout formLayout(&dialog);

    QLineEdit *nameField = new QLineEdit(&dialog);
    QLineEdit *descField = new QLineEdit(&dialog);
    QCheckBox *importantCheckbox = new QCheckBox("Важный", &dialog);

    formLayout.addRow("Название:", nameField);
    formLayout.addRow("Описание:", descField);
    formLayout.addRow("", importantCheckbox);

    // Кнопки подтверждения
    QHBoxLayout buttonLayout;
    QPushButton okButton("OK", &dialog);
    QPushButton cancelButton("Отмена", &dialog);
    buttonLayout.addWidget(&okButton);
    buttonLayout.addWidget(&cancelButton);

    formLayout.addRow(&buttonLayout);

    connect(&okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(&cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QString name = nameField->text();
        QString description = descField->text();
        bool important = importantCheckbox->isChecked();

        if (name.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Название препарата не может быть пустым!");
            return;
        }

        // Сохраняем данные в базу
        QSqlQuery query;
        query.prepare("INSERT INTO medications (name, description, important, rw, column, week_number) VALUES (:name, :description, :important, :rw, :column, :week_number)");
        query.bindValue(":name", name);
        query.bindValue(":description", description);
        query.bindValue(":important", important);
        query.bindValue(":rw", currentIndex.row());
        query.bindValue(":column", currentIndex.column());
        query.bindValue(":week_number", weekNumber);

        if (query.exec()) {
            QMessageBox::information(this, "Успех", "Препарат успешно добавлен!");

            // Обновляем ячейку таблицы
            int row = currentIndex.row();
            int column = currentIndex.column();
            table->setItem(row, column, new QTableWidgetItem(name));

            // Сохраняем данные в пользовательские свойства ячейки
            table->item(row, column)->setData(Qt::UserRole, description);
            table->item(row, column)->setData(Qt::UserRole + 1, important);
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось добавить препарат: " + query.lastError().text());
        }
    }
}

void MainWindow::loadTableData() {
    // Очистим таблицу, чтобы удалить старые данные
    table->clear();
    QStringList headers = {"ПН", "ВТ", "СР", "ЧТ", "ПТ", "СБ", "ВС"};
    table->setHorizontalHeaderLabels(headers);
    QStringList timeSlots = {
        "8:00", "9:00", "10:00", "11:00", "12:00",
        "13:00", "14:00", "15:00", "16:00", "17:00",
        "18:00", "19:00", "20:00"
    };
    table->setVerticalHeaderLabels(timeSlots);

    int weekNumber = weekSelector->currentIndex() + 1; // Получаем номер недели
    QSqlQuery query;
    query.prepare("SELECT name, description, important, rw, column FROM medications WHERE week_number = :week_number");
    query.bindValue(":week_number", weekNumber);

    if (!query.exec()) {
        QMessageBox::critical(this, "Ошибка выборки", "Не удалось загрузить данные: " + query.lastError().text());
        return;
    }

    while (query.next()) {
        QString name = query.value(0).toString();
        QString description = query.value(1).toString();
        bool important = query.value(2).toBool();
        int row = query.value(3).toInt();
        int column = query.value(4).toInt();

        // Заполняем таблицу только если индексы в пределах допустимого диапазона
        if (row < table->rowCount() && column < table->columnCount()) {
            table->setItem(row, column, new QTableWidgetItem(name));
            table->item(row, column)->setData(Qt::UserRole, description);
            table->item(row, column)->setData(Qt::UserRole + 1, important);
        }
    }
}


void MainWindow::showMedicationDetails(int row, int column) {
    QTableWidgetItem *item = table->item(row, column);

    if (!item) {
        QMessageBox::information(this, "Информация", "Нет данных для отображения.");
        return;
    }

    QString name = item->text();
    QString description = item->data(Qt::UserRole).toString();
    bool important = item->data(Qt::UserRole + 1).toBool();

    // Создаем диалоговое окно для показа информации
    QDialog dialog(this);
    dialog.setWindowTitle("Информация о препарате");

    QFormLayout formLayout(&dialog);

    QLabel *nameLabel = new QLabel(name, &dialog);
    QLabel *descLabel = new QLabel(description, &dialog);
    QLabel *importantLabel = new QLabel(important ? "Важный" : "Второстепенный", &dialog);

    formLayout.addRow("Название:", nameLabel);
    formLayout.addRow("Описание:", descLabel);
    formLayout.addRow("Тип:", importantLabel);

    // Добавляем кнопки "Закрыть" и "Удалить"
    QHBoxLayout buttonLayout;
    QPushButton closeButton("Закрыть", &dialog);
    QPushButton deleteButton("Удалить", &dialog);
    buttonLayout.addWidget(&closeButton);
    buttonLayout.addWidget(&deleteButton);

    formLayout.addRow(&buttonLayout);

    connect(&closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    // Сигнал для удаления препарата
    connect(&deleteButton, &QPushButton::clicked, [&]() {
        // Подтверждение удаления
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(&dialog, "Удаление препарата",
                                      "Вы уверены, что хотите удалить этот препарат?",
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            // Удаляем из базы данных
            QSqlQuery query;
            query.prepare("DELETE FROM medications WHERE name = :name");
            query.bindValue(":name", name);

            if (!query.exec()) {
                QMessageBox::critical(this, "Ошибка", "Не удалось удалить препарат из базы данных: " + query.lastError().text());
                return;
            }

            // Удаляем из таблицы
            table->takeItem(row, column);

            QMessageBox::information(this, "Удаление", "Препарат успешно удален.");
            dialog.accept(); // Закрываем диалог после удаления
        }
    });

    dialog.exec();
}

void MainWindow::searchMedication() {
    QString searchText = searchField->text().trimmed();

    if (searchText.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите название препарата для поиска.");
        return;
    }

    // Убираем подсветку перед новым поиском
    for (int row = 0; row < table->rowCount(); ++row) {
        for (int column = 0; column < table->columnCount(); ++column) {
            QTableWidgetItem *item = table->item(row, column);
            if (item) {
                item->setBackground(QBrush(QColor(0, 0, 0, 0))); // Сбрасываем фон на белый
            }
        }
    }

    // Ищем препараты
    bool found = false;
    for (int row = 0; row < table->rowCount(); ++row) {
        for (int column = 0; column < table->columnCount(); ++column) {
            QTableWidgetItem *item = table->item(row, column);
            if (item && item->text().contains(searchText, Qt::CaseInsensitive)) {
                item->setBackground(QBrush(QColor("#50C878")));
                found = true;
            }
        }
    }

    if (!found) {
        QMessageBox::information(this, "Результаты поиска", "Препараты не найдены.");
    }
}

void MainWindow::sortColumn(int column) {
    // Собираем пары (время, значение) для заполненных и незаполненных строк
    QVector<QPair<QString, QString>> filledItems; // Заполненные
    QVector<QString> emptyTimes;                  // Временные метки для пустых ячеек

    for (int row = 0; row < table->rowCount(); ++row) {
        QTableWidgetItem *item = table->item(row, column);
        QString time = table->verticalHeaderItem(row)->text();

        if (item && !item->text().isEmpty()) {
            filledItems.append(qMakePair(time, item->text()));
        } else {
            emptyTimes.append(time);
        }
    }

    // Сортируем заполненные элементы по имени препарата
    std::sort(filledItems.begin(), filledItems.end(), [](const QPair<QString, QString> &a, const QPair<QString, QString> &b) {
        return a.second < b.second;
    });

    // Очищаем все элементы в столбце
    for (int row = 0; row < table->rowCount(); ++row) {
        table->takeItem(row, column);
    }

    // Заполняем отсортированными данными
    int row = 0;
    for (const auto &pair : filledItems) {
        QTableWidgetItem *item = new QTableWidgetItem(pair.second);
        table->setItem(row, column, item);

        // Обновляем время для текущей строки
        table->verticalHeaderItem(row)->setText(pair.first);
        ++row;
    }

    // Обновляем пустые строки с временными метками
    for (const auto &time : emptyTimes) {
        table->verticalHeaderItem(row)->setText(time);
        ++row;
    }
}


void MainWindow::resetTable() {
    table->clear();
    loadTableData(); // Подразумевается, что эта функция перезагрузит данные из базы данных
}

void MainWindow::clearAllData() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Очистка данных",
                                  "Вы уверены, что хотите удалить все препараты?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        QSqlQuery query;
        query.prepare("DELETE FROM medications");

        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка", "Не удалось очистить данные: " + query.lastError().text());
            return;
        }

        // Очистим таблицу в интерфейсе
        table->clearContents(); // Очищаем содержимое таблицы
        loadTableData(); // Можно также вызвать загрузку данных, если хотите перезагрузить их
        QMessageBox::information(this, "Очистка", "Все препараты были успешно удалены.");
    }
}

