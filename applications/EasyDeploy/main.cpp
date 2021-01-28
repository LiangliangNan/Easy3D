#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>


int deploy(const std::vector<QString>& argv);

int main(int argc, char **argv)
{
    // QCoreApplication is necessary for using QString
    QCoreApplication app(argc, argv);

#if 0
    argc = 3;
    argv[1] = "/Users/lnan/Projects/Easy3D/cmake-build-release/bin/Mapple.app";
    argv[2] = "-verbose=1";
#endif

    std::vector<QString> arguments(argc);
    for (int i=0; i<argc; ++i)
        arguments[i] =  QString::fromLocal8Bit(argv[i]);

    if (argc < 2) {
        qInfo() << "Usage: \n\tEasyDeploy <executable or bundle file> [options]";
        qInfo() << "Options:";
        qInfo() << "   -verbose=<0-3> :  0 = no output, 1 = error/warning (default), 2 = normal, 3 = debug";
        qInfo() << "   -dmg           :  Create a .dmg disk image [macOS only]";
        qInfo() << "   -appimage      :  Create an .AppImage (implies -bundle-non-qt-libs) [Linux only]";
        return EXIT_SUCCESS;
    }

    //---------------------------------------------------------------------------

    QFileInfo app_info(argv[1]);
    QString app_name = app_info.absoluteFilePath();
    QString app_dir = app_info.absolutePath();
    qInfo() << "application name:" << app_name;
#ifdef __APPLE__
    if (!app_info.isBundle()) {
        qWarning() << "argv[1] is not a valid application bundle file\n\targv[1] =" << argv[1];
        return EXIT_FAILURE;
    }
#endif

    QString deploy_dir = app_dir + "/deploy";
    QFileInfo deploy_info(deploy_dir);
    QDir dir(app_dir);
    if (deploy_info.isFile()) {
        qInfo() << deploy_info.baseName() << "is a file, deleting it...";
        dir.remove(deploy_dir);
    }
    else if (deploy_info.isDir()) {
        qInfo() << deploy_info.baseName() << "directory already exists, deleting it...";
        dir.cd(deploy_dir);
        dir.removeRecursively();
        dir.cdUp();
        dir.remove(deploy_dir);
    }

    qInfo() << "creating directory:" << deploy_dir;
    dir.mkdir(deploy_dir);

#if (defined(_WIN32) || defined(_WIN64) || defined(__APPLE__))
    QString deployed_app_name = deploy_dir + "/" + app_info.fileName();
    qInfo() << "copying" << app_info.fileName() << "into" << deploy_dir;
    QFile::copy(app_info.absoluteFilePath(), deployed_app_name);

    arguments[1] = deployed_app_name;
    qInfo() << "deploying" << deployed_app_name;
    return deploy(arguments);
#elif (defined(__linux) || defined(__linux__))
    dir.cd(deploy_dir);
    const QString usr_dir = deploy_dir + "/usr";
    dir.mkdir(usr_dir);
    dir.cd(usr_dir);
    const QString bin_dir = usr_dir + "/bin";
    dir.mkdir(bin_dir);
    QFile::copy(app_info.absoluteFilePath(), bin_dir + "/" + app_info.fileName());
    dir.mkdir(usr_dir + "/lib");
    const QString share_dir = usr_dir + "/share";
    dir.mkdir(share_dir);
    dir.cd(share_dir);
    const QString applications_dir = share_dir + "/applications";
    dir.mkdir(applications_dir);
    dir.cd(applications_dir);
    QFile desktopfile(applications_dir + "/" + app_info.baseName() + ".desktop");
    desktopfile.open(QFile::WriteOnly);
    QString contents = "[Desktop Entry]\n";
    contents += "Type=Application\n";
    contents += "Name=" + app_info.baseName() + "\n";
    contents += "Comment=The best Qt Application Ever\n";
    contents += "Exec=" +  app_info.baseName() + "\n";
    contents += "Icon=" +  app_info.baseName() + "\n";
    contents += "Categories=Office";
    desktopfile.write(contents.toStdString().c_str());
    desktopfile.close();
    dir.cdUp();
    const QString icons_dir = share_dir + "/icons";
    dir.mkdir(icons_dir);
    dir.cd(icons_dir);
    const QString theme_dir = icons_dir + "/highcolor";
    dir.mkdir(theme_dir);
    dir.cd(theme_dir);
    const QString resolution_dir = theme_dir + "/256x256";
    dir.mkdir(resolution_dir);
    dir.cd(resolution_dir);
    const QString apps_dir = resolution_dir + "/apps";
    dir.mkdir(apps_dir);
    dir.cd(apps_dir);
    QFile iconfile(apps_dir + "/" + app_info.baseName() + ".png");
    iconfile.open(QFile::WriteOnly);
    iconfile.write("Please replace this file with your app icon file");
    iconfile.close();

    arguments[1] = desktopfile.fileName();
    int result = deploy(arguments);
    if (result == 0) {
        qInfo() << "Please replace the following files with with your actual icon image:\n"
                << "\t" << deploy_dir + "/" + app_info.baseName() + ".png" << "\n"
                << "\t" << iconfile.fileName();
    }
    return result;
#endif
}



