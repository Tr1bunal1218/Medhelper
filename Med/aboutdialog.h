#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
signals:
    void exitRequested(); // Сигнал для запроса выхода
};

#endif // ABOUTDIALOG_H
