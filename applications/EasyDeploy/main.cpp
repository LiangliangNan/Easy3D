#include <QDir>
#include <QFileInfo>
#include <QDebug>




int deploy(int argc, char **argv);

int main(int argc, char **argv)
{
#if 0
    argc = 3;
    argv[1] = "/Users/lnan/Projects/Easy3D/cmake-build-release/bin/Mapple.app";
    argv[2] = "-verbose=1";
#endif

    if (argc < 2) {
        qInfo() << "Usage: \n\tEasyDeploy <executable or app-bundle> [options]";
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
    qDebug() << "application name:" << app_name;

    QString deploy_dir = app_dir + "/deploy";
    QFileInfo deploy_info(deploy_dir);
    QDir dir(app_dir);
    if (deploy_info.isFile()) {
        qDebug() << deploy_info.baseName() << "is a file, deleting it...";
        dir.remove(deploy_dir);
    }
    else if (deploy_info.isDir()) {
        qDebug() << deploy_info.baseName() << "directory already exists, deleting it...";
        dir.cd(deploy_dir);
        dir.removeRecursively();
        dir.cdUp();
        dir.remove(deploy_dir);
    }

    qDebug() << "creating directory:" << deploy_dir;
    dir.mkdir(deploy_dir);

    QString deployed_app_name = deploy_dir + "/" + app_info.fileName();
    qDebug() << "copying" << app_info.fileName() << "into" << deploy_dir;
    QFile::copy(app_info.absoluteFilePath(), deployed_app_name);

    qDebug() << "deploying" << deployed_app_name;

    argv[1] = (char*)deployed_app_name.toStdString().c_str();
    return deploy(argc, argv);
}


