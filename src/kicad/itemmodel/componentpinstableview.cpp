/**
 ** This file is part of the uConfig project.
 ** Copyright 2018 Robotips sebastien.caux@robotips.fr.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/

#include "componentpinstableview.h"

#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>

ComponentPinsTableView::ComponentPinsTableView(Component *component, QWidget *parent)
    : QTableView(parent)
{
    if (component)
        _model = new ComponentPinsItemModel(component);
    else
        _model = new ComponentPinsItemModel(new Component());

    _sortProxy = new NumericalSortFilterProxyModel();
    _sortProxy->setSourceModel(_model);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setModel(_sortProxy);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &ComponentPinsTableView::updateSelect);

    sortByColumn(0, Qt::AscendingOrder);
    setSortingEnabled(true);

    _sortProxy->setFilterRole(Qt::DisplayRole);
    _sortProxy->setFilterKeyColumn(ComponentPinsItemModel::PinName);

    _delegate = new ComponentPinDelegate(this);
    setItemDelegate(_delegate);

    createActions();
}

Component *ComponentPinsTableView::component() const
{
    return _model->component();
}

void ComponentPinsTableView::setComponent(Component *component)
{
    _model->setComponent(component);
    resizeColumnsToContents();
}

void ComponentPinsTableView::selectPin(Pin *pin)
{
    if (!pin)
    {
        selectionModel()->clearSelection();
        return;
    }
    const QPersistentModelIndex index = _model->index(pin);
    if (!index.isValid())
        return;
    const QModelIndex &indexPin = _sortProxy->mapFromSource(index);
    if (!indexPin.isValid())
        return;
    selectionModel()->select(indexPin, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    scrollTo(indexPin);
}

void ComponentPinsTableView::selectPins(QList<Pin *> pins)
{
    if (pins.isEmpty())
    {
        selectionModel()->clearSelection();
        return;
    }
    if (pins.count() == 1)
    {
        selectPin(pins.first());
        return;
    }
    foreach (Pin *pin, pins)
    {
        const QPersistentModelIndex index = _model->index(pin);
        if (!index.isValid())
            return;
        const QModelIndex &indexPin = _sortProxy->mapFromSource(index);
        if (!indexPin.isValid())
            return;
        selectionModel()->select(indexPin, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
}

void ComponentPinsTableView::setPinFilter(const QString &filter)
{
    _sortProxy->setFilterRegExp(QRegExp(filter, Qt::CaseInsensitive));
    _delegate->setSearchPattern(QRegularExpression(filter, QRegularExpression::CaseInsensitiveOption));

    viewport()->update();
}

void ComponentPinsTableView::remove()
{
    QModelIndexList selection = selectionModel()->selectedIndexes();
    if (selection.isEmpty())
        return;

    if (selection.size() > 0)
    {
        QList<QPersistentModelIndex> pindex;
        foreach (QModelIndex selected, selection)
        {
            const QModelIndex &indexComponent = _sortProxy->mapToSource(selected);
            if (!indexComponent.isValid())
                continue;

            pindex.append(indexComponent);
        }
        if (QMessageBox::question(this, tr("Remove pins?"), tr("Do you realy want to remove theses %1 pins?")
                                 .arg(pindex.count() / ComponentPinsItemModel::ColumnCount)) != QMessageBox::Yes)
            return;
        selectionModel()->clearSelection();
        //emit openedComponent(Q_NULLPTR);
        foreach (QPersistentModelIndex index, pindex)
            _model->remove(index);
    }
}

void ComponentPinsTableView::updateSelect(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(selected)
    Q_UNUSED(deselected)

    QSet<Pin*> selectedPins;
    foreach (const QModelIndex &index, selectionModel()->selectedIndexes())
    {
        if (!index.isValid())
            continue;
        const QModelIndex &indexComponent = _sortProxy->mapToSource(index);
        if (!indexComponent.isValid())
            continue;

        selectedPins.insert(_model->pin(indexComponent));
    }
    _removeAction->setEnabled(!selectedPins.isEmpty());

    emit pinSelected(selectedPins.toList());
}

void ComponentPinsTableView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu;
    menu.addAction(_removeAction);
    menu.exec(event->globalPos());
}

void ComponentPinsTableView::createActions()
{
    _removeAction = new QAction(this);
    _removeAction->setText(tr("Remove"));
    _removeAction->setShortcut(QKeySequence::Delete);
    _removeAction->setShortcutContext(Qt::WidgetShortcut);
    _removeAction->setEnabled(false);
    connect(_removeAction, SIGNAL(triggered(bool)), this, SLOT(remove()));
    addAction(_removeAction);
}

ComponentPinsItemModel *ComponentPinsTableView::model() const
{
    return _model;
}
