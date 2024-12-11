#include <QSqlError>
#include <QDebug>
#include <QSqlQuery>
#include "manager.h"

DatabaseManager::DatabaseManager() {
    // Используем драйвер QSQLITE для SQLite
    this->db = QSqlDatabase::addDatabase("QSQLITE");
}

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::openConnection() {
    db.setDatabaseName("C:\\Users\\Danila\\Documents\\Med\\med.db");

    // Открываем базу данных
    if (!db.open()) {
        qDebug() << "Ошибка подключения к базе данных:" << db.lastError().text();
        return false;
    }

    qDebug() << "Подключение к базе данных успешно!";
    return true;
}

void DatabaseManager::closeConnection() {
    if (db.isOpen()) {
        db.close();  // Закрываем соединение
    }
    QString connectionName = db.connectionName();
    db = QSqlDatabase();                // Обнуляем объект базы данных
    QSqlDatabase::removeDatabase(connectionName);  // Удаляем соединение из пула

    qDebug() << "Соединение с базой данных закрыто и удалено из пула.";
}

