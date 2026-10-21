#!/usr/bin/env python3
"""XPD desktop launcher: install missing PySide6, then open the packer GUI.

This is the only entry point requiring pip. xpd.py/xpd_reader remain dependency-free.
"""
from __future__ import annotations

import importlib.util
import os
import subprocess
import sys
import traceback


def show_failure(message: str) -> None:
    print(message, file=sys.stderr)
    # A .pyw launch has no visible console on Windows. Prefer a system dialog.
    if sys.platform == "win32":
        try:
            import ctypes
            ctypes.windll.user32.MessageBoxW(None, message, "XPD Packer — Startup Error", 0x10)
        except Exception:
            pass


def bootstrap() -> bool:
    if sys.version_info < (3, 10):
        show_failure("Python 3.10 or newer is required to run the XPD Packer GUI.")
        return False
    try:
        import PySide6.QtCore  # noqa: F401
        import PySide6.QtWidgets  # noqa: F401
        return True
    except ImportError:
        pass

    print("PySide6 is missing. Installing the required GUI module using pip...", flush=True)
    if importlib.util.find_spec("pip") is None:
        print("pip is missing; attempting to enable it with ensurepip...", flush=True)
        try:
            setup = subprocess.run([sys.executable, "-m", "ensurepip", "--upgrade"],
                                   capture_output=True, text=True, timeout=120)
            if setup.returncode != 0:
                show_failure("pip is not available and ensurepip failed:\n" + setup.stderr[-2500:])
                return False
        except (OSError, subprocess.TimeoutExpired) as error:
            show_failure("pip setup failed: " + str(error))
            return False
    cmd = [sys.executable, "-m", "pip", "install", "--disable-pip-version-check", "PySide6>=6.5,<7"]
    try:
        # Hide the extra console when users run this file with pythonw.exe.
        kwargs = {}
        if sys.platform == "win32" and hasattr(subprocess, "CREATE_NO_WINDOW"):
            kwargs["creationflags"] = subprocess.CREATE_NO_WINDOW
        proc = subprocess.run(cmd, capture_output=True, text=True, timeout=300, **kwargs)
    except (OSError, subprocess.TimeoutExpired) as error:
        show_failure("Could not install PySide6 automatically:\n" + str(error) +
                     "\n\nRun this manually:\n" + ' '.join(cmd))
        return False
    if proc.returncode != 0:
        output = (proc.stderr or proc.stdout or "Unknown pip error")[-3500:]
        show_failure("PySide6 installation failed:\n" + output +
                     "\n\nRun this manually:\n" + ' '.join(cmd))
        return False
    try:
        import PySide6.QtWidgets  # noqa: F401
    except ImportError as error:
        show_failure("PySide6 was installed but cannot be imported: " + str(error))
        return False
    print("PySide6 installation complete.", flush=True)
    return True


def main() -> int:
    if not bootstrap():
        return 1
    try:
        from xpd_gui import main as run_gui
        return run_gui()
    except Exception:
        show_failure("Could not start XPD Packer:\n" + traceback.format_exc())
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
