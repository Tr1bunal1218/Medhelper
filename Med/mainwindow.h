#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QComboBox>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void addMedication();
    void loadTableData();
    QTableWidget *table;
    void showMedicationDetails(int row, int column);
    void searchMedication(); // Объявление нового слота
    QLineEdit *searchField;
    void sortColumn(int column);
    void resetTable(); // Объявление слота для сброса
    QComboBox *weekSelector;
    void clearAllData();

};

#endif // MAINWINDOW_H
