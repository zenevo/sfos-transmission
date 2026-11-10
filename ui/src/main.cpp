#include <sailfishapp.h>
#include <QGuiApplication>
#include <QQuickView>
#include <QQmlContext>
#include <QScopedPointer>
#include "backend.h"

int main(int argc, char *argv[])
{
    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    QScopedPointer<QQuickView> view(SailfishApp::createView());

    Backend backend;
    view->rootContext()->setContextProperty("backend", &backend);

    view->setSource(SailfishApp::pathTo("qml/main.qml"));
    view->show();
    return app->exec();
}
