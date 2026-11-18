import importlib.util
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location("installer", ROOT / "tools/install_into_rockbox.py")
installer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(installer)

class TestInstall(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root / "apps/plugins").mkdir(parents=True)
        (self.root / "firmware/export/config").mkdir(parents=True)
        (self.root / "apps/plugins/SOURCES").write_text("/* plugins common to all models */\npong.c\n")
        (self.root / "apps/main.c").write_text(
            '// #define AUTOROCK /* define this to check for "autostart.rock" on boot */\n')
        (self.root / "firmware/export/config/shanlingq2.h").write_text(
            '#define MODEL_NAME "Shanling Q2"\n')

    def test_installer_idempotent(self):
        installer.install(self.root)
        installer.install(self.root)
        content = (self.root / "apps/plugins/SOURCES").read_text()
        self.assertEqual(content.count(installer.MARKER), 1)
        self.assertIn("q2_games.c", content)
        self.assertTrue((self.root / "apps/plugins/q2_games.c").is_file())
        self.assertTrue((self.root / "apps/plugins/q2_games_catalog.inc").is_file())
        self.assertNotIn(installer.AUTOROCK, (self.root / "apps/main.c").read_text())

    def test_explicit_games_first(self):
        installer.install(self.root, games_first=True)
        installer.install(self.root, games_first=True)
        contents = (self.root / "apps/main.c").read_text()
        self.assertEqual(contents.count(installer.AUTOROCK), 1)

    def test_missing_checkout_rejected(self):
        with self.assertRaises(ValueError):
            installer.install(self.root / "absent")

if __name__ == "__main__":
    unittest.main()
