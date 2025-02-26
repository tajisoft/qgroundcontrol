/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#ifndef __mobile__
#pragma once

#include <QObject>
#include <QtTest>
#include <QMessageBox>
#include <QFileDialog>

#include "QGCMAVLink.h"
#include "LinkInterface.h"
#include "Fact.h"
#include "MissionItem.h"
#include "MockLink.h"

#define UT_REGISTER_TEST(className)             static UnitTestWrapper<className> className(#className, false);
#define UT_REGISTER_TEST_STANDALONE(className)  static UnitTestWrapper<className> className(#className, true);  // Test will only be run with specifically called to from command line

class QGCMessageBox;
class QGCQFileDialog;
class LinkManager;
class MockLink;
class Vehicle;

class UnitTest : public QObject
{
    Q_OBJECT

public:
    UnitTest(void);
    virtual ~UnitTest(void);

    /// @brief Called to run all the registered unit tests
    ///     @param singleTest Name of test to just run a single test
    static int run(QString& singleTest);

    /// @brief Sets up for an expected QGCMessageBox
    ///     @param response Response to take on message box
    void setExpectedMessageBox(QMessageBox::StandardButton response);

    /// @brief Types for UnitTest::setExpectedFileDialog
    enum FileDialogType {
        getExistingDirectory,
        getOpenFileName,
        getOpenFileNames,
        getSaveFileName
    };

    /// @brief Sets up for an expected QGCQFileDialog
    ///     @param type Type of expected file dialog
    ///     @param response Files to return from call. Multiple files only supported by getOpenFileNames
    void setExpectedFileDialog(enum FileDialogType type, QStringList response);

    enum {
        expectFailNoFailure =           1 << 0, ///< not expecting any failures
        expectFailNoDialog =            1 << 1, ///< expecting a failure due to no dialog displayed
        expectFailBadResponseButton =   1 << 2, ///< expecting a failure due to bad button response (QGCMessageBox only)
        expectFailWrongFileDialog =     1 << 3  ///< expecting one dialog type, got the wrong type (QGCQFileDialog ony)
    };

    /// @brief Check whether a message box was displayed and correctly responded to
    //          @param Expected failure response flags
    void checkExpectedMessageBox(int expectFailFlags = expectFailNoFailure);

    /// Checks that the specified number of message boxes where shown. Do not call setExpectedMessageBox when using this method.
    void checkMultipleExpectedMessageBox(int messageCount);

    /// @brief Check whether a message box was displayed and correctly responded to
    //          @param Expected failure response flags
    void checkExpectedFileDialog(int expectFailFlags = expectFailNoFailure);

    bool standalone(void) const{ return _standalone; }
    void setStandalone(bool standalone) { _standalone = standalone; }

    /// @brief Adds a unit test to the list. Should only be called by UnitTestWrapper.
    static void _addTest(UnitTest* test);

    /// Will throw qWarning at location where files differ
    /// @return true: files are alike, false: files differ
    static bool fileCompare(const QString& file1, const QString& file2);

    /// Changes the Facts rawValue such that it emits a valueChanged signal.
    ///     @param increment 0 use standard increment, other increment by specified amount if double value
    void changeFactValue(Fact* fact, double increment = 0);

    QGeoCoordinate changeCoordinateValue(const QGeoCoordinate& coordinate);

    /// Returns true is the position of the two coordinates is less then a meter from each other.
    /// Does not check altitude.
    static bool fuzzyCompareLatLon(const QGeoCoordinate& coord1, const QGeoCoordinate& coord2);

protected slots:

    // These are all pure virtuals to force the derived class to implement each one and in turn
    // call the UnitTest private implementation.

    /// @brief Called before each test.
    ///         Make sure to call UnitTest::init first in your derived class.
    virtual void init(void);

    /// @brief Called after each test.
    ///         Make sure to call UnitTest::cleanup last in your derived class.
    virtual void cleanup(void);

protected:
    void _connectMockLink(MAV_AUTOPILOT autopilot = MAV_AUTOPILOT_PX4, MockConfiguration::FailureMode_t failureMode = MockConfiguration::FailNone);
    void _connectMockLinkNoInitialConnectSequence(void) { _connectMockLink(MAV_AUTOPILOT_INVALID); }
    void _disconnectMockLink(void);
    void _missionItemsEqual(MissionItem& actual, MissionItem& expected);

    LinkManager*    _linkManager    = nullptr;
    MockLink*       _mockLink       = nullptr;
    Vehicle*        _vehicle        = nullptr;

    bool _expectMissedFileDialog = false;   // true: expect a missed file dialog, used for internal testing
    bool _expectMissedMessageBox = false;   // true: expect a missed message box, used for internal testing

private slots:
    void _linkDeleted(LinkInterface* link);

private:
    // When the app is running in unit test mode the QGCMessageBox methods are re-routed here.

    static QMessageBox::StandardButton _messageBox(QMessageBox::Icon icon,
                                                   const QString& title,
                                                   const QString& text,
                                                   QMessageBox::StandardButtons buttons,
                                                   QMessageBox::StandardButton defaultButton);

    // This allows the private call to _messageBox
    friend class QGCMessageBox;

    // When the app is running in unit test mode the QGCQFileDialog methods are re-routed here.

    static QString _getExistingDirectory(
        QWidget* parent,
        const QString& caption,
        const QString& dir,
        QFileDialog::Options options);

    static QString _getOpenFileName(
        QWidget* parent,
        const QString& caption,
        const QString& dir,
        const QString& filter,
        QFileDialog::Options options);

    static QStringList _getOpenFileNames(
        QWidget* parent,
        const QString& caption,
        const QString& dir,
        const QString& filter,
        QFileDialog::Options options);

    static QString _getSaveFileName(
        QWidget* parent,
        const QString& caption,
        const QString& dir,
        const QString& filter,
        const QString& defaultSuffix,
        QFileDialog::Options options);

    static QString _fileDialogResponseSingle(enum FileDialogType type);

    // This allows the private calls to the file dialog methods
    friend class QGCQFileDialog;

    void _unitTestCalled(void);
    static QList<UnitTest*>& _testList(void);

    // Catch QGCMessageBox calls
    static bool                         _messageBoxRespondedTo;     ///< Message box was responded to
    static bool                         _badResponseButton;         ///< Attempt to repond to expected message box with button not being displayed
    static QMessageBox::StandardButton  _messageBoxResponseButton;  ///< Response to next message box
    static int                          _missedMessageBoxCount;     ///< Count of message box not checked with call to messageBoxWasDisplayed

    // Catch QGCQFileDialog calls
    static bool         _fileDialogRespondedTo;         ///< File dialog was responded to
    static bool         _fileDialogResponseSet;         ///< true: _fileDialogResponse was set by a call to UnitTest::setExpectedFileDialog
    static QStringList  _fileDialogResponse;            ///< Response to next file dialog
    static enum FileDialogType _fileDialogExpectedType; ///< type of file dialog expected to show
    static int          _missedFileDialogCount;         ///< Count of file dialogs not checked with call to UnitTest::fileDialogWasDisplayed

    bool _unitTestRun   = false;    ///< true: Unit Test was run
    bool _initCalled    = false;    ///< true: UnitTest::_init was called
    bool _cleanupCalled = false;    ///< true: UnitTest::_cleanup was called
    bool _standalone    = false;    ///< true: Only run when requested specifically from command line
};

template <class T>
class UnitTestWrapper {
public:
    UnitTestWrapper(const QString& name, bool standalone)
        : _unitTest(new T)
    {
        _unitTest->setObjectName(name);
        _unitTest->setStandalone(standalone);
        UnitTest::_addTest(_unitTest.data());
    }

private:
    QSharedPointer<T> _unitTest;
};

#endif
