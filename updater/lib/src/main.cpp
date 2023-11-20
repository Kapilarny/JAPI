#include "main.h"

#include "updater.h"

#include "downloader.h"

Version GetUpdaterVersion() {
    return {JAPI_UPDATER_MAJOR, JAPI_UPDATER_MINOR, JAPI_UPDATER_PATCH};
}

void LaunchUpdater() {
    UpdaterMain();
}