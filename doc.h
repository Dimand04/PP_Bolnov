#ifndef DOC_H
#define DOC_H

#include <QWidget>
#include <QSqlDatabase>
#include <QByteArray>
#include <QLabel>

namespace Ui {
class doc;
}

class doc : public QWidget
{
    Q_OBJECT

public:
    explicit doc(int docID, QWidget *parent = nullptr);
    ~doc();

private:
    Ui::doc *ui;
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", "bd_bolnov");
    int docID;
    void loadImages();
};

#endif // DOC_H
