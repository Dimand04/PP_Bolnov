#include "doc.h"
#include "ui_doc.h"
#include <QSqlQuery>
#include <QPixmap>
#include <QImage>
#include <QBuffer>
#include "global.h"
#include <QDebug>

doc::doc(int docID, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::doc),
    docID(docID)
{
    ui->setupUi(this);
    ui->tableWidget->setColumnWidth(0,684);
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(1000);
    loadImages();
}

doc::~doc()
{
    delete ui;
}

void doc::loadImages()
{
    if (getDBConnection(db))
    {
        QSqlQuery query(db);
        query.prepare("SELECT order_doc FROM orders WHERE id = ?");
        query.addBindValue(docID);
        query.exec();

        int row = 0;

        while (query.next())
        {
            QByteArray imageData = query.value(0).toByteArray();
            QImage image;
            image.loadFromData(imageData);
            QLabel *imageLabel = new QLabel;
            imageLabel->setPixmap(QPixmap::fromImage(image).scaled(684, 1000, Qt::KeepAspectRatio));
            ui->tableWidget->insertRow(row);
            ui->tableWidget->setCellWidget(row, 0, imageLabel);
            ++row;
        }
    }
}
