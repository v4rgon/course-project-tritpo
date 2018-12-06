#ifndef TABLEMEMORYITEM_H
#define TABLEMEMORYITEM_H
#include <QString>
#include "memoryconverter.h"

class TableMemoryItem : public QTableWidgetItem
{
public:


    bool operator <(const QTableWidgetItem &other) const
    {
        if (other.text() == "N/A") { return false; }
        const TableMemoryItem& otherMemoryItem = dynamic_cast<const TableMemoryItem&>(other);

        return memory < otherMemoryItem.memory;
    }
protected:
    memoryConverter memory;
private:
    static QString calculateTxt(memoryConverter *memory) {
        if (memory->getValue() > 0.0) {
            return QString::fromStdString(memory->to_string());
        }
        return "N/A";
    }
};

#endif // TABLEMEMORYITEM_H

