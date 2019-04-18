/****************************************************************************
** Meta object code from reading C++ file 'qglviewer.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../qglviewer.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qglviewer.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QGLViewer_t {
    QByteArrayData data[153];
    char stringdata0[2145];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QGLViewer_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QGLViewer_t qt_meta_stringdata_QGLViewer = {
    {
QT_MOC_LITERAL(0, 0, 9), // "QGLViewer"
QT_MOC_LITERAL(1, 10, 17), // "viewerInitialized"
QT_MOC_LITERAL(2, 28, 0), // ""
QT_MOC_LITERAL(3, 29, 10), // "drawNeeded"
QT_MOC_LITERAL(4, 40, 12), // "drawFinished"
QT_MOC_LITERAL(5, 53, 9), // "automatic"
QT_MOC_LITERAL(6, 63, 13), // "animateNeeded"
QT_MOC_LITERAL(7, 77, 12), // "helpRequired"
QT_MOC_LITERAL(8, 90, 18), // "axisIsDrawnChanged"
QT_MOC_LITERAL(9, 109, 5), // "drawn"
QT_MOC_LITERAL(10, 115, 18), // "gridIsDrawnChanged"
QT_MOC_LITERAL(11, 134, 21), // "FPSIsDisplayedChanged"
QT_MOC_LITERAL(12, 156, 9), // "displayed"
QT_MOC_LITERAL(13, 166, 20), // "textIsEnabledChanged"
QT_MOC_LITERAL(14, 187, 7), // "enabled"
QT_MOC_LITERAL(15, 195, 21), // "cameraIsEditedChanged"
QT_MOC_LITERAL(16, 217, 6), // "edited"
QT_MOC_LITERAL(17, 224, 13), // "stereoChanged"
QT_MOC_LITERAL(18, 238, 2), // "on"
QT_MOC_LITERAL(19, 241, 13), // "pointSelected"
QT_MOC_LITERAL(20, 255, 18), // "const QMouseEvent*"
QT_MOC_LITERAL(21, 274, 1), // "e"
QT_MOC_LITERAL(22, 276, 19), // "mouseGrabberChanged"
QT_MOC_LITERAL(23, 296, 24), // "qglviewer::MouseGrabber*"
QT_MOC_LITERAL(24, 321, 12), // "mouseGrabber"
QT_MOC_LITERAL(25, 334, 14), // "setAxisIsDrawn"
QT_MOC_LITERAL(26, 349, 4), // "draw"
QT_MOC_LITERAL(27, 354, 14), // "setGridIsDrawn"
QT_MOC_LITERAL(28, 369, 17), // "setFPSIsDisplayed"
QT_MOC_LITERAL(29, 387, 7), // "display"
QT_MOC_LITERAL(30, 395, 16), // "setTextIsEnabled"
QT_MOC_LITERAL(31, 412, 6), // "enable"
QT_MOC_LITERAL(32, 419, 17), // "setCameraIsEdited"
QT_MOC_LITERAL(33, 437, 4), // "edit"
QT_MOC_LITERAL(34, 442, 17), // "toggleAxisIsDrawn"
QT_MOC_LITERAL(35, 460, 17), // "toggleGridIsDrawn"
QT_MOC_LITERAL(36, 478, 20), // "toggleFPSIsDisplayed"
QT_MOC_LITERAL(37, 499, 19), // "toggleTextIsEnabled"
QT_MOC_LITERAL(38, 519, 20), // "toggleCameraIsEdited"
QT_MOC_LITERAL(39, 540, 18), // "setBackgroundColor"
QT_MOC_LITERAL(40, 559, 5), // "color"
QT_MOC_LITERAL(41, 565, 18), // "setForegroundColor"
QT_MOC_LITERAL(42, 584, 14), // "setSceneRadius"
QT_MOC_LITERAL(43, 599, 6), // "radius"
QT_MOC_LITERAL(44, 606, 14), // "setSceneCenter"
QT_MOC_LITERAL(45, 621, 14), // "qglviewer::Vec"
QT_MOC_LITERAL(46, 636, 6), // "center"
QT_MOC_LITERAL(47, 643, 19), // "setSceneBoundingBox"
QT_MOC_LITERAL(48, 663, 3), // "min"
QT_MOC_LITERAL(49, 667, 3), // "max"
QT_MOC_LITERAL(50, 671, 15), // "showEntireScene"
QT_MOC_LITERAL(51, 687, 9), // "setCamera"
QT_MOC_LITERAL(52, 697, 23), // "qglviewer::Camera*const"
QT_MOC_LITERAL(53, 721, 6), // "camera"
QT_MOC_LITERAL(54, 728, 19), // "setManipulatedFrame"
QT_MOC_LITERAL(55, 748, 28), // "qglviewer::ManipulatedFrame*"
QT_MOC_LITERAL(56, 777, 5), // "frame"
QT_MOC_LITERAL(57, 783, 15), // "setMouseGrabber"
QT_MOC_LITERAL(58, 799, 13), // "setFullScreen"
QT_MOC_LITERAL(59, 813, 10), // "fullScreen"
QT_MOC_LITERAL(60, 824, 16), // "setStereoDisplay"
QT_MOC_LITERAL(61, 841, 6), // "stereo"
QT_MOC_LITERAL(62, 848, 16), // "toggleFullScreen"
QT_MOC_LITERAL(63, 865, 19), // "toggleStereoDisplay"
QT_MOC_LITERAL(64, 885, 16), // "toggleCameraMode"
QT_MOC_LITERAL(65, 902, 12), // "saveSnapshot"
QT_MOC_LITERAL(66, 915, 9), // "overwrite"
QT_MOC_LITERAL(67, 925, 8), // "fileName"
QT_MOC_LITERAL(68, 934, 19), // "setSnapshotFileName"
QT_MOC_LITERAL(69, 954, 4), // "name"
QT_MOC_LITERAL(70, 959, 17), // "setSnapshotFormat"
QT_MOC_LITERAL(71, 977, 6), // "format"
QT_MOC_LITERAL(72, 984, 18), // "setSnapshotCounter"
QT_MOC_LITERAL(73, 1003, 7), // "counter"
QT_MOC_LITERAL(74, 1011, 18), // "setSnapshotQuality"
QT_MOC_LITERAL(75, 1030, 7), // "quality"
QT_MOC_LITERAL(76, 1038, 24), // "openSnapshotFormatDialog"
QT_MOC_LITERAL(77, 1063, 19), // "snapshotToClipboard"
QT_MOC_LITERAL(78, 1083, 19), // "copyBufferToTexture"
QT_MOC_LITERAL(79, 1103, 5), // "GLint"
QT_MOC_LITERAL(80, 1109, 14), // "internalFormat"
QT_MOC_LITERAL(81, 1124, 6), // "GLenum"
QT_MOC_LITERAL(82, 1131, 18), // "setAnimationPeriod"
QT_MOC_LITERAL(83, 1150, 6), // "period"
QT_MOC_LITERAL(84, 1157, 14), // "startAnimation"
QT_MOC_LITERAL(85, 1172, 13), // "stopAnimation"
QT_MOC_LITERAL(86, 1186, 7), // "animate"
QT_MOC_LITERAL(87, 1194, 15), // "toggleAnimation"
QT_MOC_LITERAL(88, 1210, 4), // "help"
QT_MOC_LITERAL(89, 1215, 14), // "aboutQGLViewer"
QT_MOC_LITERAL(90, 1230, 6), // "select"
QT_MOC_LITERAL(91, 1237, 5), // "event"
QT_MOC_LITERAL(92, 1243, 5), // "point"
QT_MOC_LITERAL(93, 1249, 19), // "setSelectBufferSize"
QT_MOC_LITERAL(94, 1269, 4), // "size"
QT_MOC_LITERAL(95, 1274, 20), // "setSelectRegionWidth"
QT_MOC_LITERAL(96, 1295, 5), // "width"
QT_MOC_LITERAL(97, 1301, 21), // "setSelectRegionHeight"
QT_MOC_LITERAL(98, 1323, 6), // "height"
QT_MOC_LITERAL(99, 1330, 15), // "setSelectedName"
QT_MOC_LITERAL(100, 1346, 2), // "id"
QT_MOC_LITERAL(101, 1349, 11), // "setShortcut"
QT_MOC_LITERAL(102, 1361, 14), // "KeyboardAction"
QT_MOC_LITERAL(103, 1376, 6), // "action"
QT_MOC_LITERAL(104, 1383, 3), // "key"
QT_MOC_LITERAL(105, 1387, 22), // "setKeyboardAccelerator"
QT_MOC_LITERAL(106, 1410, 17), // "setKeyDescription"
QT_MOC_LITERAL(107, 1428, 11), // "description"
QT_MOC_LITERAL(108, 1440, 14), // "clearShortcuts"
QT_MOC_LITERAL(109, 1455, 14), // "setKeyFrameKey"
QT_MOC_LITERAL(110, 1470, 5), // "index"
QT_MOC_LITERAL(111, 1476, 27), // "setPlayKeyFramePathStateKey"
QT_MOC_LITERAL(112, 1504, 11), // "buttonState"
QT_MOC_LITERAL(113, 1516, 19), // "setPlayPathStateKey"
QT_MOC_LITERAL(114, 1536, 22), // "setAddKeyFrameStateKey"
QT_MOC_LITERAL(115, 1559, 10), // "setPathKey"
QT_MOC_LITERAL(116, 1570, 28), // "setPlayPathKeyboardModifiers"
QT_MOC_LITERAL(117, 1599, 21), // "Qt::KeyboardModifiers"
QT_MOC_LITERAL(118, 1621, 9), // "modifiers"
QT_MOC_LITERAL(119, 1631, 31), // "setAddKeyFrameKeyboardModifiers"
QT_MOC_LITERAL(120, 1663, 15), // "setMouseBinding"
QT_MOC_LITERAL(121, 1679, 5), // "state"
QT_MOC_LITERAL(122, 1685, 12), // "MouseHandler"
QT_MOC_LITERAL(123, 1698, 7), // "handler"
QT_MOC_LITERAL(124, 1706, 11), // "MouseAction"
QT_MOC_LITERAL(125, 1718, 14), // "withConstraint"
QT_MOC_LITERAL(126, 1733, 11), // "ClickAction"
QT_MOC_LITERAL(127, 1745, 11), // "doubleClick"
QT_MOC_LITERAL(128, 1757, 16), // "Qt::MouseButtons"
QT_MOC_LITERAL(129, 1774, 13), // "buttonsBefore"
QT_MOC_LITERAL(130, 1788, 26), // "setMouseBindingDescription"
QT_MOC_LITERAL(131, 1815, 15), // "Qt::MouseButton"
QT_MOC_LITERAL(132, 1831, 7), // "buttons"
QT_MOC_LITERAL(133, 1839, 6), // "button"
QT_MOC_LITERAL(134, 1846, 15), // "setWheelBinding"
QT_MOC_LITERAL(135, 1862, 7), // "Qt::Key"
QT_MOC_LITERAL(136, 1870, 18), // "clearMouseBindings"
QT_MOC_LITERAL(137, 1889, 11), // "wheelAction"
QT_MOC_LITERAL(138, 1901, 12), // "wheelHandler"
QT_MOC_LITERAL(139, 1914, 27), // "setHandlerKeyboardModifiers"
QT_MOC_LITERAL(140, 1942, 18), // "setHandlerStateKey"
QT_MOC_LITERAL(141, 1961, 16), // "setMouseStateKey"
QT_MOC_LITERAL(142, 1978, 18), // "initFromDOMElement"
QT_MOC_LITERAL(143, 1997, 11), // "QDomElement"
QT_MOC_LITERAL(144, 2009, 7), // "element"
QT_MOC_LITERAL(145, 2017, 15), // "saveStateToFile"
QT_MOC_LITERAL(146, 2033, 20), // "restoreStateFromFile"
QT_MOC_LITERAL(147, 2054, 16), // "setStateFileName"
QT_MOC_LITERAL(148, 2071, 10), // "saveToFile"
QT_MOC_LITERAL(149, 2082, 15), // "restoreFromFile"
QT_MOC_LITERAL(150, 2098, 16), // "resetVisualHints"
QT_MOC_LITERAL(151, 2115, 17), // "delayedFullScreen"
QT_MOC_LITERAL(152, 2133, 11) // "hideMessage"

    },
    "QGLViewer\0viewerInitialized\0\0drawNeeded\0"
    "drawFinished\0automatic\0animateNeeded\0"
    "helpRequired\0axisIsDrawnChanged\0drawn\0"
    "gridIsDrawnChanged\0FPSIsDisplayedChanged\0"
    "displayed\0textIsEnabledChanged\0enabled\0"
    "cameraIsEditedChanged\0edited\0stereoChanged\0"
    "on\0pointSelected\0const QMouseEvent*\0"
    "e\0mouseGrabberChanged\0qglviewer::MouseGrabber*\0"
    "mouseGrabber\0setAxisIsDrawn\0draw\0"
    "setGridIsDrawn\0setFPSIsDisplayed\0"
    "display\0setTextIsEnabled\0enable\0"
    "setCameraIsEdited\0edit\0toggleAxisIsDrawn\0"
    "toggleGridIsDrawn\0toggleFPSIsDisplayed\0"
    "toggleTextIsEnabled\0toggleCameraIsEdited\0"
    "setBackgroundColor\0color\0setForegroundColor\0"
    "setSceneRadius\0radius\0setSceneCenter\0"
    "qglviewer::Vec\0center\0setSceneBoundingBox\0"
    "min\0max\0showEntireScene\0setCamera\0"
    "qglviewer::Camera*const\0camera\0"
    "setManipulatedFrame\0qglviewer::ManipulatedFrame*\0"
    "frame\0setMouseGrabber\0setFullScreen\0"
    "fullScreen\0setStereoDisplay\0stereo\0"
    "toggleFullScreen\0toggleStereoDisplay\0"
    "toggleCameraMode\0saveSnapshot\0overwrite\0"
    "fileName\0setSnapshotFileName\0name\0"
    "setSnapshotFormat\0format\0setSnapshotCounter\0"
    "counter\0setSnapshotQuality\0quality\0"
    "openSnapshotFormatDialog\0snapshotToClipboard\0"
    "copyBufferToTexture\0GLint\0internalFormat\0"
    "GLenum\0setAnimationPeriod\0period\0"
    "startAnimation\0stopAnimation\0animate\0"
    "toggleAnimation\0help\0aboutQGLViewer\0"
    "select\0event\0point\0setSelectBufferSize\0"
    "size\0setSelectRegionWidth\0width\0"
    "setSelectRegionHeight\0height\0"
    "setSelectedName\0id\0setShortcut\0"
    "KeyboardAction\0action\0key\0"
    "setKeyboardAccelerator\0setKeyDescription\0"
    "description\0clearShortcuts\0setKeyFrameKey\0"
    "index\0setPlayKeyFramePathStateKey\0"
    "buttonState\0setPlayPathStateKey\0"
    "setAddKeyFrameStateKey\0setPathKey\0"
    "setPlayPathKeyboardModifiers\0"
    "Qt::KeyboardModifiers\0modifiers\0"
    "setAddKeyFrameKeyboardModifiers\0"
    "setMouseBinding\0state\0MouseHandler\0"
    "handler\0MouseAction\0withConstraint\0"
    "ClickAction\0doubleClick\0Qt::MouseButtons\0"
    "buttonsBefore\0setMouseBindingDescription\0"
    "Qt::MouseButton\0buttons\0button\0"
    "setWheelBinding\0Qt::Key\0clearMouseBindings\0"
    "wheelAction\0wheelHandler\0"
    "setHandlerKeyboardModifiers\0"
    "setHandlerStateKey\0setMouseStateKey\0"
    "initFromDOMElement\0QDomElement\0element\0"
    "saveStateToFile\0restoreStateFromFile\0"
    "setStateFileName\0saveToFile\0restoreFromFile\0"
    "resetVisualHints\0delayedFullScreen\0"
    "hideMessage"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QGLViewer[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
     127,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      13,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,  649,    2, 0x06 /* Public */,
       3,    0,  650,    2, 0x06 /* Public */,
       4,    1,  651,    2, 0x06 /* Public */,
       6,    0,  654,    2, 0x06 /* Public */,
       7,    0,  655,    2, 0x06 /* Public */,
       8,    1,  656,    2, 0x06 /* Public */,
      10,    1,  659,    2, 0x06 /* Public */,
      11,    1,  662,    2, 0x06 /* Public */,
      13,    1,  665,    2, 0x06 /* Public */,
      15,    1,  668,    2, 0x06 /* Public */,
      17,    1,  671,    2, 0x06 /* Public */,
      19,    1,  674,    2, 0x06 /* Public */,
      22,    1,  677,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      25,    1,  680,    2, 0x0a /* Public */,
      25,    0,  683,    2, 0x2a /* Public | MethodCloned */,
      27,    1,  684,    2, 0x0a /* Public */,
      27,    0,  687,    2, 0x2a /* Public | MethodCloned */,
      28,    1,  688,    2, 0x0a /* Public */,
      28,    0,  691,    2, 0x2a /* Public | MethodCloned */,
      30,    1,  692,    2, 0x0a /* Public */,
      30,    0,  695,    2, 0x2a /* Public | MethodCloned */,
      32,    1,  696,    2, 0x0a /* Public */,
      32,    0,  699,    2, 0x2a /* Public | MethodCloned */,
      34,    0,  700,    2, 0x0a /* Public */,
      35,    0,  701,    2, 0x0a /* Public */,
      36,    0,  702,    2, 0x0a /* Public */,
      37,    0,  703,    2, 0x0a /* Public */,
      38,    0,  704,    2, 0x0a /* Public */,
      39,    1,  705,    2, 0x0a /* Public */,
      41,    1,  708,    2, 0x0a /* Public */,
      42,    1,  711,    2, 0x0a /* Public */,
      44,    1,  714,    2, 0x0a /* Public */,
      47,    2,  717,    2, 0x0a /* Public */,
      50,    0,  722,    2, 0x0a /* Public */,
      51,    1,  723,    2, 0x0a /* Public */,
      54,    1,  726,    2, 0x0a /* Public */,
      57,    1,  729,    2, 0x0a /* Public */,
      58,    1,  732,    2, 0x0a /* Public */,
      58,    0,  735,    2, 0x2a /* Public | MethodCloned */,
      60,    1,  736,    2, 0x0a /* Public */,
      60,    0,  739,    2, 0x2a /* Public | MethodCloned */,
      62,    0,  740,    2, 0x0a /* Public */,
      63,    0,  741,    2, 0x0a /* Public */,
      64,    0,  742,    2, 0x0a /* Public */,
      65,    2,  743,    2, 0x0a /* Public */,
      65,    1,  748,    2, 0x2a /* Public | MethodCloned */,
      65,    0,  751,    2, 0x2a /* Public | MethodCloned */,
      65,    2,  752,    2, 0x0a /* Public */,
      65,    1,  757,    2, 0x2a /* Public | MethodCloned */,
      68,    1,  760,    2, 0x0a /* Public */,
      70,    1,  763,    2, 0x0a /* Public */,
      72,    1,  766,    2, 0x0a /* Public */,
      74,    1,  769,    2, 0x0a /* Public */,
      76,    0,  772,    2, 0x0a /* Public */,
      77,    0,  773,    2, 0x0a /* Public */,
      78,    2,  774,    2, 0x0a /* Public */,
      78,    1,  779,    2, 0x2a /* Public | MethodCloned */,
      82,    1,  782,    2, 0x0a /* Public */,
      84,    0,  785,    2, 0x0a /* Public */,
      85,    0,  786,    2, 0x0a /* Public */,
      86,    0,  787,    2, 0x0a /* Public */,
      87,    0,  788,    2, 0x0a /* Public */,
      88,    0,  789,    2, 0x0a /* Public */,
      89,    0,  790,    2, 0x0a /* Public */,
      90,    1,  791,    2, 0x0a /* Public */,
      90,    1,  794,    2, 0x0a /* Public */,
      93,    1,  797,    2, 0x0a /* Public */,
      95,    1,  800,    2, 0x0a /* Public */,
      97,    1,  803,    2, 0x0a /* Public */,
      99,    1,  806,    2, 0x0a /* Public */,
     101,    2,  809,    2, 0x0a /* Public */,
     105,    2,  814,    2, 0x0a /* Public */,
     106,    2,  819,    2, 0x0a /* Public */,
     108,    0,  824,    2, 0x0a /* Public */,
     109,    2,  825,    2, 0x0a /* Public */,
     111,    1,  830,    2, 0x0a /* Public */,
     113,    1,  833,    2, 0x0a /* Public */,
     114,    1,  836,    2, 0x0a /* Public */,
     115,    2,  839,    2, 0x0a /* Public */,
     115,    1,  844,    2, 0x2a /* Public | MethodCloned */,
     116,    1,  847,    2, 0x0a /* Public */,
     119,    1,  850,    2, 0x0a /* Public */,
     120,    4,  853,    2, 0x0a /* Public */,
     120,    3,  862,    2, 0x2a /* Public | MethodCloned */,
     120,    4,  869,    2, 0x0a /* Public */,
     120,    3,  878,    2, 0x2a /* Public | MethodCloned */,
     120,    2,  885,    2, 0x2a /* Public | MethodCloned */,
     130,    4,  890,    2, 0x0a /* Public */,
     130,    3,  899,    2, 0x2a /* Public | MethodCloned */,
     130,    2,  906,    2, 0x2a /* Public | MethodCloned */,
     120,    5,  911,    2, 0x0a /* Public */,
     120,    4,  922,    2, 0x2a /* Public | MethodCloned */,
     120,    5,  931,    2, 0x0a /* Public */,
     120,    4,  942,    2, 0x2a /* Public | MethodCloned */,
     120,    3,  951,    2, 0x2a /* Public | MethodCloned */,
     134,    4,  958,    2, 0x0a /* Public */,
     134,    3,  967,    2, 0x2a /* Public | MethodCloned */,
     130,    5,  974,    2, 0x0a /* Public */,
     130,    4,  985,    2, 0x2a /* Public | MethodCloned */,
     130,    3,  994,    2, 0x2a /* Public | MethodCloned */,
     120,    6, 1001,    2, 0x0a /* Public */,
     120,    5, 1014,    2, 0x2a /* Public | MethodCloned */,
     120,    6, 1025,    2, 0x0a /* Public */,
     120,    5, 1038,    2, 0x2a /* Public | MethodCloned */,
     120,    4, 1049,    2, 0x2a /* Public | MethodCloned */,
     134,    5, 1058,    2, 0x0a /* Public */,
     134,    4, 1069,    2, 0x2a /* Public | MethodCloned */,
     130,    6, 1078,    2, 0x0a /* Public */,
     130,    5, 1091,    2, 0x2a /* Public | MethodCloned */,
     130,    4, 1102,    2, 0x2a /* Public | MethodCloned */,
     136,    0, 1111,    2, 0x0a /* Public */,
     137,    1, 1112,    2, 0x0a /* Public */,
     138,    1, 1115,    2, 0x0a /* Public */,
     139,    2, 1118,    2, 0x0a /* Public */,
     140,    2, 1123,    2, 0x0a /* Public */,
     141,    2, 1128,    2, 0x0a /* Public */,
     142,    1, 1133,    2, 0x0a /* Public */,
     145,    0, 1136,    2, 0x0a /* Public */,
     146,    0, 1137,    2, 0x0a /* Public */,
     147,    1, 1138,    2, 0x0a /* Public */,
     148,    1, 1141,    2, 0x0a /* Public */,
     148,    0, 1144,    2, 0x2a /* Public | MethodCloned */,
     149,    1, 1145,    2, 0x0a /* Public */,
     149,    0, 1148,    2, 0x2a /* Public | MethodCloned */,
     150,    0, 1149,    2, 0x0a /* Public */,
     151,    0, 1150,    2, 0x08 /* Private */,
     152,    0, 1151,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    9,
    QMetaType::Void, QMetaType::Bool,    9,
    QMetaType::Void, QMetaType::Bool,   12,
    QMetaType::Void, QMetaType::Bool,   14,
    QMetaType::Void, QMetaType::Bool,   16,
    QMetaType::Void, QMetaType::Bool,   18,
    QMetaType::Void, 0x80000000 | 20,   21,
    QMetaType::Void, 0x80000000 | 23,   24,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,   26,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   26,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   29,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   31,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   33,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QColor,   40,
    QMetaType::Void, QMetaType::QColor,   40,
    QMetaType::Void, QMetaType::QReal,   43,
    QMetaType::Void, 0x80000000 | 45,   46,
    QMetaType::Void, 0x80000000 | 45, 0x80000000 | 45,   48,   49,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 52,   53,
    QMetaType::Void, 0x80000000 | 55,   56,
    QMetaType::Void, 0x80000000 | 23,   24,
    QMetaType::Void, QMetaType::Bool,   59,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   61,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool, QMetaType::Bool,    5,   66,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,   67,   66,
    QMetaType::Void, QMetaType::QString,   67,
    QMetaType::Void, QMetaType::QString,   69,
    QMetaType::Void, QMetaType::QString,   71,
    QMetaType::Void, QMetaType::Int,   73,
    QMetaType::Void, QMetaType::Int,   75,
    QMetaType::Bool,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 79, 0x80000000 | 81,   80,   71,
    QMetaType::Void, 0x80000000 | 79,   80,
    QMetaType::Void, QMetaType::Int,   83,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 20,   91,
    QMetaType::Void, QMetaType::QPoint,   92,
    QMetaType::Void, QMetaType::Int,   94,
    QMetaType::Void, QMetaType::Int,   96,
    QMetaType::Void, QMetaType::Int,   98,
    QMetaType::Void, QMetaType::Int,  100,
    QMetaType::Void, 0x80000000 | 102, QMetaType::UInt,  103,  104,
    QMetaType::Void, 0x80000000 | 102, QMetaType::UInt,  103,  104,
    QMetaType::Void, QMetaType::UInt, QMetaType::QString,  104,  107,
    QMetaType::Void,
    QMetaType::Void, QMetaType::UInt, QMetaType::Int,  110,  104,
    QMetaType::Void, QMetaType::UInt,  112,
    QMetaType::Void, QMetaType::UInt,  112,
    QMetaType::Void, QMetaType::UInt,  112,
    QMetaType::Void, QMetaType::Int, QMetaType::UInt,  104,  110,
    QMetaType::Void, QMetaType::Int,  104,
    QMetaType::Void, 0x80000000 | 117,  118,
    QMetaType::Void, 0x80000000 | 117,  118,
    QMetaType::Void, QMetaType::UInt, 0x80000000 | 122, 0x80000000 | 124, QMetaType::Bool,  121,  123,  103,  125,
    QMetaType::Void, QMetaType::UInt, 0x80000000 | 122, 0x80000000 | 124,  121,  123,  103,
    QMetaType::Void, QMetaType::UInt, 0x80000000 | 126, QMetaType::Bool, 0x80000000 | 128,  121,  103,  127,  129,
    QMetaType::Void, QMetaType::UInt, 0x80000000 | 126, QMetaType::Bool,  121,  103,  127,
    QMetaType::Void, QMetaType::UInt, 0x80000000 | 126,  121,  103,
    QMetaType::Void, QMetaType::UInt, QMetaType::QString, QMetaType::Bool, 0x80000000 | 128,  121,  107,  127,  129,
    QMetaType::Void, QMetaType::UInt, QMetaType::QString, QMetaType::Bool,  121,  107,  127,
    QMetaType::Void, QMetaType::UInt, QMetaType::QString,  121,  107,
    QMetaType::Void, 0x80000000 | 117, 0x80000000 | 131, 0x80000000 | 122, 0x80000000 | 124, QMetaType::Bool,  118,  132,  123,  103,  125,
    QMetaType::Void, 0x80000000 | 117, 0x80000000 | 131, 0x80000000 | 122, 0x80000000 | 124,  118,  132,  123,  103,
    QMetaType::Void, 0x80000000 | 117, 0x80000000 | 131, 0x80000000 | 126, QMetaType::Bool, 0x80000000 | 128,  118,  133,  103,  127,  129,
    QMetaType::Void, 0x80000000 | 117, 0x80000000 | 131, 0x80000000 | 126, QMetaType::Bool,  118,  133,  103,  127,
    QMetaType::Void, 0x80000000 | 117, 0x80000000 | 131, 0x80000000 | 126,  118,  133,  103,
    QMetaType::Void, 0x80000000 | 117, 0x80000000 | 122, 0x80000000 | 124, QMetaType::Bool,  118,  123,  103,  125,
    QMetaType::Void, 0x80000000 | 117, 0x80000000 | 122, 0x80000000 | 124,  118,  123,  103,
    QMetaType::Void, 0x80000000 | 117, 0x80000000 | 131, QMetaType::QString, QMetaType::Bool, 0x80000000 | 128,  118,  133,  107,  127,  129,
    QMetaType::Void, 0x80000000 | 117, 0x80000000 | 131, QMetaType::QString, QMetaType::Bool,  118,  133,  107,  127,
    QMetaType::Void, 0x80000000 | 117, 0x80000000 | 131, QMetaType::QString,  118,  133,  107,
    QMetaType::Void, 0x80000000 | 135, 0x80000000 | 117, 0x80000000 | 131, 0x80000000 | 122, 0x80000000 | 124, QMetaType::Bool,  104,  118,  132,  123,  103,  125,
    QMetaType::Void, 0x80000000 | 135, 0x80000000 | 117, 0x80000000 | 131, 0x80000000 | 122, 0x80000000 | 124,  104,  118,  132,  123,  103,
    QMetaType::Void, 0x80000000 | 135, 0x80000000 | 117, 0x80000000 | 131, 0x80000000 | 126, QMetaType::Bool, 0x80000000 | 128,  104,  118,  133,  103,  127,  129,
    QMetaType::Void, 0x80000000 | 135, 0x80000000 | 117, 0x80000000 | 131, 0x80000000 | 126, QMetaType::Bool,  104,  118,  133,  103,  127,
    QMetaType::Void, 0x80000000 | 135, 0x80000000 | 117, 0x80000000 | 131, 0x80000000 | 126,  104,  118,  133,  103,
    QMetaType::Void, 0x80000000 | 135, 0x80000000 | 117, 0x80000000 | 122, 0x80000000 | 124, QMetaType::Bool,  104,  118,  123,  103,  125,
    QMetaType::Void, 0x80000000 | 135, 0x80000000 | 117, 0x80000000 | 122, 0x80000000 | 124,  104,  118,  123,  103,
    QMetaType::Void, 0x80000000 | 135, 0x80000000 | 117, 0x80000000 | 131, QMetaType::QString, QMetaType::Bool, 0x80000000 | 128,  104,  118,  133,  107,  127,  129,
    QMetaType::Void, 0x80000000 | 135, 0x80000000 | 117, 0x80000000 | 131, QMetaType::QString, QMetaType::Bool,  104,  118,  133,  107,  127,
    QMetaType::Void, 0x80000000 | 135, 0x80000000 | 117, 0x80000000 | 131, QMetaType::QString,  104,  118,  133,  107,
    QMetaType::Void,
    0x80000000 | 124, 0x80000000 | 117,  118,
    QMetaType::Int, 0x80000000 | 117,  118,
    QMetaType::Void, 0x80000000 | 122, 0x80000000 | 117,  123,  118,
    QMetaType::Void, 0x80000000 | 122, QMetaType::UInt,  123,  112,
    QMetaType::Void, 0x80000000 | 122, QMetaType::UInt,  123,  112,
    QMetaType::Void, 0x80000000 | 143,  144,
    QMetaType::Void,
    QMetaType::Bool,
    QMetaType::Void, QMetaType::QString,   69,
    QMetaType::Void, QMetaType::QString,   67,
    QMetaType::Void,
    QMetaType::Bool, QMetaType::QString,   67,
    QMetaType::Bool,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void QGLViewer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<QGLViewer *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->viewerInitialized(); break;
        case 1: _t->drawNeeded(); break;
        case 2: _t->drawFinished((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->animateNeeded(); break;
        case 4: _t->helpRequired(); break;
        case 5: _t->axisIsDrawnChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->gridIsDrawnChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 7: _t->FPSIsDisplayedChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->textIsEnabledChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->cameraIsEditedChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 10: _t->stereoChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 11: _t->pointSelected((*reinterpret_cast< const QMouseEvent*(*)>(_a[1]))); break;
        case 12: _t->mouseGrabberChanged((*reinterpret_cast< qglviewer::MouseGrabber*(*)>(_a[1]))); break;
        case 13: _t->setAxisIsDrawn((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 14: _t->setAxisIsDrawn(); break;
        case 15: _t->setGridIsDrawn((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 16: _t->setGridIsDrawn(); break;
        case 17: _t->setFPSIsDisplayed((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 18: _t->setFPSIsDisplayed(); break;
        case 19: _t->setTextIsEnabled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 20: _t->setTextIsEnabled(); break;
        case 21: _t->setCameraIsEdited((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 22: _t->setCameraIsEdited(); break;
        case 23: _t->toggleAxisIsDrawn(); break;
        case 24: _t->toggleGridIsDrawn(); break;
        case 25: _t->toggleFPSIsDisplayed(); break;
        case 26: _t->toggleTextIsEnabled(); break;
        case 27: _t->toggleCameraIsEdited(); break;
        case 28: _t->setBackgroundColor((*reinterpret_cast< const QColor(*)>(_a[1]))); break;
        case 29: _t->setForegroundColor((*reinterpret_cast< const QColor(*)>(_a[1]))); break;
        case 30: _t->setSceneRadius((*reinterpret_cast< qreal(*)>(_a[1]))); break;
        case 31: _t->setSceneCenter((*reinterpret_cast< const qglviewer::Vec(*)>(_a[1]))); break;
        case 32: _t->setSceneBoundingBox((*reinterpret_cast< const qglviewer::Vec(*)>(_a[1])),(*reinterpret_cast< const qglviewer::Vec(*)>(_a[2]))); break;
        case 33: _t->showEntireScene(); break;
        case 34: _t->setCamera((*reinterpret_cast< qglviewer::Camera*const(*)>(_a[1]))); break;
        case 35: _t->setManipulatedFrame((*reinterpret_cast< qglviewer::ManipulatedFrame*(*)>(_a[1]))); break;
        case 36: _t->setMouseGrabber((*reinterpret_cast< qglviewer::MouseGrabber*(*)>(_a[1]))); break;
        case 37: _t->setFullScreen((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 38: _t->setFullScreen(); break;
        case 39: _t->setStereoDisplay((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 40: _t->setStereoDisplay(); break;
        case 41: _t->toggleFullScreen(); break;
        case 42: _t->toggleStereoDisplay(); break;
        case 43: _t->toggleCameraMode(); break;
        case 44: _t->saveSnapshot((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 45: _t->saveSnapshot((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 46: _t->saveSnapshot(); break;
        case 47: _t->saveSnapshot((*reinterpret_cast< const QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 48: _t->saveSnapshot((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 49: _t->setSnapshotFileName((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 50: _t->setSnapshotFormat((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 51: _t->setSnapshotCounter((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 52: _t->setSnapshotQuality((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 53: { bool _r = _t->openSnapshotFormatDialog();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 54: _t->snapshotToClipboard(); break;
        case 55: _t->copyBufferToTexture((*reinterpret_cast< GLint(*)>(_a[1])),(*reinterpret_cast< GLenum(*)>(_a[2]))); break;
        case 56: _t->copyBufferToTexture((*reinterpret_cast< GLint(*)>(_a[1]))); break;
        case 57: _t->setAnimationPeriod((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 58: _t->startAnimation(); break;
        case 59: _t->stopAnimation(); break;
        case 60: _t->animate(); break;
        case 61: _t->toggleAnimation(); break;
        case 62: _t->help(); break;
        case 63: _t->aboutQGLViewer(); break;
        case 64: _t->select((*reinterpret_cast< const QMouseEvent*(*)>(_a[1]))); break;
        case 65: _t->select((*reinterpret_cast< const QPoint(*)>(_a[1]))); break;
        case 66: _t->setSelectBufferSize((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 67: _t->setSelectRegionWidth((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 68: _t->setSelectRegionHeight((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 69: _t->setSelectedName((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 70: _t->setShortcut((*reinterpret_cast< KeyboardAction(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2]))); break;
        case 71: _t->setKeyboardAccelerator((*reinterpret_cast< KeyboardAction(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2]))); break;
        case 72: _t->setKeyDescription((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 73: _t->clearShortcuts(); break;
        case 74: _t->setKeyFrameKey((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 75: _t->setPlayKeyFramePathStateKey((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 76: _t->setPlayPathStateKey((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 77: _t->setAddKeyFrameStateKey((*reinterpret_cast< uint(*)>(_a[1]))); break;
        case 78: _t->setPathKey((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2]))); break;
        case 79: _t->setPathKey((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 80: _t->setPlayPathKeyboardModifiers((*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[1]))); break;
        case 81: _t->setAddKeyFrameKeyboardModifiers((*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[1]))); break;
        case 82: _t->setMouseBinding((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< MouseHandler(*)>(_a[2])),(*reinterpret_cast< MouseAction(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4]))); break;
        case 83: _t->setMouseBinding((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< MouseHandler(*)>(_a[2])),(*reinterpret_cast< MouseAction(*)>(_a[3]))); break;
        case 84: _t->setMouseBinding((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< ClickAction(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3])),(*reinterpret_cast< Qt::MouseButtons(*)>(_a[4]))); break;
        case 85: _t->setMouseBinding((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< ClickAction(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 86: _t->setMouseBinding((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< ClickAction(*)>(_a[2]))); break;
        case 87: _t->setMouseBindingDescription((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3])),(*reinterpret_cast< Qt::MouseButtons(*)>(_a[4]))); break;
        case 88: _t->setMouseBindingDescription((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 89: _t->setMouseBindingDescription((*reinterpret_cast< uint(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        case 90: _t->setMouseBinding((*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[1])),(*reinterpret_cast< Qt::MouseButton(*)>(_a[2])),(*reinterpret_cast< MouseHandler(*)>(_a[3])),(*reinterpret_cast< MouseAction(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        case 91: _t->setMouseBinding((*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[1])),(*reinterpret_cast< Qt::MouseButton(*)>(_a[2])),(*reinterpret_cast< MouseHandler(*)>(_a[3])),(*reinterpret_cast< MouseAction(*)>(_a[4]))); break;
        case 92: _t->setMouseBinding((*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[1])),(*reinterpret_cast< Qt::MouseButton(*)>(_a[2])),(*reinterpret_cast< ClickAction(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4])),(*reinterpret_cast< Qt::MouseButtons(*)>(_a[5]))); break;
        case 93: _t->setMouseBinding((*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[1])),(*reinterpret_cast< Qt::MouseButton(*)>(_a[2])),(*reinterpret_cast< ClickAction(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4]))); break;
        case 94: _t->setMouseBinding((*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[1])),(*reinterpret_cast< Qt::MouseButton(*)>(_a[2])),(*reinterpret_cast< ClickAction(*)>(_a[3]))); break;
        case 95: _t->setWheelBinding((*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[1])),(*reinterpret_cast< MouseHandler(*)>(_a[2])),(*reinterpret_cast< MouseAction(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4]))); break;
        case 96: _t->setWheelBinding((*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[1])),(*reinterpret_cast< MouseHandler(*)>(_a[2])),(*reinterpret_cast< MouseAction(*)>(_a[3]))); break;
        case 97: _t->setMouseBindingDescription((*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[1])),(*reinterpret_cast< Qt::MouseButton(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4])),(*reinterpret_cast< Qt::MouseButtons(*)>(_a[5]))); break;
        case 98: _t->setMouseBindingDescription((*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[1])),(*reinterpret_cast< Qt::MouseButton(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4]))); break;
        case 99: _t->setMouseBindingDescription((*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[1])),(*reinterpret_cast< Qt::MouseButton(*)>(_a[2])),(*reinterpret_cast< QString(*)>(_a[3]))); break;
        case 100: _t->setMouseBinding((*reinterpret_cast< Qt::Key(*)>(_a[1])),(*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[2])),(*reinterpret_cast< Qt::MouseButton(*)>(_a[3])),(*reinterpret_cast< MouseHandler(*)>(_a[4])),(*reinterpret_cast< MouseAction(*)>(_a[5])),(*reinterpret_cast< bool(*)>(_a[6]))); break;
        case 101: _t->setMouseBinding((*reinterpret_cast< Qt::Key(*)>(_a[1])),(*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[2])),(*reinterpret_cast< Qt::MouseButton(*)>(_a[3])),(*reinterpret_cast< MouseHandler(*)>(_a[4])),(*reinterpret_cast< MouseAction(*)>(_a[5]))); break;
        case 102: _t->setMouseBinding((*reinterpret_cast< Qt::Key(*)>(_a[1])),(*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[2])),(*reinterpret_cast< Qt::MouseButton(*)>(_a[3])),(*reinterpret_cast< ClickAction(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5])),(*reinterpret_cast< Qt::MouseButtons(*)>(_a[6]))); break;
        case 103: _t->setMouseBinding((*reinterpret_cast< Qt::Key(*)>(_a[1])),(*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[2])),(*reinterpret_cast< Qt::MouseButton(*)>(_a[3])),(*reinterpret_cast< ClickAction(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        case 104: _t->setMouseBinding((*reinterpret_cast< Qt::Key(*)>(_a[1])),(*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[2])),(*reinterpret_cast< Qt::MouseButton(*)>(_a[3])),(*reinterpret_cast< ClickAction(*)>(_a[4]))); break;
        case 105: _t->setWheelBinding((*reinterpret_cast< Qt::Key(*)>(_a[1])),(*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[2])),(*reinterpret_cast< MouseHandler(*)>(_a[3])),(*reinterpret_cast< MouseAction(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        case 106: _t->setWheelBinding((*reinterpret_cast< Qt::Key(*)>(_a[1])),(*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[2])),(*reinterpret_cast< MouseHandler(*)>(_a[3])),(*reinterpret_cast< MouseAction(*)>(_a[4]))); break;
        case 107: _t->setMouseBindingDescription((*reinterpret_cast< Qt::Key(*)>(_a[1])),(*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[2])),(*reinterpret_cast< Qt::MouseButton(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5])),(*reinterpret_cast< Qt::MouseButtons(*)>(_a[6]))); break;
        case 108: _t->setMouseBindingDescription((*reinterpret_cast< Qt::Key(*)>(_a[1])),(*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[2])),(*reinterpret_cast< Qt::MouseButton(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4])),(*reinterpret_cast< bool(*)>(_a[5]))); break;
        case 109: _t->setMouseBindingDescription((*reinterpret_cast< Qt::Key(*)>(_a[1])),(*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[2])),(*reinterpret_cast< Qt::MouseButton(*)>(_a[3])),(*reinterpret_cast< QString(*)>(_a[4]))); break;
        case 110: _t->clearMouseBindings(); break;
        case 111: { MouseAction _r = _t->wheelAction((*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< MouseAction*>(_a[0]) = std::move(_r); }  break;
        case 112: { int _r = _t->wheelHandler((*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        case 113: _t->setHandlerKeyboardModifiers((*reinterpret_cast< MouseHandler(*)>(_a[1])),(*reinterpret_cast< Qt::KeyboardModifiers(*)>(_a[2]))); break;
        case 114: _t->setHandlerStateKey((*reinterpret_cast< MouseHandler(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2]))); break;
        case 115: _t->setMouseStateKey((*reinterpret_cast< MouseHandler(*)>(_a[1])),(*reinterpret_cast< uint(*)>(_a[2]))); break;
        case 116: _t->initFromDOMElement((*reinterpret_cast< const QDomElement(*)>(_a[1]))); break;
        case 117: _t->saveStateToFile(); break;
        case 118: { bool _r = _t->restoreStateFromFile();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 119: _t->setStateFileName((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 120: _t->saveToFile((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 121: _t->saveToFile(); break;
        case 122: { bool _r = _t->restoreFromFile((*reinterpret_cast< const QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 123: { bool _r = _t->restoreFromFile();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 124: _t->resetVisualHints(); break;
        case 125: _t->delayedFullScreen(); break;
        case 126: _t->hideMessage(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (QGLViewer::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QGLViewer::viewerInitialized)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (QGLViewer::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QGLViewer::drawNeeded)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (QGLViewer::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QGLViewer::drawFinished)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (QGLViewer::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QGLViewer::animateNeeded)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (QGLViewer::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QGLViewer::helpRequired)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (QGLViewer::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QGLViewer::axisIsDrawnChanged)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (QGLViewer::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QGLViewer::gridIsDrawnChanged)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (QGLViewer::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QGLViewer::FPSIsDisplayedChanged)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (QGLViewer::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QGLViewer::textIsEnabledChanged)) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (QGLViewer::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QGLViewer::cameraIsEditedChanged)) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (QGLViewer::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QGLViewer::stereoChanged)) {
                *result = 10;
                return;
            }
        }
        {
            using _t = void (QGLViewer::*)(const QMouseEvent * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QGLViewer::pointSelected)) {
                *result = 11;
                return;
            }
        }
        {
            using _t = void (QGLViewer::*)(qglviewer::MouseGrabber * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QGLViewer::mouseGrabberChanged)) {
                *result = 12;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject QGLViewer::staticMetaObject = { {
    &QGLWidget::staticMetaObject,
    qt_meta_stringdata_QGLViewer.data,
    qt_meta_data_QGLViewer,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *QGLViewer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QGLViewer::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_QGLViewer.stringdata0))
        return static_cast<void*>(this);
    return QGLWidget::qt_metacast(_clname);
}

int QGLViewer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGLWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 127)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 127;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 127)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 127;
    }
    return _id;
}

// SIGNAL 0
void QGLViewer::viewerInitialized()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void QGLViewer::drawNeeded()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void QGLViewer::drawFinished(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void QGLViewer::animateNeeded()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void QGLViewer::helpRequired()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void QGLViewer::axisIsDrawnChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void QGLViewer::gridIsDrawnChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void QGLViewer::FPSIsDisplayedChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void QGLViewer::textIsEnabledChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void QGLViewer::cameraIsEditedChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void QGLViewer::stereoChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void QGLViewer::pointSelected(const QMouseEvent * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 11, _a);
}

// SIGNAL 12
void QGLViewer::mouseGrabberChanged(qglviewer::MouseGrabber * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
