#!/bin/bash
###
 # @Author       : LIN Guocheng
 # @Date         : 2024-09-20 04:01:18
 # @LastEditors  : LIN Guocheng
 # @LastEditTime : 2024-10-10 21:31:35
 # @FilePath     : /root/RouterRL/cmd/install.sh
 # @Description  : Install RouterRL on Ubuntu system.
### 

# Get the directory where the script is located
WORKSPACE_DIR=$(pwd)
export ROUTERRL_DIR=$WORKSPACE_DIR
mkdir -p lib
LIB_DIR=$WORKSPACE_DIR/lib

# Parse command-line arguments
CONDA_ENV_NAME="RouterRL"
while [[ "$#" -gt 0 ]]; do
    case $1 in
        --conda-env) CONDA_ENV_NAME="$2"; shift ;;
        *) echo -e "\e[31mUnknown parameter: $1\e[0m"; return 1 ;;
    esac
    shift
done

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to install a package if not already installed
install_package() {
    if ! dpkg -s "$1" >/dev/null 2>&1; then
        echo -e "\e[32mInstalling package $1...\e[0m"
        apt install -y "$1"
    else
        echo -e "\e[32mPackage $1 is already installed.\e[0m"
    fi
}

# Function to update and upgrade apt packages
update_and_upgrade() {
    echo -e "\e[32mUpdating and upgrading apt packages...\e[0m"
    apt update && apt upgrade -y
}

install_package "sudo"

# Check if conda is installed
if command_exists conda; then
    echo -e "\e[32mConda is installed.\e[0m"
    # Get the base path of conda installation
    CONDA_BASE=$(conda info --base)
    source "$CONDA_BASE/etc/profile.d/conda.sh"
    # Check if a conda environment is activated
    if [ -z "$CONDA_DEFAULT_ENV" ]; then
        echo -e "\e[31mNo conda environment is currently activated.\e[0m"
        read -p "Please enter the name of the conda environment to activate: " CONDA_ENV_NAME
        conda activate "$CONDA_ENV_NAME"
    else
        # Check if the RouterRL conda environment exists; if not, create it
        if conda env list | grep -q "^RouterRL\s"; then
            echo -e "\e[32mConda environment 'RouterRL' exists. Activating it...\e[0m"
            conda activate RouterRL
        else
            read -p "Would you like to create and activate a new conda environment named 'RouterRL'? (y/n): " CREATE_ENV
            if [[ "$CREATE_ENV" =~ ^[Yy]$ ]]; then
                echo -e "\e[32mUpdating and upgrading apt packages before creating a new environment...\e[0m"
                update_and_upgrade

                echo -e "\e[32mCreating and activating conda environment 'RouterRL'...\e[0m"
                conda create -n RouterRL python=3.11 -y
                conda activate RouterRL
            else
                read -p "Please enter the name of an existing conda environment to activate: " CONDA_ENV_NAME
                conda activate "$CONDA_ENV_NAME"
            fi
        fi
    fi

    # Required Python packages
    trap '' PIPE
    REQUIRED_PYTHON_PACKAGES=(
        numpy
        scipy
        pandas
        matplotlib
        posix_ipc
        zmq
        rich
    )

    # Check and install missing Python packages
    echo -e "\e[32mChecking for required Python packages...\e[0m"
    for pkg in "${REQUIRED_PYTHON_PACKAGES[@]}"; do
        if ! pip show "$pkg" > /dev/null 2>&1; then
            echo -e "\e[33mPackage '$pkg' is not installed in the environment '$CONDA_ENV_NAME'. Installing...\e[0m"
            pip install "$pkg"
        else
            echo -e "\e[32mPackage '$pkg' is already installed.\e[0m"
        fi
    done
else
    echo -e "\e[31mConda is not installed. Please install Anaconda or Miniconda and create the required environment.\e[0m"
    return 1
fi

# Install dependencies
echo -e "\e[32mInstalling dependencies...\e[0m"
DEPENDENCIES=(
    build-essential
    gcc
    g++
    bison
    flex
    perl
    psmisc
    libxml2-dev
    zlib1g-dev
    xdg-utils
)

# Update and upgrade apt packages before installing dependencies to ensure the latest versions
update_and_upgrade

for package in "${DEPENDENCIES[@]}"; do
    install_package "$package"
done

# Ensure /usr/share/desktop-directories/ exists
if [ ! -d "/usr/share/desktop-directories/" ]; then
    echo -e "\e[32mCreating directory /usr/share/desktop-directories/...\e[0m"
    mkdir -p /usr/share/desktop-directories/
    echo -e "\e[32mDirectory /usr/share/desktop-directories/ created.\e[0m"
else
    echo -e "\e[32mDirectory /usr/share/desktop-directories/ already exists.\e[0m"
fi

# Install ZMQ
echo -e "\e[32mInstalling ZMQ...\e[0m"
install_package libzmq3-dev

# Install OMNeT++
cd "$LIB_DIR"
OMNETPP_VERSION="6.0.1"
OMNETPP_TGZ="omnetpp-${OMNETPP_VERSION}-linux-x86_64.tgz"
OMNETPP_DIR="omnetpp-${OMNETPP_VERSION}"

if [ ! -f "$OMNETPP_TGZ" ]; then
    echo -e "\e[32mDownloading OMNeT++...\e[0m"
    wget "https://github.com/omnetpp/omnetpp/releases/download/omnetpp-${OMNETPP_VERSION}/${OMNETPP_TGZ}"
else
    echo -e "\e[32mOMNeT++ archive already exists.\e[0m"
fi

if [ ! -d "$OMNETPP_DIR" ]; then
    echo -e "\e[32mExtracting OMNeT++...\e[0m"
    tar zxvf "$OMNETPP_TGZ" -C ./
else
    echo -e "\e[32mOMNeT++ directory already exists.\e[0m"
fi

cd "$OMNETPP_DIR"

# Modify configure.user
echo -e "\e[32mConfiguring OMNeT++...\e[0m"
if [ -f "configure.user" ]; then
    sed -i 's/^WITH_QTENV=yes/WITH_QTENV=no/' configure.user
    sed -i 's/^WITH_OSG=yes/WITH_OSG=no/' configure.user
    sed -i 's/^WITH_OSGEARTH=yes/WITH_OSGEARTH=no/' configure.user
else
    echo "WITH_QTENV=no" >> configure.user
    echo "WITH_OSG=no" >> configure.user
    echo "WITH_OSGEARTH=no" >> configure.user
fi

# Run setenv
echo -e "\e[32mExecuting setenv...\e[0m"
source setenv -f

# Run configure
echo -e "\e[32mRunning configure...\e[0m"
./configure

# Build OMNeT++
echo -e "\e[32mBuilding OMNeT++...\e[0m"

if ! make -j$(nproc); then
    echo -e "\e[31mError: make failed!\e[0m\n"
    cd $WORKSPACE_DIR
    return 1
fi

# Test OMNeT++
echo -e "\e[32mTesting OMNeT++...\e[0m"
cd samples/dyna/
./dyna


# Install INET
cd "$LIB_DIR"
INET_VERSION="4.5.0"
INET_TGZ="inet-${INET_VERSION}-src.tgz"
INET_DIR="inet4.5"

if [ ! -f "$INET_TGZ" ]; then
    echo -e "\e[32mDownloading INET...\e[0m"
    wget "https://github.com/inet-framework/inet/releases/download/v${INET_VERSION}/${INET_TGZ}"
else
    echo -e "\e[32mINET archive already exists.\e[0m"
fi

if [ ! -d "$INET_DIR" ]; then
    echo -e "\e[32mExtracting INET...\e[0m"
    tar zxvf "$INET_TGZ" -C ./
    mv "inet4.5" "$INET_DIR"
else
    echo -e "\e[32mINET directory already exists.\e[0m"
fi

cd "$INET_DIR"

# Build INET
echo -e "\e[32mBuilding INET...\e[0m"
source setenv -f
make makefiles
if [ -f "Makefile" ]; then
    echo -e "\e[32mModifying MAKEMAKE_OPTIONS to include -lzmq...\e[0m"
    sed -i 's/^\(MAKEMAKE_OPTIONS\s*:=.*\)$/\1 -lzmq/' Makefile

else
    echo -e "\e[31mError: Makefile not found after running make makefiles.\e[0m"
    return 1
fi

if ! make -j$(nproc); then
    echo -e "\e[31mError: make failed!\e[0m\n"
    cd $WORKSPACE_DIR
    return 1
fi

echo -e "\e[32mExecuting setenv...\e[0m"
source setenv -f

cd "$WORKSPACE_DIR"

echo -e "\e[32mInstalling RouterRL...\e[0m"

cp -r $WORKSPACE_DIR/modules/ipv4/* $INET_ROOT/src/inet/networklayer/ipv4/
cp -r $WORKSPACE_DIR/modules/udpapp/* $INET_ROOT/src/inet/applications/udpapp/
cp -r $WORKSPACE_DIR/modules/init_config/* $INET_ROOT/src/inet/networklayer/configurator/ipv4/

cd $INET_ROOT
make makefiles
make -j$(nproc)
source setenv -f

cd "$WORKSPACE_DIR"
cat <<EOF > "$WORKSPACE_DIR/setup.py"
from setuptools import setup, find_packages

setup(
    name='router_rl',
    version='1.0.0',
    description='Interface for RouterRL.',
    author='LIN Guocheng',
    author_email='lgc0208@foxmail.com',
    packages=find_packages(where='modules'),
    package_dir={"": "modules"},
    install_requires=[
        'zmq',
        'rich',
    ],
    python_requires='>=3.6',
)
EOF

echo -e "\e[32msetup.py has been created at $WORKSPACE_DIR.\e[0m"
pip install .
echo -e "\e[32mrouter_rl package has been installed.\e[0m"
python "$WORKSPACE_DIR/setup.py" clean --all
rm -r "$WORKSPACE_DIR/modules/router_rl.egg-info"
rm "$WORKSPACE_DIR/setup.py"
echo -e "\e[32msetup.py has been removed.\e[0m"

echo -e "\e[32mRemoving OMNeT++ archive $OMNETPP_TGZ...\e[0m"
rm "$LIB_DIR/$OMNETPP_TGZ"
echo -e "\e[32mOMNeT++ archive removed.\e[0m"

# Delete the INET tgz file after extraction
echo -e "\e[32mRemoving INET archive $INET_TGZ...\e[0m"
rm "$LIB_DIR/$INET_TGZ"
echo -e "\e[32mINET archive removed.\e[0m"

echo -e "\e[32mSetting start scripts...\e[0m"
echo -e "\n# State scripts for OMNeT++ and INET environments in RouterRL" >> "$WORKSPACE_DIR/cmd/start.sh"
touch "$WORKSPACE_DIR/cmd/start.sh"
echo "cd $LIB_DIR/$OMNETPP_DIR" >> "$WORKSPACE_DIR/cmd/start.sh"
echo ". setenv -f" >> "$WORKSPACE_DIR/cmd/start.sh"
echo "cd $LIB_DIR/$INET_DIR" >> "$WORKSPACE_DIR/cmd/start.sh"
echo ". setenv -f" >> "$WORKSPACE_DIR/cmd/start.sh"
echo "cd $WORKSPACE_DIR" >> "$WORKSPACE_DIR/cmd/start.sh"
echo "conda activate RouterRL" >> "$WORKSPACE_DIR/cmd/start.sh"
echo -e "\e[32mOMNeT++, INET, and RouterRL environments have been set to start script.\e[0m"



read -p "Would you like to set the auto-configuration for OMNeT++, INET and RouterRL environments? (y/n): " SELF_START
if [[ "$SELF_START" =~ ^[Yy]$ ]]; then
    echo -e "\e[32mSetting self-start...\e[0m"
    if ! grep -q "Auto-configuration for OMNeT++ and INET environments" ~/.bashrc; then
        echo -e "\n# Auto-configuration for OMNeT++ and INET environments" >> ~/.bashrc
        echo "export ROUTERRL_DIR=$WORKSPACE_DIR" >> ~/.bashrc
        echo "cd $LIB_DIR/$OMNETPP_DIR" >> ~/.bashrc
        echo ". setenv -f" >> ~/.bashrc
        echo "cd $LIB_DIR/$INET_DIR" >> ~/.bashrc
        echo ". setenv -f" >> ~/.bashrc
        echo "cd $WORKSPACE_DIR" >> ~/.bashrc
        echo "conda activate RouterRL" >> ~/.bashrc
        echo -e "\e[32mOMNeT++, INET, and RouterRL environments have been set to auto-configuration.\e[0m"
    else
        echo -e "\e[33mAuto-configuration already exists in .bashrc. Skipping...\e[0m"
    fi
fi
cd $WORKSPACE_DIR
echo -e "\e[32mInstallation completed.\e[0m\n"


# Start testing
echo -e "\e[32mStarting test scripts...\e[0m\n"

# Run OSPF test script
echo -e "\e[34mRunning OSPF script\e[0m"
if python src/convention/ospf/main.py > /dev/null 2>&1; then
    echo -e "\e[32mOSPF test passed.\e[0m\n"
else
    echo -e "\e[31mOSPF test failed.\e[0m\n"
fi

# Run ECMP test script
echo -e "\e[34mRunning ECMP script\e[0m"
if python src/convention/ecmp/main.py > /dev/null 2>&1; then
    echo -e "\e[32mECMP test passed.\e[0m\n"
else
    echo -e "\e[31mECMP test failed.\e[0m\n"
fi

# Run probabilistic routing test script
echo -e "\e[34mRunning probabilistic routing script\e[0m"
echo -e "\e[34m\t1 - Running global probabilistic routing script\e[0m"
if python src/rl_probabilistic_routing/demo_global/main.py > /dev/null 2>&1; then
    echo -e "\e[32m\tGlobal probabilistic routing passed.\e[0m"
else
    echo -e "\e[31m\tGlobal probabilistic routing failed.\e[0m"
fi
echo -e "\e[34m\t2 - Running distributed probabilistic routing script\e[0m"
if python src/rl_probabilistic_routing/demo_distributed/main.py > /dev/null 2>&1; then
    echo -e "\e[32m\tDistributed probabilistic routing passed.\e[0m"
else
    echo -e "\e[31m\tDistributed probabilistic routing failed.\e[0m"
fi
echo -e "\e[32mProbabilistic routing tests finished.\e[0m\n"

# Run multipath routing test script
echo -e "\e[34mRunning multipath routing script\e[0m"
echo -e "\e[34m\t1 - Running global multipath routing script\e[0m"
if python src/rl_multipath_routing/demo_global/main.py > /dev/null 2>&1; then
    echo -e "\e[32m\tGlobal multipath routing passed.\e[0m"
else
    echo -e "\e[31m\tGlobal multipath routing failed.\e[0m"
fi
echo -e "\e[34m\t2 - Running distributed multipath routing script\e[0m"
if python src/rl_multipath_routing/demo_distributed/main.py > /dev/null 2>&1; then
    echo -e "\e[32m\tDistributed multipath routing passed.\e[0m"
else
    echo -e "\e[31m\tDistributed multipath routing failed.\e[0m"
fi
echo -e "\e[32mMultipath routing tests finished.\e[0m\n"

# Run path routing test script
echo -e "\e[34mRunning path routing script\e[0m"
echo -e "\e[34m\t1 - Running global path routing script\e[0m"
if python src/rl_path_routing/demo_global/main.py > /dev/null 2>&1; then
    echo -e "\e[32m\tGlobal path routing passed.\e[0m"
else
    echo -e "\e[31m\tGlobal path routing failed.\e[0m"
fi
echo -e "\e[34m\t2 - Running distributed path routing script\e[0m"
if python src/rl_path_routing/demo_distributed/main.py > /dev/null 2>&1; then
    echo -e "\e[32m\tDistributed path routing passed.\e[0m"
else
    echo -e "\e[31m\tDistributed path routing failed.\e[0m"
fi
echo -e "\e[32mPath Routing tests finished.\e[0m\n"

echo -e "\e[32mAll tests finished.\e[0m"