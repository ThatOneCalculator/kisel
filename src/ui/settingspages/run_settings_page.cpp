#include "run_settings_page.hpp"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QProcess>
#include <QPushButton>
#include <QToolTip>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;
using namespace kisel;

RunSettingsPage::RunSettingsPage(BaseSettings* settings, QWidget* parent)
    : QWidget(parent)
    , m_settings(settings)
    , m_customRunCommandEdit(new QLineEdit(this))
    , m_tableWidget(new QTableWidget(this))
{
    auto* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop);

    auto* commandEditLabel = new QLabel(tr("Launch command"), this);
    layout->addWidget(commandEditLabel);

    m_normalEditPalette = m_customRunCommandEdit->palette();
    m_errorEditPalette = m_customRunCommandEdit->palette();
    m_errorEditPalette.setColor(QPalette::Text, QColor(255, 0, 0));

    m_customRunCommandEdit->setClearButtonEnabled(true);
    m_customRunCommandEdit->setToolTip(tr("Edit the launch command"));
    m_customRunCommandEdit->setPlaceholderText("%command%"_L1);
    m_customRunCommandEdit->setText(m_settings->customRunCommand());
    connect(m_customRunCommandEdit, &QLineEdit::textEdited, this, &RunSettingsPage::onCustomRunCommandEdit);
    connect(m_customRunCommandEdit, &QLineEdit::editingFinished, this, [this] {
        m_customRunCommandEdit->setText(m_settings->customRunCommand());
        clearCustomRunCommandEditError();
    });
    layout->addWidget(m_customRunCommandEdit);

    auto* variablesLabel = new QLabel(tr("Environment variables"), this);
    layout->addWidget(variablesLabel);

    m_tableWidget->setColumnCount(2);
    m_tableWidget->setHorizontalHeaderLabels({ tr("Variable"), tr("Value") });
    m_tableWidget->horizontalHeader()->setStretchLastSection(true);
    m_tableWidget->verticalHeader()->setVisible(false);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setEditTriggers(QAbstractItemView::DoubleClicked
        | QAbstractItemView::EditKeyPressed
        | QAbstractItemView::AnyKeyPressed);
    m_tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_tableWidget, &QTableWidget::customContextMenuRequested, this, &RunSettingsPage::onContextMenuRequested);
    layout->addWidget(m_tableWidget);

    auto* delegate = new EnvDelegate(m_tableWidget);
    m_tableWidget->setItemDelegate(delegate);

    connect(delegate, &EnvDelegate::closeEditor, this, &RunSettingsPage::onCloseEditor);

    auto* addVarButton = new QPushButton(QIcon::fromTheme("list-add"), tr("Add"), this);
    connect(addVarButton, &QPushButton::clicked, this, &RunSettingsPage::addEditRow);
    layout->addWidget(addVarButton);

    loadVariables();
}

void RunSettingsPage::loadVariables()
{
    const QMap<QString, QVariant> map = m_settings->envMap();
    int row = -1;
    m_tableWidget->setRowCount(0);
    for (auto [key, value] : map.asKeyValueRange()) {
        m_tableWidget->insertRow(++row);
        m_tableWidget->setItem(row, 0, new QTableWidgetItem(key));
        m_tableWidget->setItem(row, 1, new QTableWidgetItem(value.toString()));
        m_tableWidget->setCurrentCell(row, 0);
    }
}

void RunSettingsPage::onCustomRunCommandEdit(QString text)
{
    text = text.trimmed();

    if (text.isEmpty()) {
        clearCustomRunCommandEditError();
        return;
    }

    const QStringList tokens = QProcess::splitCommand(text);

    // 1. %command%
    if (!tokens.contains("%command%"_L1)) {
        setCustomRunCommandEditError(tr("The string must contain %command%"));
        return;
    }

    // 2. Multiple %command%
    const qsizetype count = tokens.count("%command%"_L1);
    if (count > 1) {
        setCustomRunCommandEditError(tr("Multiple %command% found — only one is allowed"));
        return;
    }

    // 3. KEY=VALUE
    static const QRegularExpression envPattern(
        R"(^[\p{L}_][\p{L}\p{N}_]*=)"_L1,
        QRegularExpression::UseUnicodePropertiesOption);

    for (const QString& token : tokens) {
        if (token == "%command%"_L1) {
            continue;
        }

        if (envPattern.match(token).hasMatch()) {
            setCustomRunCommandEditError(tr("Environment variables (KEY=VALUE) are not allowed — use the table below"));
            return;
        }
    }

    clearCustomRunCommandEditError();
}

void RunSettingsPage::setCustomRunCommandEditError(const QString& message)
{
    m_customRunCommandEdit->setPalette(m_errorEditPalette);
    m_customRunCommandEdit->setToolTip(message);

    if (m_customRunCommandEdit->hasFocus()) {
        QToolTip::showText(mapToGlobal(m_customRunCommandEdit->pos()), message, m_customRunCommandEdit);
    }
}

void RunSettingsPage::clearCustomRunCommandEditError()
{
    m_customRunCommandEdit->setPalette(m_normalEditPalette);
    setToolTip(QString());
    QToolTip::hideText();
    m_settings->setCustomRunCommand(m_customRunCommandEdit->text().trimmed());
}

void RunSettingsPage::onContextMenuRequested(const QPoint& pos)
{
    QModelIndex index = m_tableWidget->indexAt(pos);

    if (!index.isValid()) {
        return;
    }

    QMenu menu(this);

    QAction* deleteAction = menu.addAction(QIcon::fromTheme("entry-delete"), tr("Delete"));
    connect(deleteAction, &QAction::triggered, this, [this] {
        removeSelectedRows();
    });

    menu.exec(m_tableWidget->viewport()->mapToGlobal(pos));
}

void RunSettingsPage::addEditRow()
{
    const int row = m_tableWidget->rowCount();
    m_tableWidget->insertRow(row);
    m_tableWidget->setItem(row, 0, new QTableWidgetItem(QString()));
    m_tableWidget->setItem(row, 1, new QTableWidgetItem(QString()));
    m_tableWidget->setCurrentCell(row, 0);
    m_tableWidget->editItem(m_tableWidget->item(row, 0));
}

void RunSettingsPage::removeSelectedRows()
{
    m_settings->beginEnv();
    const auto items = m_tableWidget->selectionModel()->selectedRows();
    for (int i = static_cast<int>(items.count()) - 1; i >= 0; --i) {
        const int row = items.at(i).row();
        QString varName = m_tableWidget->model()->index(row, 0).data().toString();
        m_settings->remove(varName);
        m_tableWidget->removeRow(row);
    }
    m_settings->endGroup();
}

QWidget* EnvDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    auto* lineEdit = new QLineEdit(parent);

    if (index.column() == 0) {
        static QRegularExpression exp("[A-Za-z_][A-Za-z0-9_]*"_L1);
        lineEdit->setValidator(new QRegularExpressionValidator(exp, lineEdit));
    }

    return lineEdit;
}

void RunSettingsPage::onCloseEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint)
{
    const auto index = m_tableWidget->currentIndex();
    const int row = index.row();
    const QString text = index.data().toString();

    if (index.column() == 0) {
        if (text.isEmpty()) {
            m_tableWidget->removeRow(index.row());
            return;
        }

        m_settings->insertEnv(text, { });
    } else {
        QString varName = m_tableWidget->model()->index(row, 0).data().toString();
        m_settings->insertEnv(varName, { text });
    }
}

void EnvDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* edit = dynamic_cast<QLineEdit*>(editor);
    const QString text = edit->text().trimmed();

    if (index.column() == 0) {
        for (int row = 0; row < model->rowCount(); ++row) {
            if (row != index.row() && model->index(row, 0).data().toString() == text) {
                return;
            }
        }
    }
    model->setData(index, text, Qt::EditRole);
}
