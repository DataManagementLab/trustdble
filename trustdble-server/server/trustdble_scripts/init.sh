#!/bin/bash

# CONSTANTS
export BOOST_VERSION="1.73.0"
export BOOST_CHECKSUM="9995e192e68528793755692917f9eb6422f3052a53c5e13ba278a228af6c7acf"
export BOOST_DIR=/tmp/boost

# DEFAULTS
PROGNAME=$(basename $0)
SYSTEM=
SYS_OPTIONS=("linux" "macos")

# USAGE FUNCTION
function usage()
{
   cat << HEREDOC
   This script initializes the development environment for the specified system type.

   Usage: $PROGNAME --system SYSTEM

   required arguments:
     -s, --system SYSTEM    pass in the system to run initialization for [ ${SYS_OPTIONS[*]} ]

HEREDOC
}

# ARGUMENTS
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -h|--help)
    usage
    exit 0
    ;;
    -s|--system) # parameter with value
    SYSTEM="$2"
    shift # past argument
    shift # past value
    ;;
    *) shift ;;     # not used parameters
esac
done

# PARAMETER CHECK

echo "[INIT] Checking parameter"

if [ -z $SYSTEM ] ; then
  echo "You have to specify a system to initialize dev environment. Use '--help' for more information."
  exit 0
elif [[ ! ${SYS_OPTIONS[*]} =~ $SYSTEM ]] ; then
  echo "The selected system is not valid. Supported systems are: ${SYS_OPTIONS[*]}"
  exit 0
fi

# PRE INSTALL

echo "[INIT] Running pre-install"

if [ $SYSTEM == "linux" ] ; then
  sudo apt-get update && export DEBIAN_FRONTEND=noninteractive
elif [ $SYSTEM == "macos" ] ; then
  brew update
  # Install Apple Command Line Tools
  if [ -d "/Library/Developer/CommandLineTools" ]; then
    echo "Apple Command Line Tools are installed."
  else
    echo "Apple Command Line Tools are not installed. Please re-run initialization script after Apple Command Line Tools are installed."
    xcode-select --install
    exit 0
  fi
fi

# INSTALL BUILD TOOLS

echo "[INIT] Install build tools"

if [ $SYSTEM == "linux" ] ; then
  sudo apt-get -y install build-essential cmake cppcheck ninja-build curl
  # check if clang and clang-tools are installed
  if [ -x "$(command -v clang++)" ] && [ -x "$(command -v clang)" ] && [ -x "$(command -v clang-tidy)" ]; then
    echo "clang and clang-tools already installed"
  else
    wget https://apt.llvm.org/llvm.sh
    chmod +x llvm.sh
    sudo ./llvm.sh ${LLVM_VERSION}
    sudo apt-get -y install clang-tidy-${LLVM_VERSION} clang-format-${LLVM_VERSION}
    sudo ln -s -f "/usr/bin/clang-tidy-${LLVM_VERSION}" "/usr/bin/clang-tidy"
    sudo ln -s -f "/usr/bin/run-clang-tidy-${LLVM_VERSION}" "/usr/bin/run-clang-tidy"
    sudo ln -s -f "/usr/bin/clang-format-${LLVM_VERSION}" "/usr/bin/clang-format"
    sudo ln -s -f "/usr/bin/clang-apply-replacements-${LLVM_VERSION}" "/usr/bin/clang-apply-replacements"
    sudo ln -s -f "/usr/bin/clang++-${LLVM_VERSION}" "/usr/bin/clang++"
    sudo ln -s -f "/usr/bin/clang-${LLVM_VERSION}" "/usr/bin/clang"
    rm llvm.sh
  fi
  if [ -x "$(command -v docker)" ]; then
    echo "Docker already installed"
  else
    sudo apt-get -y install apt-transport-https ca-certificates curl gnupg lsb-release
    curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg
    echo "deb [arch=amd64 signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

    sudo apt-get -y update
    sudo apt-get -y install docker-ce docker-ce-cli containerd.io
    echo "Please make sure to follow the docker post-installation steps to enable docker for non-root users. You might need to reboot your system for changes to take effect"
  fi
elif [ $SYSTEM == "macos" ] ; then
  brew install cmake ninja
  brew install llvm
  ln -s "$(brew --prefix llvm)/bin/clang-format" "/usr/local/bin/clang-format"
  ln -s "$(brew --prefix llvm)/bin/clang-tidy" "/usr/local/bin/clang-tidy"
  ln -s "$(brew --prefix llvm)/bin/clang-apply-replacements" "/usr/local/bin/clang-apply-replacements"
  ln -s "$(brew --prefix llvm)/share/clang/run-clang-tidy.py" "/usr/local/bin/run-clang-tidy"
fi

gitSubrepoPath="$HOME/git-subrepo"
if [ -d "$gitSubrepoPath" ]; then
  echo "git-subrepo already installed"
else
  git clone https://github.com/ingydotnet/git-subrepo $gitSubrepoPath # tool git subrepo tool to manage mysql as subrepo dependency
  rcFile="$HOME/.bashrc"
  if [[ -f "$rcFile" ]]; then
      echo "source $gitSubrepoPath/.rc" >> $rcFile
  fi
  rcFile="$HOME/.zshrc"
  if [[ -f "$rcFile" ]]; then
      echo "source $gitSubrepoPath/.rc" >> $rcFile
  fi
fi

# install node v16 and npm it is required for truffle
sudo curl -s https://deb.nodesource.com/gpgkey/nodesource.gpg.key | apt-key add -
sudo sh -c "echo deb https://deb.nodesource.com/node_16.x focal main \> /etc/apt/sources.list.d/nodesource.list"
sudo apt-get update
sudo apt-get -y install nodejs npm

# INSTALL PYTHON MODULES
echo "[INIT] Install Python Modules"
sudo python3 -m pip install grpcio-tools grpcio mysql-connector-python jurigged cryptography

# INSTALL PREREQUISITES

SYSTEM=$SYSTEM /bin/bash trustdble_scripts/.init_adapter_deps.sh

# MYSQL PREREQUISITES

# Warning: bison 3.7.6 is already installed and up-to-date.
# To reinstall 3.7.6, run:
#   brew reinstall bison
# libtirpc: Linux is required for this software.
# Error: An unsatisfied requirement failed this build.

echo "[INIT] Install MySQL Prerequisites"

if [ $SYSTEM == "linux" ] ; then
  sudo apt-get -y install libssl-dev libncurses5-dev pkg-config bison libtirpc-dev libldap-dev libsasl2-dev libsasl2-modules-gssapi-mit libcurl4-openssl-dev libudev-dev
elif [ $SYSTEM == "macos" ] ; then
  brew install openssl ncurses pkg-config bison curl libtirpc
  curl_path_export='export PATH="/usr/local/opt/curl/bin:$PATH"'
  if ! grep -q "${curl_path_export}" ~/.zshrc; then
    echo "${curl_path_export}" >> ~/.zshrc
  fi
fi

# INSTALL SPECIFIC VERSION OF BOOST

echo "[INIT] Install specific version of BOOST (1.73.0)"

install_boost_unix () {(
  mkdir -p "${BOOST_DIR}"
  cd ${BOOST_DIR}
  BOOST_VERSION_URL=${BOOST_VERSION//./_}
  curl -L --retry 3 "https://boostorg.jfrog.io/artifactory/main/release/${BOOST_VERSION}/source/boost_${BOOST_VERSION_URL}.tar.gz" -o boost.tar.gz
  echo "${BOOST_CHECKSUM}  boost.tar.gz" | sha256sum -c
  tar -xzf boost.tar.gz --strip 1
  ./bootstrap.sh
  sudo ./b2 install --without-python --prefix=/usr/local link=shared runtime-link=shared install
  cd ..
  sudo rm -rf ${BOOST_DIR}
  sudo rm -rf /var/cache/*
)}

if [ $SYSTEM == "linux" ] || [ $SYSTEM == "macos" ] ; then
  # check if boost is installed
  # This is were boost is usually installed under linux and macos
  BOOST_INSTALL_PATH="/usr/local/include/boost/version.hpp"
  BOOST_VERSION_URL=${BOOST_VERSION//./_}
  # If the patch version == 0, the version.hpp of boost only contains the major and minor version
  # Hence we only grep for the major and minor version in the following
  BOOST_VERSION_CHECK="$(echo $BOOST_VERSION_URL | cut -d _ -f 1,2)"
  # grep for version and check return value
  grep -q $BOOST_VERSION_CHECK ${BOOST_INSTALL_PATH}
  if [ 0 == $? ] ; then
    echo "[INIT] Boost ${BOOST_VERSION_CHECK} already installed, not re-installing"
  else
    echo "[INIT] Boost ${BOOST_VERSION_CHECK} NOT found, installing it"
    install_boost_unix
  fi
fi

# INSTALL DOCU BUILD TOOLS

echo "[INIT] Install documentation build tools"

if [ $SYSTEM == "linux" ] ; then
  sudo apt-get -y install doxygen plantuml graphviz
elif [ $SYSTEM == "macos" ] ; then
  brew install doxygen plantuml graphviz
fi

# INSTALL CUSTOM GIT HOOKS

echo "[INIT] Install custom git hooks"

git config core.hooksPath ./githooks

# CLEANUP

echo "[INIT] Cleanup initialization"

if [ $SYSTEM == "linux" ] ; then
  sudo apt-get autoremove -y
  sudo apt-get clean -y
  sudo rm -rf /var/lib/apt/lists/*
elif [ $SYSTEM == "macos" ] ; then
  brew autoremove
  brew cleanup
fi

echo "[INIT] Initialization finished"

echo "Please make sure to follow the docker post-installation steps to enable docker for non-root users. You might need to reboot your system for changes to take effect"
echo "See this link for further instructions https://docs.docker.com/engine/install/linux-postinstall/#manage-docker-as-a-non-root-user"
echo "Make sure that you can run docker commands as non-root user, e.g., docker ps -a"