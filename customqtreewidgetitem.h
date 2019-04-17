#ifndef CUSTOMQTREEWIDGETITEM_H
#define CUSTOMQTREEWIDGETITEM_H

#include <QTreeWidgetItem>

class CustomQTreeWidgetItem : public QTreeWidgetItem
{
public:
    explicit CustomQTreeWidgetItem(const QStringList &strings, const qint64 &size);
    qint64 size;
};

#endif // CUSTOMQTREEWIDGETITEM_H
