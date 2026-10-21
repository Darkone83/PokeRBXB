"""Simple ROM-to-XPD desktop packer (PySide6, Python 3.10+)."""
from __future__ import annotations

import os
import sys
import tempfile
from pathlib import Path

from PySide6.QtCore import Qt, QSettings, QThread, Signal, Slot
from PySide6.QtWidgets import (
    QApplication, QFileDialog, QFrame, QGridLayout, QHBoxLayout, QLabel,
    QLineEdit, QMainWindow, QMessageBox, QProgressBar, QPushButton,
    QVBoxLayout, QWidget,
)

from xpd import ROM_SIZE, build_pack, hash_rom, read_archive

APP_VERSION = "0.3.0"
STYLES = """
QWidget { background: #121018; color: #e8e2ef; font-family: 'Segoe UI', Arial; font-size: 10pt; }
QFrame#roms { background: #1e1928; border: 1px solid #383046; border-radius: 10px; }
QFrame#roms QLabel { background: transparent; }
QLabel#title { font-size: 21pt; font-weight: 700; color: #f3ecfa; }
QLabel#hint, QLabel#status { color: #b6a9c6; }
QLineEdit { background: #100e16; color: #f1eafb; border: 1px solid #4c3b5d; border-radius: 6px; padding: 8px; selection-background-color: #8b54ba; }
QLineEdit:focus { border-color: #b67bd8; }
QLineEdit:disabled { color: #80748d; }
QPushButton { background: #33273f; border: 1px solid #5b4970; border-radius: 7px; padding: 8px 14px; color: #f4eafc; font-weight: 600; }
QPushButton:hover { background: #443254; }
QPushButton:disabled { color: #80748d; background: #27222e; border-color: #35303e; }
QPushButton#primary { background: #7e46ad; border-color: #a66fc8; padding: 10px 22px; }
QPushButton#primary:hover { background: #9453c7; }
QPushButton#primary:disabled { background: #403248; border-color: #51405d; }
QProgressBar { background: #0f0c15; border: 1px solid #423651; border-radius: 6px; text-align: center; min-height: 20px; }
QProgressBar::chunk { background: #9857c4; border-radius: 5px; }
"""


def output_directory() -> Path:
    # A frozen build must use the EXE folder, not its temporary extraction folder.
    app_path = Path(sys.executable if getattr(sys, "frozen", False) else __file__)
    return app_path.resolve().parent / "data"


def load_rom(path: str, game: str) -> bytes:
    try:
        with Path(path).open("rb") as stream:
            data = stream.read(ROM_SIZE + 1)
    except OSError as error:
        raise ValueError(
            f"Cannot read the {game.title()} ROM.\n"
            "Choose an accessible Game Boy ROM file."
        ) from error
    try:
        hash_rom(data, game)
    except ValueError as error:
        raise ValueError(
            f"The selected file is not a supported Pokémon {game.title()} ROM.\n"
            "Choose an unmodified English (USA/Europe) ROM for this slot."
        ) from error
    return data


class BuildThread(QThread):
    progress = Signal(int, str)
    completed = Signal(object)
    failed = Signal(str)

    def __init__(self, inputs: dict[str, str], output: Path, parent=None):
        super().__init__(parent)
        self.inputs = dict(inputs)
        self.output = Path(output)

    def run(self) -> None:
        temporary: list[Path] = []
        saved: list[dict] = []
        try:
            selected = [game for game in ("red", "blue") if self.inputs.get(game)]
            if not selected:
                raise ValueError("Choose a Red or Blue ROM before building.")
            roms = {}
            # Validate every selected ROM before touching existing output.
            for index, game in enumerate(selected):
                self.progress.emit(index * 15 // len(selected), f"Checking Pokémon {game.title()}…")
                roms[game] = load_rom(self.inputs[game], game)
            self.progress.emit(15, "Preparing packs…")
            self.output.mkdir(parents=True, exist_ok=True)
            staged = []
            for index, game in enumerate(selected):
                base = 15 + index * 70 // len(selected)
                self.progress.emit(base, f"Packing Pokémon {game.title()}…")
                with tempfile.NamedTemporaryFile(
                    prefix=f".{game}.", suffix=".building", dir=self.output, delete=False
                ) as stream:
                    temporary.append(Path(stream.name))
                temp = temporary[-1]
                # Keep the full ROM and all bank views in the existing XPD1 format.
                info = build_pack(game, roms[game], [], temp)
                self.progress.emit(base + 45 // len(selected), f"Verifying Pokémon {game.title()}…")
                try:
                    checked = read_archive(temp, deep=True)
                    if checked["game"] != game:
                        raise ValueError("Game identity mismatch")
                except ValueError as error:
                    raise ValueError(
                        f"The Pokémon {game.title()} pack could not be verified.\n"
                        "Please try building it again."
                    ) from error
                staged.append((game, temp, info))
                self.progress.emit(15 + (index + 1) * 70 // len(selected), f"Pokémon {game.title()} verified")
            # Each destination is replaced only by a complete, verified pack.
            for index, (game, temp, info) in enumerate(staged):
                self.progress.emit(85 + index * 15 // len(selected), f"Saving Pokémon {game.title()}…")
                destination = self.output / f"{game}.xpd"
                os.replace(temp, destination)
                info["path"] = str(destination)
                saved.append(info)
            self.progress.emit(100, "Build complete")
            self.completed.emit(saved)
        except Exception as error:
            if isinstance(error, OSError):
                message = (
                    f"Cannot save packs to:\n{self.output}\n\n"
                    "Check that the folder is writable and has enough free space."
                )
            else:
                message = str(error)
            if saved:
                message += "\n\nAlready saved: " + ", ".join(Path(item["path"]).name for item in saved)
            self.failed.emit(message)
        finally:
            for path in temporary:
                try:
                    path.unlink(missing_ok=True)
                except OSError:
                    pass


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("XPD Packer")
        self.resize(660, 340)
        self.setMinimumWidth(560)
        self.settings = QSettings("Darkone Customs", "XPD Packer")
        self.output = output_directory()
        self.worker: BuildThread | None = None
        self.controls: list[QWidget] = []
        self.build_ui()
        self.restore_settings()

    def build_ui(self) -> None:
        root = QWidget()
        self.setCentralWidget(root)
        layout = QVBoxLayout(root)
        layout.setContentsMargins(22, 18, 22, 18)
        layout.setSpacing(12)

        title = QLabel("XPD Packer")
        title.setObjectName("title")
        layout.addWidget(title)
        hint = QLabel("Choose Pokémon Red, Blue, or both.")
        hint.setObjectName("hint")
        layout.addWidget(hint)

        panel = QFrame()
        panel.setObjectName("roms")
        grid = QGridLayout(panel)
        grid.setContentsMargins(14, 14, 14, 14)
        grid.setHorizontalSpacing(10)
        grid.setVerticalSpacing(12)
        grid.setColumnStretch(1, 1)
        self.red_edit = self.path_row(grid, 0, "Red ROM", "Choose Pokémon Red…", self.pick_red)
        self.blue_edit = self.path_row(grid, 1, "Blue ROM", "Choose Pokémon Blue…", self.pick_blue)
        layout.addWidget(panel)

        actions = QHBoxLayout()
        actions.addStretch()
        self.build_btn = QPushButton("Build Packs")
        self.build_btn.setObjectName("primary")
        self.build_btn.clicked.connect(self.start_build)
        actions.addWidget(self.build_btn)
        layout.addLayout(actions)

        self.progress = QProgressBar()
        self.progress.setRange(0, 100)
        self.progress.setValue(0)
        layout.addWidget(self.progress)
        self.phase = QLabel()
        self.phase.setObjectName("status")
        self.phase.setTextFormat(Qt.TextFormat.PlainText)
        self.phase.setWordWrap(True)
        self.phase.setTextInteractionFlags(Qt.TextInteractionFlag.TextSelectableByMouse)
        layout.addWidget(self.phase)
        layout.addStretch()

    def path_row(self, grid, row, label, hint, callback) -> QLineEdit:
        field = QLineEdit()
        field.setPlaceholderText(hint)
        field.setClearButtonEnabled(True)
        field.setAccessibleName(label)
        field.textChanged.connect(self.inputs_changed)
        button = QPushButton("Browse…")
        button.setAccessibleName(f"Browse for {label}")
        button.clicked.connect(callback)
        caption = QLabel(label)
        caption.setBuddy(field)
        grid.addWidget(caption, row, 0)
        grid.addWidget(field, row, 1)
        grid.addWidget(button, row, 2)
        self.controls.extend((field, button))
        return field

    def restore_settings(self) -> None:
        for game, field in (("red", self.red_edit), ("blue", self.blue_edit)):
            field.setText(str(self.settings.value(game, "") or ""))
        self.inputs_changed()

    def save_settings(self) -> None:
        self.settings.setValue("red", self.red_edit.text().strip())
        self.settings.setValue("blue", self.blue_edit.text().strip())

    def inputs(self) -> dict[str, str]:
        return {"red": self.red_edit.text().strip(), "blue": self.blue_edit.text().strip()}

    @Slot()
    def inputs_changed(self) -> None:
        if self.worker is not None:
            return
        selected = any(self.inputs().values())
        self.build_btn.setEnabled(selected)
        self.progress.setValue(0)
        self.phase.setText(
            "Ready to build. Packs are saved to the data folder beside this app."
            if selected else "Select a ROM to begin."
        )

    def choose_file(self, target: QLineEdit, game: str) -> None:
        if self.worker is not None:
            return
        path, _ = QFileDialog.getOpenFileName(
            self, f"Select Pokémon {game} ROM",
            target.text().strip() or str(Path.home()),
            "Game Boy ROMs (*.gb *.gbc);;All files (*)",
        )
        if path:
            target.setText(path)

    def pick_red(self) -> None:
        self.choose_file(self.red_edit, "Red")

    def pick_blue(self) -> None:
        self.choose_file(self.blue_edit, "Blue")

    def set_busy(self, busy: bool) -> None:
        for control in self.controls:
            control.setEnabled(not busy)
        self.build_btn.setEnabled(not busy and any(self.inputs().values()))
        self.build_btn.setText("Building…" if busy else "Build Packs")

    @Slot()
    def start_build(self) -> None:
        if self.worker is not None:
            return
        inputs = self.inputs()
        if not any(inputs.values()):
            return
        self.save_settings()
        self.progress.setValue(0)
        self.phase.setText("Checking ROMs…")
        self.set_busy(True)
        self.worker = BuildThread(inputs, self.output, self)
        self.worker.progress.connect(self.on_progress)
        self.worker.completed.connect(self.on_build_complete)
        self.worker.failed.connect(self.on_failed)
        self.worker.finished.connect(self.on_worker_finished)
        self.worker.start()

    @Slot(int, str)
    def on_progress(self, value: int, status: str) -> None:
        self.progress.setValue(max(0, min(100, value)))
        self.phase.setText(status)

    @Slot(object)
    def on_build_complete(self, reports: list[dict]) -> None:
        self.progress.setValue(100)
        names = " and ".join(Path(item["path"]).name for item in reports)
        self.phase.setText(f"Created {names}.\nSaved to: {self.output}")

    @Slot(str)
    def on_failed(self, problem: str) -> None:
        self.progress.setValue(0)
        self.phase.setText("Build failed. Review the error and try again.")
        dialog = QMessageBox(self)
        dialog.setIcon(QMessageBox.Icon.Critical)
        dialog.setWindowTitle("Unable to build packs")
        dialog.setTextFormat(Qt.TextFormat.PlainText)
        dialog.setText(problem)
        dialog.setStandardButtons(QMessageBox.StandardButton.Ok)
        dialog.exec()

    @Slot()
    def on_worker_finished(self) -> None:
        worker = self.worker
        self.worker = None
        if worker is not None:
            worker.deleteLater()
        self.set_busy(False)

    def closeEvent(self, event) -> None:
        if self.worker is not None and self.worker.isRunning():
            self.phase.setText("Please wait for the build to finish.")
            event.ignore()
            return
        self.save_settings()
        super().closeEvent(event)


def main() -> int:
    app = QApplication(sys.argv)
    app.setApplicationName("XPD Packer")
    app.setApplicationVersion(APP_VERSION)
    app.setOrganizationName("Darkone Customs")
    app.setStyle("Fusion")
    app.setStyleSheet(STYLES)
    window = MainWindow()
    window.show()
    return app.exec()


if __name__ == "__main__":
    raise SystemExit(main())
