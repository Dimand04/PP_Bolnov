#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QtWidgets/QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void showAuthMenu();
    void showRegMenu();
    void tryLogin();
    void showMainMenu();
    void tryReg();
    void loadFaceName();
    void logout();
    void lereadonly();
    void loadOrderTable();
    void loadUser();
    void clearFilter();
    void loadSortOptions();
    void loadSearchOptions();
    void applyFiltersAndSort();
    void updatePlaceholder();
    void TableWidgetRowClicked(int row, int column);
    void loadOrderDetails(int orderId);
    void updateOrderStatus();
    void addNewOrder();
    void selectDoc();
    void adminFunction();

private:
    Ui::MainWindow *ui;
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", "bd_bolnov");
    QString userID;
    QString userRole;
    void getUserInfo(QString);
    void updateUserInfo();
    int currentOrderId;
    QPixmap currentImage;
    void loadDoc(int ID);
    QPixmap getImageFromDB(QByteArray);
    bool hasAccess(const QString &requiredRole);

};
#endif // MAINWINDOW_H
