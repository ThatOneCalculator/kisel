#pragma once

#include <QStyledItemDelegate>
#include <QTableWidget>
#include <QWidget>

#include "core/settings/base_settings.hpp"

namespace kisel {
class RunSettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit RunSettingsPage(BaseSettings* settings, QWidget* parent = nullptr);

private slots:
    void onCustomRunCommandEdit(QString text);
    void onContextMenuRequested(const QPoint& pos);
    void addEditRow();
    void onCloseEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint);

private:
    void setCustomRunCommandEditError(const QString& message);
    void clearCustomRunCommandEditError();
    void loadVariables();
    void removeSelectedRows();

    QPalette m_normalEditPalette;
    QPalette m_errorEditPalette;
    BaseSettings* m_settings;
    QLineEdit* m_customRunCommandEdit;
    QTableWidget* m_tableWidget;
};

class EnvDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

signals:
    void editFinished(const QPersistentModelIndex& index);
};
}