#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <string>

#include <QMainWindow>


namespace Ui {
    class MainWindow;
}

namespace easy3d {
    class Model;
}

class ViewerQt;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    ViewerQt* viewer() { return viewer_; }

private slots:

    // file
    bool onOpen();
    bool onSave();
    void onOpenRecentFile();
    void onClearRecentFiles();
    void onCurrentModelChanged();

    // view
    void saveSnapshot();
    void setBackgroundColor();

    // topology
    void reportTopologyStatistics();

    // about
    void onAbout();

protected:
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dropEvent(QDropEvent *) override;
    void closeEvent(QCloseEvent *event) override;
private:
    // Open a file with file name given
    // NOTE: model will not be added to the viewer
    easy3d::Model* open(const std::string& file_name);

private:
    void createActions();

    void createActionsForFileMenu();
    void createActionsForViewMenu();
    void createActionsForTopologyMenu();

    bool okToContinue();
    void readSettings();
    void writeSettings();
    void updateWindowTitle();

    void setCurrentFile(const QString &fileName);

    void updateRecentFileActions();
    QString strippedName(const QString &fullFileName);

private:
    ViewerQt*   viewer_;

    QStringList recentFiles_;
    QString		curDataDirectory_;

    enum { MaxRecentFiles = 5 };
    QAction *actionsRecentFile[MaxRecentFiles],
        *actionSeparator;

private:
    Ui::MainWindow* ui;

};


#endif // MAIN_WINDOW_H
