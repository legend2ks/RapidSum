#include "customqtreewidgetitem.h"

CustomQTreeWidgetItem::CustomQTreeWidgetItem(const QStringList &strings, const qint64 &size) : QTreeWidgetItem(strings)
{
    this->size = size;
}
