#include <QApplication>
#include <QPalette>
#include "mainwindow.h"
#include "aboutdialog.h"
#include "manager.h"

void setDarkTheme(QApplication &app) {
    QPalette darkPalette;

    // Устанавливаем базовые цвета
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(42, 42, 42));
    darkPalette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);

    // Цвета для выделений
    darkPalette.setColor(QPalette::Highlight, QColor(80, 216, 230));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    // Применяем палитру
    app.setPalette(darkPalette);

    // Устанавливаем стиль Fusion
    app.setStyle("Fusion");
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    setDarkTheme(app);
    MainWindow mainWindow;
    mainWindow.show();

    DatabaseManager::instance().openConnection();

    auto appResult = app.exec();

    DatabaseManager::instance().closeConnection();

    return appResult;
}
