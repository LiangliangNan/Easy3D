#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QImage>
#include <QCoreApplication>

#include <iostream>
#include <limits>


void usage();
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
    for (int i = 0; i < argc; ++i)
        arguments[i] = QString::fromLocal8Bit(argv[i]);

    if (argc < 2) {
        usage();
        return EXIT_SUCCESS;
    }

    //---------------------------------------------------------------------------

    QFileInfo app_info(argv[1]);
    QString app_name = app_info.absoluteFilePath();
    QString app_dir = app_info.absolutePath();
    qDebug() << "Application name:" << app_name;
#ifdef __APPLE__
    if (!app_info.isBundle()) {
        qWarning() << "argv[1] is not a valid application bundle file\n\targv[1] =" << argv[1];
        return EXIT_FAILURE;
    }
#endif

    QString deploy_dir = app_dir + "/deploy";
    const QString output_dir = deploy_dir;
    qDebug() << "Deploy directory:" << output_dir;

    QFileInfo deploy_info(deploy_dir);
    QDir dir(app_dir);
    if (deploy_info.isFile()) {
        qDebug() << deploy_dir << "is a file, deleting it...";
        dir.remove(deploy_dir);
    } else if (deploy_info.isDir()) {
        QDir(deploy_dir).removeRecursively();
        if (QFileInfo(deploy_dir).isDir()) {
            qDebug() << "Deploy directory" << deploy_dir << "already exists, but EasyDeploy failed to delete it";
            return EXIT_FAILURE;
        }
    }

    dir.mkdir(deploy_dir);
    if (!QFileInfo(deploy_dir).isDir()) {
        qWarning() << "Failed creating deploy directory";
        return EXIT_FAILURE;
    } else
        qDebug() << "Successfully created deploy directory" << deploy_dir;

#if (defined(_WIN32) || defined(_WIN64) || defined(__APPLE__))
    QString deployed_app_name = deploy_dir + "/" + app_info.fileName();
    qDebug() << "Copying" << app_info.fileName() << "into" << deploy_dir;
    QFile::copy(app_info.absoluteFilePath(), deployed_app_name);

    arguments[1] = deployed_app_name;
#elif (defined(__linux) || defined(__linux__))
    deploy_dir += "/" + app_info.baseName();
    dir.mkdir(deploy_dir);
    if (!QFileInfo(deploy_dir).isDir()) {
        qWarning() << "Failed creating directory" << deploy_dir;
        return EXIT_FAILURE;
    }

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
    contents += "Version=1.0\n";
    contents += "Type=Application\n";
    contents += "Name=" + app_info.baseName() + "\n";
    contents += "Comment=Deployed by EasyDeploy (liangliang.nan@gmail.com)\n";
    contents += "Exec=" +  app_info.baseName() + "\n";
    contents += "Icon=" +  app_info.baseName() + "\n";
    contents += "Categories=Development;";
    desktopfile.write(contents.toStdString().c_str());
    desktopfile.close();
    dir.cdUp();
    const QString icons_dir = share_dir + "/icons";
    dir.mkdir(icons_dir);
    dir.cd(icons_dir);
    const QString theme_dir = icons_dir + "/hicolor";
    dir.mkdir(theme_dir);
    dir.cd(theme_dir);

    // icon file
    QString icon_image;
    while (!QFileInfo(icon_image).isFile() || !(icon_image.endsWith(".png"))) {
        qWarning() << "IMPORTANT: An icon image (in '.png' format) is needed for your application. Please provide the"
                   << "\n  full path (including file name and extension) to the icon image (resolution >= 256x256)";
        qWarning() << "Current working directory:" << dir.currentPath();
        qWarning() << "Icon image:";
        std::string file;
        std::getline(std::cin, file);

        // assume relative path
        icon_image = dir.currentPath() + "/" + QString::fromStdString(file);
        if (!QFileInfo(icon_image).isFile()) // now try absolute path
            icon_image = QString::fromStdString(file);
    }

    const QImage image(icon_image);
    const int original_width = image.width();
    const int original_height = image.height();
    qDebug() << "Input icon image size" << original_width << "x" << original_height;
    const std::vector<int> sizes = {1024, 512, 256, 128, 64, 32, 16};
    for (auto size : sizes) {
        if (size > original_width || size > original_height) // we allow only downscaling
            continue;
        const auto size_str = std::to_string(size);
        const QString resolution_dir = theme_dir + "/" + QString::fromStdString(size_str + 'x' + size_str);
        dir.mkdir(resolution_dir);
        dir.cd(resolution_dir);
        const QString apps_dir = resolution_dir + "/apps";
        dir.mkdir(apps_dir);
        dir.cd(apps_dir);
        const QString scaled_image = apps_dir + "/" + app_info.baseName() + ".png";
        image.scaled(size, size).save(scaled_image);
    }

    const QString resources_dir = usr_dir + "/resources";
    qWarning() << "IMPORTANT: Do you have any resources necessary for your application [Y/n]?";
    char yes_no;
    std::cin >> yes_no;
    if (yes_no == 'Y' || yes_no == 'y') {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // ignore the previous user "Enter"
        dir.mkdir(resources_dir);
        qWarning() << "  Put all your resources in:" << "\n\t" << resources_dir << "\n"
                   << "  and then press 'Enter' to continue";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    arguments[1] = desktopfile.fileName();
    dir.setCurrent(output_dir); // the AppImage (if requested) will be generated here

    // Adding icon and icon theme support
    // To enable icon and icon theme support you must add iconengines as an extra Qt plugin. In order for your
    // application to locate the system theme icons, the libqgtk3.so platform theme must also be added:
    arguments.push_back(QString("-extra-plugins=iconengines,platformthemes/libqgtk3.so"));
#endif

    qDebug() << "Deploying" << app_info.fileName() << "...";
    int result = deploy(arguments);
    if (result == 0)
        qDebug() << "Done! The deployed application is located at" << output_dir;
    else
        qDebug() << "Error(s) occurred with code " << result;
    return result;
}


void usage() {
#ifdef __APPLE__
    qDebug() << "Usage: EasyDeploy <app-bundle> [options]";
    qDebug() << "";
    qDebug() << "Options:";
    qDebug() << "   -verbose=<0-3>                : 0 = no output, 1 = error/warning (default), 2 = normal, 3 = debug";
    qDebug() << "   -no-plugins                   : Skip plugin deployment";
    qDebug() << "   -dmg                          : Create a .dmg disk image";
    qDebug() << "   -no-strip                     : Don't run 'strip' on the binaries";
    qDebug() << "   -use-debug-libs               : Deploy with debug versions of frameworks and plugins (implies -no-strip)";
    qDebug() << "   -executable=<path>            : Let the given executable use the deployed frameworks too";
    qDebug() << "   -qmldir=<path>                : Scan for QML imports in the given path";
    qDebug() << "   -qmlimport=<path>             : Add the given path to the QML module search locations";
    qDebug() << "   -always-overwrite             : Copy files even if the target file exists";
    qDebug() << "   -codesign=<ident>             : Run codesign with the given identity on all executables";
    qDebug() << "   -hardened-runtime             : Enable Hardened Runtime when code signing";
    qDebug() << "   -timestamp                    : Include a secure timestamp when code signing (requires internet connection)";
    qDebug() << "   -sign-for-notarization=<ident>: Activate the necessary options for notarization (requires internet connection)";
    qDebug() << "   -appstore-compliant           : Skip deployment of components that use private API";
    qDebug() << "   -libpath=<path>               : Add the given path to the library search path";
    qDebug() << "   -fs=<filesystem>              : Set the filesystem used for the .dmg disk image (defaults to HFS+)";
    qDebug() << "";
    qDebug() << "EasyDeploy takes an application bundle as input and makes it";
    qDebug() << "self-contained by copying in the Qt frameworks and plugins that";
    qDebug() << "the application uses.";
    qDebug() << "";
    qDebug() << "Plugins related to a framework are copied in with the";
    qDebug() << "framework. The accessibility, image formats, and text codec";
    qDebug() << "plugins are always copied, unless \"-no-plugins\" is specified.";
    qDebug() << "";
    qDebug() << "Qt plugins may use private API and will cause the app to be";
    qDebug() << "rejected from the Mac App store. EasyDeploy will print a warning";
    qDebug() << "when known incompatible plugins are deployed. Use -appstore-compliant ";
    qDebug() << "to skip these plugins. Currently two SQL plugins are known to";
    qDebug() << "be incompatible: qsqlodbc and qsqlpsql.";
    qDebug() << "";
    qDebug() << "See the \"Deploying Applications on OS X\" topic in the";
    qDebug() << "documentation for more information about deployment on OS X.";
#elif (defined(__linux) || defined(__linux__))
    qDebug() << "Usage: EasyDeploy <app-binary> [options]";
    qDebug() << "";
    qDebug() << "Options:";
    qDebug() << "   -always-overwrite        : Copy files even if the target file exists.";
    qDebug() << "   -appimage                : Create an AppImage (implies -bundle-non-qt-libs).";
    qDebug() << "   -bundle-non-qt-libs      : Also bundle non-core, non-Qt libraries.";
    qDebug() << "   -exclude-libs=<list>     : List of libraries which should be excluded,";
    qDebug() << "                              separated by comma.";
    qDebug() << "   -ignore-glob=<glob>      : Glob pattern relative to appdir to ignore when";
    qDebug() << "                              searching for libraries.";
    qDebug() << "   -executable=<path>       : Let the given executable use the deployed libraries";
    qDebug() << "                              too";
    qDebug() << "   -extra-plugins=<list>    : List of extra plugins which should be deployed,";
    qDebug() << "                              separated by comma.";
    qDebug() << "   -no-copy-copyright-files : Skip deployment of copyright files.";
    qDebug() << "   -no-plugins              : Skip plugin deployment.";
    qDebug() << "   -no-strip                : Don't run 'strip' on the binaries.";
    qDebug() << "   -no-translations         : Skip deployment of translations.";
    qDebug() << "   -qmake=<path>            : The qmake executable to use.";
    qDebug() << "   -qmldir=<path>           : Scan for QML imports in the given path.";
    qDebug() << "   -qmlimport=<path>        : Add the given path to QML module search locations.";
    qDebug() << "   -show-exclude-libs       : Print exclude libraries list.";
    qDebug() << "   -verbose=<0-3>           : 0 = no output, 1 = error/warning (default),";
    qDebug() << "                              2 = normal, 3 = debug.";
    qDebug() << "   -updateinformation=<update string>        : Embed update information STRING; if zsyncmake is installed, generate zsync file";
    qDebug() << "   -version                 : Print version statement and exit.";
    qDebug() << "";
    qDebug() << "EasyDeploy takes an application as input and makes it";
    qDebug() << "self-contained by copying in the Qt libraries and plugins that";
    qDebug() << "the application uses.";
    qDebug() << "";
    qDebug() << "By default it deploys the Qt instance that qmake on the $PATH points to.";
    qDebug() << "The '-qmake' option can be used to point to the qmake executable";
    qDebug() << "to be used instead.";
    qDebug() << "";
    qDebug() << "Plugins related to a Qt library are copied in with the library.";
    /* TODO: To be implemented
    qDebug() << "The accessibility, image formats, and text codec";
    qDebug() << "plugins are always copied, unless \"-no-plugins\" is specified.";
    */
    qDebug() << "";
    qDebug() << "See the \"Deploying Applications on Linux\" topic in the";
    qDebug() << "documentation for more information about deployment on Linux.";
#endif
}

