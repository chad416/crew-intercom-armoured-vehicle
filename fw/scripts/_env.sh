# Sourced by the other scripts. Sets REPO, WS and BUILD, and activates the
# Python venv. Environment variables (all optional):
#   ICOM_WS     west workspace (zephyr/, modules/); default $HOME/recorder-ws
#   ICOM_VENV   Python venv with west and Robot; default $HOME/zvenv
#   ICOM_BUILD  firmware build directory; default $ICOM_WS/build/icom
# Zephyr's build breaks on paths that contain a space. If the repository path
# has one (for example a Windows folder seen from WSL), it is bind-mounted at
# $HOME/icom-src first (scripts/wsl-mount.sh, needs root).

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
if [[ "$REPO" == *" "* ]]; then
	bash "$REPO/scripts/wsl-mount.sh" >/dev/null
	REPO="$HOME/icom-src"
fi
WS="${ICOM_WS:-$HOME/recorder-ws}"
BUILD="${ICOM_BUILD:-$WS/build/icom}"

# shellcheck disable=SC1091
source "${ICOM_VENV:-$HOME/zvenv}/bin/activate"
