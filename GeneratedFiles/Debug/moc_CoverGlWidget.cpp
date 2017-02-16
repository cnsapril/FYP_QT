/****************************************************************************
** Meta object code from reading C++ file 'CoverGlWidget.hpp'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../CoverGlWidget.hpp"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CoverGlWidget.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_CoverGlWidget_t {
    QByteArrayData data[8];
    char stringdata0[108];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_CoverGlWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_CoverGlWidget_t qt_meta_stringdata_CoverGlWidget = {
    {
QT_MOC_LITERAL(0, 0, 13), // "CoverGlWidget"
QT_MOC_LITERAL(1, 14, 20), // "selectionModeChanged"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 6), // "status"
QT_MOC_LITERAL(4, 43, 22), // "selectionButtonChanged"
QT_MOC_LITERAL(5, 66, 15), // "upButtonPressed"
QT_MOC_LITERAL(6, 82, 12), // "coverChanged"
QT_MOC_LITERAL(7, 95, 12) // "currentIndex"

    },
    "CoverGlWidget\0selectionModeChanged\0\0"
    "status\0selectionButtonChanged\0"
    "upButtonPressed\0coverChanged\0currentIndex"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_CoverGlWidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,
       4,    1,   32,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    1,   35,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void, QMetaType::Bool,    5,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    7,

       0        // eod
};

void CoverGlWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        CoverGlWidget *_t = static_cast<CoverGlWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->selectionModeChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->selectionButtonChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->coverChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (CoverGlWidget::*_t)(bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&CoverGlWidget::selectionModeChanged)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (CoverGlWidget::*_t)(bool );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&CoverGlWidget::selectionButtonChanged)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject CoverGlWidget::staticMetaObject = {
    { &QGLWidget::staticMetaObject, qt_meta_stringdata_CoverGlWidget.data,
      qt_meta_data_CoverGlWidget,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *CoverGlWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CoverGlWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_CoverGlWidget.stringdata0))
        return static_cast<void*>(const_cast< CoverGlWidget*>(this));
    return QGLWidget::qt_metacast(_clname);
}

int CoverGlWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGLWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void CoverGlWidget::selectionModeChanged(bool _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void CoverGlWidget::selectionButtonChanged(bool _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
