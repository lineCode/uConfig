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

#include "componentpinsitemmodel.h"

ComponentPinsItemModel::ComponentPinsItemModel(Component *component, QObject *parent) :
    QAbstractItemModel(parent)
{
    _component = component;
}

Component *ComponentPinsItemModel::component() const
{
    return _component;
}

void ComponentPinsItemModel::setComponent(Component *component)
{
    emit layoutAboutToBeChanged();
    _component = component;
    emit layoutChanged();
}

Pin *ComponentPinsItemModel::pin(const QModelIndex &index) const
{
    if (!index.isValid())
        return Q_NULLPTR;
    return static_cast<Pin*>(index.internalPointer());
}

QModelIndex ComponentPinsItemModel::index(Pin *pin) const
{
    QModelIndexList list = match(this->index(0, 0), ComponentPinsItemModel::PinNumber, pin->padName(), -1, Qt::MatchRecursive);
    if (list.isEmpty())
        return QModelIndex();
    return list.first();
}

int ComponentPinsItemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return ColumnCount;
}

QVariant ComponentPinsItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation==Qt::Vertical)
        return QVariant();
    switch (role)
    {
    case Qt::DisplayRole:
        switch (section)
        {
        case PinNumber:
            return QVariant("#");
        case PinName:
            return QVariant("Name");
        case PinElecType:
            return QVariant("Elec type");
        case PinStyle:
            return QVariant("Style");
        case PinClass:
            return QVariant("Class");
        }
        break;
    }
    return QVariant();
}

QVariant ComponentPinsItemModel::data(const QModelIndex &index, int role) const
{
    if (!_component)
        return QVariant();

    Pin *pin = _component->pins().at(index.row());

    switch (role)
    {
    case Qt::DisplayRole:
        switch (index.column())
        {
        case PinNumber:
            return QVariant(pin->padName());
        case PinName:
            return QVariant(pin->name());
        case PinElecType:
            return QVariant(Pin::electricalTypeDesc(pin->electricalType()));
        case PinStyle:
            return QVariant(Pin::pinTypeDesc(pin->pinType()));
        case PinClass:
            return QVariant(pin->className());
        }
    case Qt::EditRole:
        switch (index.column())
        {
        case PinNumber:
            return QVariant(pin->padName());
        case PinName:
            return QVariant(pin->name());
        case PinElecType:
            return QVariant(pin->electricalType());
        case PinStyle:
            return QVariant(pin->pinType());
        case PinClass:
            return QVariant();
        }
    }
    return QVariant();
}

QModelIndex ComponentPinsItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!_component)
        return QModelIndex();
    Q_UNUSED(parent)
    return createIndex(row, column, _component->pins()[row]);
}

QModelIndex ComponentPinsItemModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return QModelIndex();
}

int ComponentPinsItemModel::rowCount(const QModelIndex &parent) const
{
    if (!_component)
        return 0;
    if (!parent.isValid())
        return _component->pins().count();
    return 0;
}


bool ComponentPinsItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!_component)
        return false;

    Pin *pin = _component->pins().at(index.row());

    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch (index.column())
        {
        case PinNumber:
            pin->setPadName(value.toString());
            break;
        case PinName:
            pin->setName(value.toString());
            break;
        case PinElecType:
            pin->setElectricalType(static_cast<Pin::ElectricalType>(value.toInt()));
            break;
        case PinStyle:
            pin->setPinType(static_cast<Pin::PinType>(value.toInt()));
            break;
        case PinClass:
            return false;
        }
        emit pinModified(pin);
        return true;
    }
    return false;
}

Qt::ItemFlags ComponentPinsItemModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags baseFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (index.column() == PinNumber || index.column() == PinName
     || index.column() == PinElecType || index.column() == PinStyle)
        return baseFlags | Qt::ItemIsEditable;
    return baseFlags;
}

void ComponentPinsItemModel::remove(const QModelIndex &index)
{
    Pin *mpin = pin(index);
    if (!mpin)
        return;

    emit layoutAboutToBeChanged();
    emit pinRemoved(mpin);
    _component->removePin(mpin);
    emit layoutChanged();
}
