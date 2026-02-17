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

struct DeviceInfoData
{
    QString board;
    QString product;
    QString manufacturer;
    QString model;
    QString android_id;
    QString dev_id;
};

static QHash<QString, LocationData> g_locations;
static QHash<QString, DeviceInfoData> g_device_info;


QString generateRandomId(int length = 12)
{
    const QString chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "0123456789";

    QString result;
    result.reserve(length);

    for (int i = 0; i < length; ++i) {
        int index = QRandomGenerator::global()->bounded(chars.size());
        result.append(chars.at(index));
    }

    return result;
}

QHttpServerResponse m_deviceinfo(const QHttpServerRequest &req) {

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
    if (!obj.contains("android_id") || !obj.contains("dev_id")) {
        return QHttpServerResponse(
            QJsonObject{
                {"status", "error"},
                {"message", "Missing fields"}
            },
            QHttpServerResponse::StatusCode::BadRequest
            );
    }

    DeviceInfoData dev;
    dev.board = obj["board"].toString();
    dev.product = obj["product"].toString();
    dev.manufacturer = obj["manufacturer"].toString();
    dev.model = obj["model"].toString();
    dev.android_id = obj["android_id"].toString();
    dev.dev_id = obj["dev_id"].toString();


    g_device_info[obj["dev_id"].toString()] = dev;

    qDebug() << "Device:"
             << dev.board
             << dev.product
             << dev.manufacturer
             << dev.model
             << dev.android_id;

    return QHttpServerResponse(
        QJsonObject{{"status", "ok"}}
        );
}

QHttpServerResponse m_locationInfo(const QHttpServerRequest &req) {

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
             << loc.lat
             << loc.lon
             << loc.alt
             << loc.utc
             << obj["dev_id"].toString();

    return QHttpServerResponse(
        QJsonObject{{"status", "ok"}}
        );
}

QHttpServerResponse m_getLoaction(const QHttpServerRequest &req) {

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
    if (!obj.contains("dev_id") ) {
        return QHttpServerResponse(
            QJsonObject{
                {"status", "error"},
                {"message", "Missing fields"}
            },
            QHttpServerResponse::StatusCode::BadRequest
            );
    }

    const QString devId = obj["dev_id"].toString();

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
    qDebug()<<"Send Data of "<<devId;
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
}

QHttpServerResponse m_getNewId(const QHttpServerRequest &) {

    QString devId;

    // Ensure uniqueness (very unlikely loop, but safe)
    do {
        devId = "ARISE1600_"+generateRandomId(12);
    } while (g_locations.contains(devId));

    return QHttpServerResponse(
        QJsonObject{
            {"status", "ok"},
            {"NewDev_id", devId}
        }
        );
}
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QHttpServer httpServer;
    QTcpServer tcpServer;

    httpServer.route("/api/send_devinfo",
                     QHttpServerRequest::Method::Post,
                     [](const QHttpServerRequest &req) {
                         return m_deviceinfo(req);
                     });

    // REST endpoint
    httpServer.route("/api/send_location",
                     QHttpServerRequest::Method::Post,
                     [](const QHttpServerRequest &req) {
                         return m_locationInfo(req);
                     });

    httpServer.route("/api/get_location",
                     QHttpServerRequest::Method::Get,
                     [](const QHttpServerRequest &req) {
                         return m_getLoaction(req);
                     });

    httpServer.route("/api/get_newid",
                     QHttpServerRequest::Method::Get,
                     [](const QHttpServerRequest &req) {
                         return m_getNewId(req);
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
