#!/usr/bin/env python3
"""M30e adversarial checks for the pinned cross-repository source contract.

Only disposable copied source text is mutated. No Qt process, window,
network style, binary, native bridge or operator desktop is used.
"""
from __future__ import annotations

import importlib.util
from pathlib import Path
import shutil
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location(
    "m30e_source_contract",
    ROOT / "tests/stage_director/ci_company_maplibre_source_contract.py")
assert SPEC is not None and SPEC.loader is not None
gate = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(gate)


class MapLibreSourceNegativeTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix="sd-company-m30e-")
        self.addCleanup(self.tmp.cleanup)
        self.company = Path(self.tmp.name) / "company"
        self.sway = Path(self.tmp.name) / "swaypx"
        external = ROOT / "company-source"
        self.assertTrue((external / "experiments/company-vector-qt/main.cpp").is_file(),
                        "test needs exact Company commit checked out by CI")
        for file in gate.REQUIRED_COMPANY_FILES:
            dest = self.company / file
            dest.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(external / file, dest)
        for file in gate.REQUIRED_SWAY_FILES:
            dest = self.sway / file
            dest.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT / file, dest)

    def replace(self, folder: Path, file: str, original: str, new: str):
        path = folder / file
        text = path.read_text()
        self.assertIn(original, text)
        path.write_text(text.replace(original, new, 1))

    def requires_refusal(self, code: str):
        reasons = gate.verify(self.company, self.sway)
        self.assertIn(code, reasons, reasons)

    def test_original_company_source_is_reviewable_only(self):
        self.assertEqual(gate.verify(self.company, self.sway), [])

    def test_wrong_maplibre_target_fails(self):
        self.replace(self.company, "experiments/company-vector-qt/CMakeLists.txt",
                     "QMapLibre::Location", "Qt6::Location")
        self.assertTrue(any(x.startswith("MAPLIBRE_CMAKE_CONTRACT:")
                            for x in gate.verify(self.company, self.sway)))

    def test_removed_map_plugin_fails(self):
        self.replace(self.company, "experiments/company-vector-qt/main.qml",
                     'name: "maplibre"', 'name: "foreign-map"')
        self.assertTrue(any(x.startswith("MAPLIBRE_QML_CONTRACT:")
                            for x in gate.verify(self.company, self.sway)))

    def test_opengl_rhi_after_app_construction_fails(self):
        file = "experiments/company-vector-qt/main.cpp"
        path = self.company / file
        src = path.read_text()
        a = "QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGLRhi);"
        b = "QGuiApplication app(argc, argv);"
        self.assertIn(a, src)
        self.assertIn(b, src)
        path.write_text(src.replace(a, "/* RHI moved below application */", 1)
                        .replace(b, b + "\n    " + a, 1))
        self.assertIn("GL_API_NOT_PROCESS_LOCAL_BEFORE_QT",
                      gate.verify(self.company, self.sway))

    def test_online_default_injection_fails(self):
        self.replace(self.company, "experiments/company-vector-qt/main.cpp",
                     ": QUrl::fromLocalFile(localPath);",
                     ': QUrl(QStringLiteral("https://example.invalid/street"));')
        self.requires_refusal("OFFLINE_DEFAULT_REPLACED")

    def test_network_url_in_offline_geojson_fails(self):
        file = "experiments/company-vector-qt/style-fixture.json"
        self.replace(self.company, file, '"probe-features": {',
                     '"probe-features": { "url": "https://example.invalid/tiles",')
        self.requires_refusal("OFFLINE_STYLE_UNEXPECTED_NETWORK")

    def test_synthetic_tag_removed_fails(self):
        self.replace(self.company, "experiments/company-vector-qt/style-fixture.json",
                     '"not-real-streets": true', '"not-real-streets": false')
        self.requires_refusal("OFFLINE_SYNTHETIC_METADATA_LOST")

    def test_launcher_detached_process_fails(self):
        self.replace(self.company,
                     "experiments/company-vector-qt/CompanyVectorHostBridge.py",
                     "os.execve(str(binary)", "subprocess.Popen(str(binary)")
        self.requires_refusal("ATTACHED_SAME_PID_EXEC_CHAIN_BROKEN")

    def test_bridge_path_mismatch_fails(self):
        self.replace(self.company,
                     "services/company/CompanyVectorSidecarLaunch.py",
                     '".local", "libexec", "post-apollo"',
                     '".local", "bin", "post-apollo"')
        self.requires_refusal("M13_M15_INSTALLED_BRIDGE_PATH_DRIFT")

    def test_final_elf_path_drift_fails(self):
        self.replace(self.sway, "sway/stage_director_company_executable.c",
                     'open_owned_dir(home, "postapollo-vector-lab"',
                     'open_owned_dir(home, "alternate-lab"')
        self.requires_refusal("M29_FINAL_ELF_INODE_CONTRACT_DRIFT")

    def test_unapproved_elf_hash_cannot_sneak_in(self):
        self.replace(self.sway, "sway/stage_director_company_digest.c",
                     'company_m30_manifest_elf_sha256[] = "";',
                     'company_m30_manifest_elf_sha256[] = "untrusted";')
        self.requires_refusal("M30A_UNAPPROVED_DIGEST_PIN")

    def test_native_bridge_default_on_fails(self):
        self.replace(self.company, "widgets/CompanyNetworkW.qml",
                     "property bool vectorBridgeOptIn: false",
                     "property bool vectorBridgeOptIn: true")
        self.requires_refusal("COMPANY_NATIVE_BRIDGE_NOT_DEFAULT_OFF")

    def test_missing_or_symlinked_source_fails_closed(self):
        path = self.company / "experiments/company-vector-qt/main.qml"
        self.assertTrue(path.is_file())
        path.unlink()
        self.assertTrue(gate.verify(self.company, self.sway)[0].startswith(
            "MISSING_OR_SYMLINKED:"))
        path.symlink_to(self.company / "experiments/company-vector-qt/main.cpp")
        self.assertTrue(gate.verify(self.company, self.sway)[0].startswith(
            "MISSING_OR_SYMLINKED:"))


if __name__ == "__main__":
    unittest.main()
