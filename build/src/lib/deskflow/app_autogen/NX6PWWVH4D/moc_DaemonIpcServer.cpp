/****************************************************************************
** Meta object code from reading C++ file 'DaemonIpcServer.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../src/lib/deskflow/ipc/DaemonIpcServer.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DaemonIpcServer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN8deskflow4core3ipc15DaemonIpcServerE_t {};
} // unnamed namespace


#ifdef QT_MOC_HAS_STRINGDATA
static constexpr auto qt_meta_stringdata_ZN8deskflow4core3ipc15DaemonIpcServerE = QtMocHelpers::stringData(
    "deskflow::core::ipc::DaemonIpcServer",
    "logLevelChanged",
    "",
    "logLevel",
    "elevateModeChanged",
    "elevate",
    "commandChanged",
    "command",
    "startProcessRequested",
    "stopProcessRequested",
    "clearSettingsRequested",
    "handleNewConnection",
    "handleReadyRead",
    "handleDisconnected",
    "handleErrorOccurred"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA

Q_CONSTINIT static const uint qt_meta_data_ZN8deskflow4core3ipc15DaemonIpcServerE[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   74,    2, 0x06,    1 /* Public */,
       4,    1,   77,    2, 0x06,    3 /* Public */,
       6,    1,   80,    2, 0x06,    5 /* Public */,
       8,    0,   83,    2, 0x06,    7 /* Public */,
       9,    0,   84,    2, 0x06,    8 /* Public */,
      10,    0,   85,    2, 0x06,    9 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      11,    0,   86,    2, 0x08,   10 /* Private */,
      12,    0,   87,    2, 0x08,   11 /* Private */,
      13,    0,   88,    2, 0x08,   12 /* Private */,
      14,    0,   89,    2, 0x08,   13 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject deskflow::core::ipc::DaemonIpcServer::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ZN8deskflow4core3ipc15DaemonIpcServerE.offsetsAndSizes,
    qt_meta_data_ZN8deskflow4core3ipc15DaemonIpcServerE,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_tag_ZN8deskflow4core3ipc15DaemonIpcServerE_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DaemonIpcServer, std::true_type>,
        // method 'logLevelChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'elevateModeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'commandChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'startProcessRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'stopProcessRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'clearSettingsRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleNewConnection'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleReadyRead'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleDisconnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleErrorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void deskflow::core::ipc::DaemonIpcServer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DaemonIpcServer *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->logLevelChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->elevateModeChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 2: _t->commandChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->startProcessRequested(); break;
        case 4: _t->stopProcessRequested(); break;
        case 5: _t->clearSettingsRequested(); break;
        case 6: _t->handleNewConnection(); break;
        case 7: _t->handleReadyRead(); break;
        case 8: _t->handleDisconnected(); break;
        case 9: _t->handleErrorOccurred(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _q_method_type = void (DaemonIpcServer::*)(const QString & );
            if (_q_method_type _q_method = &DaemonIpcServer::logLevelChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _q_method_type = void (DaemonIpcServer::*)(bool );
            if (_q_method_type _q_method = &DaemonIpcServer::elevateModeChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _q_method_type = void (DaemonIpcServer::*)(const QString & );
            if (_q_method_type _q_method = &DaemonIpcServer::commandChanged; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _q_method_type = void (DaemonIpcServer::*)();
            if (_q_method_type _q_method = &DaemonIpcServer::startProcessRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _q_method_type = void (DaemonIpcServer::*)();
            if (_q_method_type _q_method = &DaemonIpcServer::stopProcessRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _q_method_type = void (DaemonIpcServer::*)();
            if (_q_method_type _q_method = &DaemonIpcServer::clearSettingsRequested; *reinterpret_cast<_q_method_type *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
    }
}

const QMetaObject *deskflow::core::ipc::DaemonIpcServer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *deskflow::core::ipc::DaemonIpcServer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZN8deskflow4core3ipc15DaemonIpcServerE.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int deskflow::core::ipc::DaemonIpcServer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void deskflow::core::ipc::DaemonIpcServer::logLevelChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void deskflow::core::ipc::DaemonIpcServer::elevateModeChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void deskflow::core::ipc::DaemonIpcServer::commandChanged(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void deskflow::core::ipc::DaemonIpcServer::startProcessRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void deskflow::core::ipc::DaemonIpcServer::stopProcessRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void deskflow::core::ipc::DaemonIpcServer::clearSettingsRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
