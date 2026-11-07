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
        // Initial native map absent by design; open actual Coverage UI
        // declaratively so there is no onCompleted ordering race.
        menuOpen: true
        coverageLoaded: true
        chassisLevel: "standard"
        dockCenterX: 960
        chassisTopY: 160
        vectorBridgeOptIn: false
    }

    function formatRect(rect) {
        return Math.round(rect.x) + "," + Math.round(rect.y) + "," +
               Math.round(rect.width) + "," + Math.round(rect.height);
    }

    function capture(phase) {
        console.log("SD_COMPANY_M30I_DEBUG phase=" + phase +
            " frame_visible=" + (company.ciM30iFrameItemVisible ? "1" : "0") +
            " coverage_present=" + (company.ciM30iCoverageItemPresent ? "1" : "0") +
            " frame_raw=" + proof.formatRect(company.ciM30iFrameRawRect) +
            " frame_mapped=" + proof.formatRect(company.ciM30iFrameMappedRect) +
            " coverage_root=" + proof.formatRect(company.ciM30iCoverageRootRect) +
            " column=" + proof.formatRect(company.ciM30iColumnRect) +
            " map_row=" + proof.formatRect(company.ciM30iMapRowRect) +
            " canvas_raw=" + proof.formatRect(company.ciM30iCanvasRawRect) +
            " canvas_in_coverage=" +
              proof.formatRect(company.ciM30iCanvasCoverageRect) +
            " surface_raw=" +
              proof.formatRect(company.ciM30iMapSurfaceRawRect) +
            " surface_in_coverage=" +
              proof.formatRect(company.ciM30iMapSurfaceInCoverage) +
            " canvas_in_surface=" +
              proof.formatRect(company.ciM30iCanvasInSurface) +
            " map_row_in_coverage=" +
              proof.formatRect(company.ciM30iMapRowInCoverage));
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
