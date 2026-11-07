import QtQuick
import Quickshell
import "widgets"

// M30k: real source-pinned CompanyNetworkW. All modifications stay in a
// disposable GitHub runner copy, with bridge/actor admission still OFF.
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
    function report(label) {
        const frame = company.activeVectorFrameInHostRect;
        const canvas = company.ciM30iActualCanvasHostRect;
        const candidate = company.activeVectorViewportRect;
        console.log("SD_COMPANY_M30I_QML_GEOMETRY" +
            " phase=" + label + " level=" + company.chassisLevel +
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
        interval: 8200
        running: true
        repeat: false
        onTriggered: company.chassisLevel = "compact"
    }
    Timer {
        interval: 10000
        running: true
        repeat: false
        onTriggered: proof.report("compact")
    }
    Timer {
        interval: 15200
        running: true
        repeat: false
        onTriggered: company.chassisLevel = "fullscreen"
    }
    Timer {
        interval: 17200
        running: true
        repeat: false
        onTriggered: proof.report("fullscreen")
    }
    Timer {
        interval: 23200
        running: true
        repeat: false
        onTriggered: company.chassisLevel = "standard"
    }
    Timer {
        interval: 25200
        running: true
        repeat: false
        onTriggered: proof.report("standard")
    }
    Timer {
        interval: 33400
        running: true
        repeat: false
        onTriggered: {
            company.menuOpen = false;
            proof.report("closed");
        }
    }
}
