#ifndef TABLENUMBERITEM_H
#define TABLENUMBERITEM_H

class TableNumberItem : public QTableWidgetItem
{
public:
    TableNumberItem(const QString txt = QString("0"))
        :QTableWidgetItem(txt)
    {
    }
    bool operator <(const QTableWidgetItem &other) const
    {
        QString str1 = text();
        QString str2 = other.text();

        if (str1[str1.length() - 1] == '%') {
            str1.chop(1);
            str2.chop(1);
        }

    return str1.toDouble() < str2.toDouble();
    }
};

#endif // TABLENUMBERITEM_H
