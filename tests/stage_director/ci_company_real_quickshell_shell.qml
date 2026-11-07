import QtQuick
import Quickshell
import "widgets"

// M30i disposable *real* CompanyNetworkW imported from pinned Company source.
// This QML file is generated into a separate temporary COPY of Company,
// not the user's installed desktop or the reviewed Git checkout.
ShellRoot {
    id: proof

    CompanyNetworkW {
        id: company
        menuOpen: false
        chassisLevel: "standard"
        dockCenterX: 960
        chassisTopY: 160
        vectorBridgeOptIn: false
        Component.onCompleted: {
            company.coverageLoaded = true;
            company.menuOpen = true;
        }
    }

    function formatRect(rect) {
        return Math.round(rect.x) + "," + Math.round(rect.y) + "," +
               Math.round(rect.width) + "," + Math.round(rect.height);
    }

    function capture(phase) {
        const host = company.activeVectorFrameInHostRect;
        const candidate = company.activeVectorViewportRect;
        const canvas = company.ciM30iActualCanvasHostRect;
        console.log("SD_COMPANY_M30I_QML_GEOMETRY" +
            " phase=" + phase +
            " level=" + company.chassisLevel +
            " open=" + (company.menuOpen ? "1" : "0") +
            " panel=" + Math.round(company.width) + "," +
                          Math.round(company.height) +
            " frame=" + proof.formatRect(host) +
            " canvas=" + proof.formatRect(canvas) +
            " candidate=" + proof.formatRect(candidate) +
            " bridge_enabled=" + (company.vectorBridgeOptIn ? "1" : "0"));
    }

    Timer {
        id: sequence
        interval: 1500
        repeat: true
        running: true
        property int phase: 0
        onTriggered: {
            if (phase === 0) {
                proof.capture("standard");
            } else if (phase === 1) {
                company.chassisLevel = "compact";
            } else if (phase === 2) {
                proof.capture("compact");
            } else if (phase === 3) {
                company.chassisLevel = "fullscreen";
            } else if (phase === 4) {
                proof.capture("fullscreen");
            } else if (phase === 5) {
                company.menuOpen = false;
            } else if (phase === 6) {
                proof.capture("closed");
            } else {
                sequence.stop();
            }
            phase++;
        }
    }
}
