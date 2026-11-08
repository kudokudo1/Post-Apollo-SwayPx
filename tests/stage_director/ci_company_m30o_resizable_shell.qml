import QtQuick
import Quickshell
import "widgets"

// M30o: real source-pinned CompanyNetworkW. All modifications stay in a
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
        interval: 1800
        running: true
        repeat: false
        onTriggered: proof.report("standard")
    }
    Timer {
        interval: 24000
        running: true
        repeat: false
        onTriggered: company.chassisLevel = "compact"
    }
    Timer {
        interval: 26000
        running: true
        repeat: false
        onTriggered: proof.report("compact")
    }
    Timer {
        interval: 46000
        running: true
        repeat: false
        onTriggered: company.chassisLevel = "fullscreen"
    }
    Timer {
        interval: 48000
        running: true
        repeat: false
        onTriggered: proof.report("fullscreen")
    }
    Timer {
        interval: 90000
        running: true
        repeat: false
        onTriggered: company.chassisLevel = "standard"
    }
    Timer {
        interval: 92000
        running: true
        repeat: false
        onTriggered: proof.report("standard")
    }
    Timer {
        interval: 122000
        running: true
        repeat: false
        onTriggered: {
            company.menuOpen = false;
            proof.report("closed");
        }
    }
}
