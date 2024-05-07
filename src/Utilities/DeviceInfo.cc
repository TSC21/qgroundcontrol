#include <QDebug>
#include <QtNetwork/QNetworkInformation>
#ifdef QGC_ENABLE_BLUETOOTH
    #include <QtBluetooth/QBluetoothLocalDevice>
#endif

namespace QGCDeviceInfo {
    static bool _isInternetReachable = false;  // Holds the current internet reachability state

    // Attempt to load the default network backend
    void loadDefaultNetworkBackend() {
        if (!QNetworkInformation::loadDefaultBackend()) {
            qWarning() << "Failed to load the default network backend.";
        }
    }

    void updateInternetReachability() {
        auto networkInfo = QNetworkInformation::instance();
        if (!networkInfo) {
            qWarning() << "QNetworkInformation instance is not available after attempting to load default backend.";
            _isInternetReachable = false;
            return;
        }

        // Update the internet reachability based on the network reachability
        _isInternetReachable = (networkInfo->reachability() == QNetworkInformation::Reachability::Online);
        if (networkInfo->isMetered()) {
            qInfo() << "Note: Connection is metered.";
        }
    }

    bool isInternetAvailable() {
        return _isInternetReachable;
    }

    bool isBluetoothAvailable() {
    #ifdef QGC_ENABLE_BLUETOOTH
        return !QBluetoothLocalDevice::allDevices().isEmpty();
    #else
        return false;
    #endif
    }

    void setupNetworkListeners() {
        loadDefaultNetworkBackend();  // Ensure the default backend is loaded before setting up listeners

        auto networkInfo = QNetworkInformation::instance();
        if (!networkInfo) {
            qWarning() << "Failed to obtain QNetworkInformation instance for setting up listeners.";
            return;
        }

        QObject::connect(networkInfo, &QNetworkInformation::reachabilityChanged,
                         [](QNetworkInformation::Reachability newReachability) {
                             qInfo() << "Network reachability changed to: " << static_cast<int>(newReachability);
                             updateInternetReachability();  // Update the reachability status whenever it changes
                         });

        QObject::connect(networkInfo, &QNetworkInformation::isMeteredChanged,
                         [](bool isMetered) {
                             qInfo() << "Metered connection state changed to: " << isMetered;
                         });

        QObject::connect(networkInfo, &QNetworkInformation::transportMediumChanged,
                         [](QNetworkInformation::TransportMedium current) {
                             qInfo() << "Transport medium changed to: " << static_cast<int>(current);
                         });
    }
}
