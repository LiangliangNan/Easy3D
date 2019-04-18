/****************************************************************************
** Meta object code from reading C++ file 'keyFrameInterpolator.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../keyFrameInterpolator.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'keyFrameInterpolator.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_qglviewer__KeyFrameInterpolator_t {
    QByteArrayData data[27];
    char stringdata0[362];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_qglviewer__KeyFrameInterpolator_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_qglviewer__KeyFrameInterpolator_t qt_meta_stringdata_qglviewer__KeyFrameInterpolator = {
    {
QT_MOC_LITERAL(0, 0, 31), // "qglviewer::KeyFrameInterpolator"
QT_MOC_LITERAL(1, 32, 12), // "interpolated"
QT_MOC_LITERAL(2, 45, 0), // ""
QT_MOC_LITERAL(3, 46, 10), // "endReached"
QT_MOC_LITERAL(4, 57, 11), // "addKeyFrame"
QT_MOC_LITERAL(5, 69, 5), // "Frame"
QT_MOC_LITERAL(6, 75, 5), // "frame"
QT_MOC_LITERAL(7, 81, 4), // "time"
QT_MOC_LITERAL(8, 86, 11), // "Frame*const"
QT_MOC_LITERAL(9, 98, 10), // "deletePath"
QT_MOC_LITERAL(10, 109, 8), // "setFrame"
QT_MOC_LITERAL(11, 118, 20), // "setInterpolationTime"
QT_MOC_LITERAL(12, 139, 21), // "setInterpolationSpeed"
QT_MOC_LITERAL(13, 161, 5), // "speed"
QT_MOC_LITERAL(14, 167, 22), // "setInterpolationPeriod"
QT_MOC_LITERAL(15, 190, 6), // "period"
QT_MOC_LITERAL(16, 197, 20), // "setLoopInterpolation"
QT_MOC_LITERAL(17, 218, 4), // "loop"
QT_MOC_LITERAL(18, 223, 13), // "setClosedPath"
QT_MOC_LITERAL(19, 237, 6), // "closed"
QT_MOC_LITERAL(20, 244, 18), // "startInterpolation"
QT_MOC_LITERAL(21, 263, 17), // "stopInterpolation"
QT_MOC_LITERAL(22, 281, 18), // "resetInterpolation"
QT_MOC_LITERAL(23, 300, 19), // "toggleInterpolation"
QT_MOC_LITERAL(24, 320, 17), // "interpolateAtTime"
QT_MOC_LITERAL(25, 338, 6), // "update"
QT_MOC_LITERAL(26, 345, 16) // "invalidateValues"

    },
    "qglviewer::KeyFrameInterpolator\0"
    "interpolated\0\0endReached\0addKeyFrame\0"
    "Frame\0frame\0time\0Frame*const\0deletePath\0"
    "setFrame\0setInterpolationTime\0"
    "setInterpolationSpeed\0speed\0"
    "setInterpolationPeriod\0period\0"
    "setLoopInterpolation\0loop\0setClosedPath\0"
    "closed\0startInterpolation\0stopInterpolation\0"
    "resetInterpolation\0toggleInterpolation\0"
    "interpolateAtTime\0update\0invalidateValues"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_qglviewer__KeyFrameInterpolator[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      23,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,  129,    2, 0x06 /* Public */,
       3,    0,  130,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    1,  131,    2, 0x0a /* Public */,
       4,    2,  134,    2, 0x0a /* Public */,
       4,    1,  139,    2, 0x0a /* Public */,
       4,    2,  142,    2, 0x0a /* Public */,
       9,    0,  147,    2, 0x0a /* Public */,
      10,    1,  148,    2, 0x0a /* Public */,
      11,    1,  151,    2, 0x0a /* Public */,
      12,    1,  154,    2, 0x0a /* Public */,
      14,    1,  157,    2, 0x0a /* Public */,
      16,    1,  160,    2, 0x0a /* Public */,
      16,    0,  163,    2, 0x2a /* Public | MethodCloned */,
      18,    1,  164,    2, 0x0a /* Public */,
      18,    0,  167,    2, 0x2a /* Public | MethodCloned */,
      20,    1,  168,    2, 0x0a /* Public */,
      20,    0,  171,    2, 0x2a /* Public | MethodCloned */,
      21,    0,  172,    2, 0x0a /* Public */,
      22,    0,  173,    2, 0x0a /* Public */,
      23,    0,  174,    2, 0x0a /* Public */,
      24,    1,  175,    2, 0x0a /* Public */,
      25,    0,  178,    2, 0x08 /* Private */,
      26,    0,  179,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 5, QMetaType::QReal,    6,    7,
    QMetaType::Void, 0x80000000 | 8,    6,
    QMetaType::Void, 0x80000000 | 8, QMetaType::QReal,    6,    7,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 8,    6,
    QMetaType::Void, QMetaType::QReal,    7,
    QMetaType::Void, QMetaType::QReal,   13,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void, QMetaType::Bool,   17,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   19,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QReal,    7,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void qglviewer::KeyFrameInterpolator::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<KeyFrameInterpolator *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->interpolated(); break;
        case 1: _t->endReached(); break;
        case 2: _t->addKeyFrame((*reinterpret_cast< const Frame(*)>(_a[1]))); break;
        case 3: _t->addKeyFrame((*reinterpret_cast< const Frame(*)>(_a[1])),(*reinterpret_cast< qreal(*)>(_a[2]))); break;
        case 4: _t->addKeyFrame((*reinterpret_cast< const Frame*const(*)>(_a[1]))); break;
        case 5: _t->addKeyFrame((*reinterpret_cast< const Frame*const(*)>(_a[1])),(*reinterpret_cast< qreal(*)>(_a[2]))); break;
        case 6: _t->deletePath(); break;
        case 7: _t->setFrame((*reinterpret_cast< Frame*const(*)>(_a[1]))); break;
        case 8: _t->setInterpolationTime((*reinterpret_cast< qreal(*)>(_a[1]))); break;
        case 9: _t->setInterpolationSpeed((*reinterpret_cast< qreal(*)>(_a[1]))); break;
        case 10: _t->setInterpolationPeriod((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 11: _t->setLoopInterpolation((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 12: _t->setLoopInterpolation(); break;
        case 13: _t->setClosedPath((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 14: _t->setClosedPath(); break;
        case 15: _t->startInterpolation((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: _t->startInterpolation(); break;
        case 17: _t->stopInterpolation(); break;
        case 18: _t->resetInterpolation(); break;
        case 19: _t->toggleInterpolation(); break;
        case 20: _t->interpolateAtTime((*reinterpret_cast< qreal(*)>(_a[1]))); break;
        case 21: _t->update(); break;
        case 22: _t->invalidateValues(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (KeyFrameInterpolator::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&KeyFrameInterpolator::interpolated)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (KeyFrameInterpolator::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&KeyFrameInterpolator::endReached)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject qglviewer::KeyFrameInterpolator::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_qglviewer__KeyFrameInterpolator.data,
    qt_meta_data_qglviewer__KeyFrameInterpolator,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *qglviewer::KeyFrameInterpolator::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *qglviewer::KeyFrameInterpolator::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_qglviewer__KeyFrameInterpolator.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int qglviewer::KeyFrameInterpolator::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 23)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 23;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 23)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 23;
    }
    return _id;
}

// SIGNAL 0
void qglviewer::KeyFrameInterpolator::interpolated()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void qglviewer::KeyFrameInterpolator::endReached()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
