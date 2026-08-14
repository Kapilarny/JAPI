import os
import subprocess
import json
import time
import hashlib
import shutil

# TODO: make this less shit

def build_japi():
    MINGW_BIN = r"C:\Users\kapil\AppData\Local\Programs\CLion\bin\mingw\bin"
    env = os.environ.copy()

    env["PATH"] = MINGW_BIN + os.pathsep + env["PATH"]
    env["CC"] = os.path.join(MINGW_BIN, "gcc.exe")
    env["CXX"] = os.path.join(MINGW_BIN, "g++.exe")
    env["NINJA_PATH"] = (
        r"C:\Users\kapil\AppData\Local\Programs\CLion\bin\ninja\win\x64\ninja.exe"
    )

    os.chdir(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

    subprocess.run(
        ["cmake", "--preset", "release_package"],
        env=env,
        check=True,
    )

    subprocess.run(
        ["cmake", "--build", "--preset", "release_package"],
        env=env,
        check=True,
    )

    # reset working directory to the script's directory
    os.chdir(os.path.dirname(os.path.abspath(__file__)))


def import_release_manifest():
    with open("release.json", "r") as f:
        return json.load(f)

def create_manifests():
    release_man = import_release_manifest()
    manifest = {
        "manifest_version": 1,
        "content": {
            "version": release_man["japi_version"],
            "timestamp": time.time_ns(),
            "files": [
                {"name": "JAPI.dll"},
                {"name": "JAPIPreload.dll"},
            ],
        }
    }

    with open("out/raw/manifest.json", "w") as f:
        json.dump(manifest, f, indent=2)

    """
    {
  "manifest_version": 1,
  "content": {
    "version": "4.0.0",
    "dlls": [
      {
        "name": "libwinpthread-1.dll",
        "hash": "somehashhere"
      },
      {
        "name": "libgcc_s_seh-1.dll",
        "hash": "somehashhere"
      },
      {
        "name": "libstdc++-6.dll",
        "hash": "somehashhere"
      },
      {
        "name": "imgui_shared.dll",
        "hash": "somehashhere"
      }
    ],
    "load_order": [
      "libwinpthread-1.dll",
      "libgcc_s_seh-1.dll",
      "libstdc++-6.dll",
      "imgui_shared.dll"
    ]
  }
}
"""
    # find all dlls in ../external/dlls and add them to the manifest
    dlls = []
    for file in os.listdir("../external/dlls"):
        if file.endswith(".dll"):
            name = file

            # compute SHA256 hash of the file
            with open(os.path.join("../external/dlls", file), "rb") as f:
                hsh = hashlib.sha256(f.read()).hexdigest()

            dlls.append({"name": name, "hash": hsh})

    # find the ../external/dlls/lib_load_order.txt file and read the load order
    load_order = []
    with open("../external/dlls/lib_load_order.txt", "r") as f:
        for line in f:
            if line.strip() and not line.startswith("#"):
                load_order.append(line.strip())

    dll_manifest = {
        "manifest_version": 1,
        "content": {
            "version": release_man["japi_version"],
            "dlls": dlls,
            "load_order": load_order,
        }
    }

    with open("out/raw/dependencies.json", "w") as f:
        json.dump(dll_manifest, f, indent=2)

def create_directories():
    os.makedirs("out/raw", exist_ok=True)
    os.makedirs("out/raw/japi/dlls", exist_ok=True)

def copy_binaries():
    for file in ["JAPI.dll", "JAPIPreload.dll"]:
        src = os.path.join("build", "out", "bin", file)
        dst = os.path.join("out", "raw", "japi", "dlls", file)

        if not os.path.exists(src):
            raise FileNotFoundError(f"{src} does not exist. Build failed?")

        shutil.copy2(src, dst)

def create_minimal_package():
    # zip the out/raw folder into out/japi_release.zip
    subprocess.run(
        ["powershell", "Compress-Archive", "-Path", "out/raw/*", "-DestinationPath", "out/japi_release.zip"],
        check=True,
    )


def create_full_package():
    os.makedirs("out/raw/japi/dlls/libs", exist_ok=True)

    for file in os.listdir("../external/dlls"):
        src = os.path.join("../external/dlls", file)
        dst = os.path.join("out/raw/japi/dlls/libs", file)

        if os.path.isfile(src):
            shutil.copy2(src, dst)

    subprocess.run(
        [
            "powershell",
            "Compress-Archive",
            "-Path",
            "out/raw/*",
            "-DestinationPath",
            "out/japi_release_full.zip",
        ],
        check=True,
    )

def sign_package():
    # sign japi_release.zip using JAPISigner.exe
    subprocess.run(
        ["build/out/bin/JAPISigner.exe", "out/japi_release.zip", "../keys"],
        check=True,
    )

    # rename the file to 'versionXXX.japi'
    release_man = import_release_manifest()
    version = release_man["japi_version"]
    # squash the version string to remove dots
    version_squashed = version.replace(".", "")
    os.rename("out/japi_release.zip.signed", f"out/update{version_squashed}.japi")


def cleanup_previous_builds():
    # remove out folder if it exists
    if os.path.exists("out"):
        shutil.rmtree("out")


def main():
    cleanup_previous_builds()
    create_directories()

    # after this step we should have JAPI.dll and JAPIPreload.dll in build/out/bin/
    # As well as the JAPISigner.exe which is necessary to sign the package for release
    build_japi()

    create_manifests()
    copy_binaries()

    create_minimal_package()
    create_full_package()

    sign_package()

if __name__ == "__main__":
    main()