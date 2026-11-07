import QtQuick
import Quickshell
import "widgets"

// M30j: original pinned CompanyNetworkW in a disposable CI-only COPY.
// The original Company launch/bridge policy remains untouched.
ShellRoot {
    id: proof
    CompanyNetworkW {
        id: company
        menuOpen: true
        coverageLoaded: true
        chassisLevel: "standard"
        dockCenterX: 960
        chassisTopY: 160
        vectorBridgeOptIn: false
    }
    function rect(r) {
        return Math.round(r.x) + "," + Math.round(r.y) + "," +
            Math.round(r.width) + "," + Math.round(r.height);
    }
    function report(phase) {
        const frame = company.activeVectorFrameInHostRect;
        const canvas = company.ciM30iActualCanvasHostRect;
        const candidate = company.activeVectorViewportRect;
        console.log("SD_COMPANY_M30I_QML_GEOMETRY" +
            " phase=" + phase + " level=" + company.chassisLevel +
            " open=" + (company.menuOpen ? "1" : "0") +
            " panel=" + Math.round(company.width) + "," +
                          Math.round(company.height) +
            " frame=" + proof.rect(frame) +
            " canvas=" + proof.rect(canvas) +
            " candidate=" + proof.rect(candidate) +
            " bridge_enabled=" + (company.vectorBridgeOptIn ? "1" : "0"));
    }
    Timer {
        interval: 1500
        running: true
        repeat: false
        onTriggered: proof.report("standard")
    }
    Timer {
        // More than enough time for Qt actor and actual Wayland pointer
        // probes. This is an intentionally bounded CI-only session.
        interval: 30000
        running: true
        repeat: false
        onTriggered: {
            company.menuOpen = false;
            proof.report("closed");
        }
    }
}
