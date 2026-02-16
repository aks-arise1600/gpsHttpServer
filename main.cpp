#include <QCoreApplication>
#include <QHttpServer>
#include <QHttpServerRequest>
#include <QHttpServerResponse>
#include <QTcpServer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <QRandomGenerator>
#include <QString>

struct LocationData
{
    QString utc;
    double lat = 0;
    double lon = 0;
    double alt = 0;
};

static QHash<QString, LocationData> g_locations;


QString generateRandomId(int length = 12)
{
    const QString chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789";

    QString result;
    result.reserve(length);

    for (int i = 0; i < length; ++i) {
        int index = QRandomGenerator::global()->bounded(chars.size());
        result.append(chars.at(index));
    }

    return result;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QHttpServer httpServer;
    QTcpServer tcpServer;

    // REST endpoint
    httpServer.route("/api/send_location", QHttpServerRequest::Method::Post,
                     [](const QHttpServerRequest &req) {

                         QJsonDocument doc = QJsonDocument::fromJson(req.body());
                         if (!doc.isObject()) {
                             return QHttpServerResponse(
                                 QJsonObject{
                                     {"status", "error"},
                                     {"message", "Invalid JSON"}
                                 },
                                 QHttpServerResponse::StatusCode::BadRequest
                                 );
                         }

                         QJsonObject obj = doc.object();
                         if (!obj.contains("lat") || !obj.contains("lon") || !obj.contains("dev_id")) {
                             return QHttpServerResponse(
                                 QJsonObject{
                                     {"status", "error"},
                                     {"message", "Missing fields"}
                                 },
                                 QHttpServerResponse::StatusCode::BadRequest
                                 );
                         }

                         LocationData loc;
                         loc.utc = obj["utc"].toString();
                         loc.lat = obj["lat"].toDouble();
                         loc.lon = obj["lon"].toDouble();
                         loc.alt = obj["alt"].toDouble();

                         g_locations[obj["dev_id"].toString()] = loc;

                         qDebug() << "Location:"
                                  << obj["lat"].toDouble()
                                  << obj["lon"].toDouble()
                                  << obj["utc"].toString()
                                  << obj["dev_id"].toString();

                         return QHttpServerResponse(
                             QJsonObject{{"status", "ok"}}
                             );
                     });

    httpServer.route("/api/get_location", QHttpServerRequest::Method::Get,
                 [](const QHttpServerRequest &req) {

                     const auto query = req.query();
                     const QString devId = query.queryItemValue("dev_id");

                     if (devId.isEmpty()) {
                         return QHttpServerResponse(
                             QJsonObject{
                                 {"status","error"},
                                 {"message","dev_id missing"}
                             },
                             QHttpServerResponse::StatusCode::BadRequest
                             );
                     }

                     if (!g_locations.contains(devId)) {
                         return QHttpServerResponse(
                             QJsonObject{
                                 {"status","error"},
                                 {"message","device not found"}
                             },
                             QHttpServerResponse::StatusCode::NotFound
                             );
                     }

                     const LocationData &loc = g_locations[devId];

                     return QHttpServerResponse(
                         QJsonObject{
                             {"status","ok"},
                             {"dev_id", devId},
                             {"utc", loc.utc},
                             {"lat", loc.lat},
                             {"lon", loc.lon},
                             {"alt", loc.alt}
                         }
                         );
                 });


    const quint16 port = 8080;

    if (!tcpServer.listen(QHostAddress::Any, port)) {
        qCritical() << "TCP listen failed:" << tcpServer.errorString();
        return -1;
    }

    // 🔗 Bind HTTP server to TCP server
    httpServer.bind(&tcpServer);

    qDebug() << "Qt HTTP server listening on port" << port;
    return app.exec();
}
