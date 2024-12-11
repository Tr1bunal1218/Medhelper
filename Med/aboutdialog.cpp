#include "aboutdialog.h"
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Информация");

    // Установка базового цвета фона, если нужно
    setStyleSheet("background-color: #353535; color: white;");

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *authorsLabel = new QLabel("Авторы: Логашов Данила ИП-217", this);
    authorsLabel->setStyleSheet("color: white;"); // Белый текст
    layout->addWidget(authorsLabel);

    QPushButton *exitButton = new QPushButton("Выход", this);
    exitButton->setStyleSheet(
        "background-color: #505050; "
        "color: white; "
        "border: 1px solid #707070; "
        "border-radius: 5px; "
        "padding: 5px;");
    layout->addWidget(exitButton);

    connect(exitButton, &QPushButton::clicked, this, [this]() {
        emit exitRequested(); // Генерируем сигнал выхода
        close(); // Закрываем диалог
    });
}
