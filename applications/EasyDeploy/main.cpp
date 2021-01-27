#include <QDir>
#include <QFileInfo>
#include <QDebug>




int deploy_main(int argc, char **argv);

int main(int argc, char **argv)
{
    argc = 3;
    argv[1] = "/Users/lnan/Projects/Easy3D/cmake-build-release/bin/Mapple.app";
    argv[2] = "-verbose=3";

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
    return deploy_main(argc, argv);
}



